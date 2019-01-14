/*
 * Node.h
 *
 *  Created on: Jan 9, 2019
 *      Author: William Anderson
 */

#ifndef NODE_H_
#define NODE_H_

#include <algorithm>
#include "circular_buffer.h"
#include "Trigger.h"
#include "Types.h"

namespace juniper {

template<typename OutputType> class Node;

template<class OutputType>
struct NodeIterator: public juniper::circular_buffer<OutputType>::iterator {
	typedef typename juniper::circular_buffer<OutputType>::iterator base_iterator;
	typedef juniper::Node<OutputType> Buff;
	typedef NodeIterator<Buff> nonconst_self;
//	typedef typename boost::cb_details::iterator<boost::circular_buffer<OutputType>, NonConstTraits> cb_iterator;
	typedef typename juniper::circular_buffer<OutputType>::iterator cb_iterator;

	typedef typename base_iterator::value_type value_type;
	typedef typename base_iterator::pointer pointer;
	typedef typename base_iterator::reference reference;
	typedef typename base_iterator::size_type size_type;
	typedef typename base_iterator::difference_type difference_type;

	// ***** Member Variables *****

	// Pointer to the Node object
	Buff* m_buff;

	// Base (non-const) iterator to the circular buffer
	cb_iterator m_cb_it;

	// ***** Constructors *****

	// Default constructor
	NodeIterator() :
			m_buff(0), m_cb_it(cb_iterator())
	{
	}

	// Copy constructor
	NodeIterator(const nonconst_self& it) :
			m_cb_it(it.m_cb_it)
	{
	}

	// Constructor based on a circular_buffer iterator
	NodeIterator(Buff* cb, const cb_iterator cb_it) :
			m_buff(cb), m_cb_it(cb_it)
	{
	}

	// ***** Operators *****
	//
	// Modeled on boost::cb_details::iterator (boost/circular_buffer/details.hpp)

	NodeIterator& operator =(const NodeIterator& it)
	{
		if (this == &it)
			return *this;
		m_buff = it.m_buff;
		m_cb_it = it.m_cb_it;
		return *this;
	}

	// Dereferencing operator.  We change the behavior here to evaluate missing values as needed.
	// Note that this requires m_cb_it to be a non_const type, even if we are a const iterator.

	reference operator *() const
	{
		return *m_cb_it;
		//reference val = *m_cb_it;
		//if(!missing_value<OutputType>::isMissing(val))
		//	return val;
		//return (*m_cb_it = m_buff->evaluate(index()));
	}

	pointer operator ->() const
	{
		return &(operator*());
	}

	difference_type operator -(const NodeIterator<OutputType>& it) const
	{
		return m_cb_it - it.m_cb_it;
	}

	NodeIterator& operator ++()
	{			// ++it
		++m_cb_it;
		return *this;
	}
	NodeIterator operator ++(int)
	{		// it++
		NodeIterator<OutputType> tmp = *this;
		++m_cb_it;
		return tmp;
	}
	NodeIterator& operator --()
	{			// --it
		--m_cb_it;
		return *this;
	}
	NodeIterator operator --(int)
	{		// it--
		NodeIterator<OutputType> tmp = *this;
		m_cb_it--;
		return tmp;
	}
	NodeIterator& operator +=(difference_type n)
	{		// it += n
		m_cb_it += n;
		return *this;
	}
	NodeIterator& operator -=(difference_type n)
	{		// it -= n
		m_cb_it -= n;
		return *this;
	}

	NodeIterator operator +(difference_type n) const
	{
		return NodeIterator<OutputType>(*this) += n;
	}
	NodeIterator operator -(difference_type n) const
	{
		return NodeIterator<OutputType>(*this) -= n;
	}

	reference operator [](difference_type n) const
	{
		return *(*this + n);
	}

	// ***** Comparisons *****
	//
	// This iterator class implements some unusual comparison behavior: two iterators are evaluated by their offset within
	// their respective buffers, even if they point to separate buffers.  When used on synchronized buffers, this allows
	// us to evaluate which of two iterators points to the earlier event.

	template<class OutputType0>
	bool operator ==(const NodeIterator<OutputType0>& it) const
	{
		return index() == it.index();
	}

	template<class OutputType0>
	bool operator !=(const NodeIterator<OutputType0>& it) const
	{
		return index() != it.index();
	}

	template<class OutputType0>
	bool operator <(const NodeIterator<OutputType0>& it) const
	{
		return index() < it.index();
	}

	template<class OutputType0>
	bool operator >(const NodeIterator<OutputType0>& it) const
	{
		return it < *this;
	}

	template<class OutputType0>
	bool operator <=(const NodeIterator<OutputType0>& it) const
	{
		return !(it < *this);
	}

	template<class OutputType0>
	bool operator >=(const NodeIterator<OutputType0>& it) const
	{
		return !(*this < it);
	}

	// ***** Special Methods *****

	// Return the offset within the buffer for this iterator's current location
	// Can be used with at() or operator[], and can be used to compare relative locations
	// of two iterators, even if they don't refer to the same buffer

	size_type index() const
	{
		return (size_type) (*this - m_buff->begin() + m_buff->firstSampleIndex_);
	}

	// Return the timestamp associated with the sample this iterator points to

	timestamp_type timestamp() const
	{
		return m_buff->timestampAt(index());
	}

	// Tells us whether the index points to a valid place in the buffer.
	// TODO: make sure this is right
	// bool isValid() const { return (index() >= m_buff->beginIndex() && index() <= (m_buff->endIndex() - 1)); }
};

class NodeBase : public TriggerSource, public TriggerDestination {
public:
	typedef size_t size_type;

	// ***** Constructors *****

	// Default constructor
	NodeBase() {}

	// Copy constructor: can't copy the mutexes
	NodeBase(NodeBase const& obj) {}

	NodeBase& operator= (NodeBase const& obj) {
		//listeners_ = obj.listeners_;
		return *this;
	}

	// ***** Destructor *****

	virtual ~NodeBase() {
		//clearTriggerSources();
	}

	// ***** Modifiers *****

	virtual void clear() = 0;
	//virtual void insertMissing(timestamp_type timestamp) = 0;

	// ***** Listener Methods *****
	//
	// We need to keep the buffers of all connected units in sync.  That means any Source or Filter needs to know what its output
	// connects to.  That functionality is accomplished by "listeners": any time a new sample is added to the buffer, the listeners
	// are notified with a corresponding timestamp.  The latter is to ensure that a buffer does not respond to notifications from multiple
	// inputs for the same data point.

	/*void registerListener(NodeBase* listener) {
		if(listener != 0) {
			listenerAccessMutex_.lock();
			listeners_.insert(listener);
			listenerAccessMutex_.unlock();
		}
	}
	void unregisterListener(NodeBase* listener) {
		listenerAccessMutex_.lock();
		listeners_.erase(listener);
		listenerAccessMutex_.unlock();
	}
	void clearListeners() {
		listenerAccessMutex_.lock();
		listeners_.clear();
		listenerAccessMutex_.unlock();
	}
protected:
	// Tell all our listeners about a new data point with the given timestamp
	void notifyListenersOfInsert(timestamp_type timestamp) {
		std::set<NodeBase*>::iterator it;

		listenerAccessMutex_.lock();
		for(it = listeners_.begin(); it != listeners_.end(); it++)
			(*it)->insertMissing(timestamp);
		listenerAccessMutex_.unlock();
	}
	// Tell all our listeners that the buffer has cleared
	void notifyListenersOfClear() {
		std::set<NodeBase*>::iterator it;

		listenerAccessMutex_.lock();
		for(it = listeners_.begin(); it != listeners_.end(); it++)
			(*it)->clear();
		listenerAccessMutex_.unlock();
	}*/

public:
	// ***** Tree-Parsing Methods *****
	//
	// Sometimes we want to find out properties of the initial data source, regardless of what filters it's passed through.  These virtual
	// methods should be implemented differently by Source and Filter classes.

	//virtual SourceBase& source() = 0;		// Return the source at the top of the tree for this unit

	// ***** Mutex Methods *****
	//
	// These methods should be used to acquire a lock whenever a process wants to read values from the buffer.  This would, for example,
	// allow iteration over the contents of the buffer without worrying that the contents will change in the course of the iteration.

	void lock_mutex() { pthread_mutex_lock(&bufferAccessMutex_); }
	bool try_lock_mutex() { return pthread_mutex_trylock(&bufferAccessMutex_); }
	void unlock_mutex() { pthread_mutex_unlock(&bufferAccessMutex_); }

	// ***** Circular Buffer (STL) Methods *****
	//
	// Certain STL methods (and related queries) that do not depend on the specific data type of the buffer should
	// be available to any object with a NodeBase reference.

	virtual size_type size() = 0;				// Size: how many elements are currently in the buffer
	virtual bool empty() = 0;
	virtual bool full() = 0;
	virtual size_type reserve() = 0;			// Reserve: how many elements are left before the buffer is full
	virtual size_type capacity() const = 0;			// Capacity: how many elements could be in the buffer

	virtual size_type beginIndex() = 0;			// Index of the first sample we still have in the buffer
	virtual size_type endIndex() = 0;			// Index just past the end of the buffer

	// ***** Timestamp Methods *****
	//
	// Every sample is tagged with a timestamp.  We don't necessarily need to know the type of the sample to retrieve its
	// associated timestamp.  However, we DO need to know the type in order to return an iterator.

	virtual timestamp_type timestampAt(size_type index) = 0;
	virtual timestamp_type latestTimestamp() = 0;
	virtual timestamp_type earliestTimestamp() = 0;

	virtual size_type indexNearestTo(timestamp_type t) = 0;
	virtual size_type indexNearestBefore(timestamp_type t) = 0;
	virtual size_type indexNearestAfter(timestamp_type t) = 0;

	// ***** Member Variables *****
protected:
	// A collection of the units that are listening for updates on this unit.
	//std::set<NodeBase*> listeners_;

	// This mutex protects access to the underlying buffer.  It is locked every time a sample is written to the buffer,
	// and external systems reading values from the buffer should also acquire at least a shared lock.
	pthread_mutex_t bufferAccessMutex_;

	// This mutex protects the list of listeners.  It prevents a listener from being added or removed while a notification
	// is in progress.
	//boost::mutex listenerAccessMutex_;

	// Keep an internal registry of who we've asked to send us triggers.  It's important to keep
	// a list of these so that when this object is destroyed, all triggers are automatically unregistered.
	//std::set<NodeBase*> triggerSources_;

	// This list tracks the destinations we are *sending* triggers to (as opposed to the sources we're receiving from above)
	//std::set<NodeBase*> triggerDestinations_;
};

template <typename T>
//struct NodeReverseIterator : public boost::reverse_iterator<T> {
struct NodeReverseIterator : public juniper::circular_buffer<T>::reverse_iterator {
	NodeReverseIterator() {}
	explicit NodeReverseIterator(T baseIt) : juniper::circular_buffer<T>::reverse_iterator(baseIt) {}

	typedef typename juniper::circular_buffer<T>::reverse_iterator::base_type::size_type size_type;

//	size_type index() const { return boost::prior(this->base_reference()).index(); }
//	timestamp_type timestamp() const { return boost::prior(this->base_reference()).timestamp(); }
};

/*
 * NodeInterpolatedIterator
 *
 * Extends the iterator concept to fractional indices, using linear interpolation to return
 * values and timestamps.  This is always a const iterator class.
 */

template<typename T>
struct NodeInterpolatedIterator :
	public juniper::circular_buffer<T>::reverse_iterator
{
	typedef NodeInterpolatedIterator<T> self_type;

    typedef typename juniper::circular_buffer<T>::reverse_iterator base_iterator;

	typedef typename base_iterator::size_type size_type;
	typedef typename base_iterator::value_type value_type;
	typedef typename base_iterator::pointer pointer;
	typedef typename base_iterator::reference reference;

	// ***** Member Variables *****

	// Reference to the buffer this iterator indexes
	Node<T>* m_buff;

	// Index location within the buffer
	double m_index;

	// Step size for ++ and similar operators
	double m_step;

	// ***** Constructors *****

	// Default (empty) constructor
	NodeInterpolatedIterator() : m_buff(0), m_index(0.0), m_step(1.0) {}

	// Constructor that should be used primarily by the Node class itself
	NodeInterpolatedIterator(Node<T>* buff, double index, double stepSize)
	: m_buff(buff), m_index(index), m_step(stepSize) {}

	// Copy constructor
	NodeInterpolatedIterator(const self_type& obj) : m_buff(obj.m_buff), m_index(obj.m_index), m_step(obj.m_step) {}

	// ***** Operators *****
	//
	// Modeled on STL iterators, but using fractional indices.  This class should be considered a sort of random access,
	// bidirectional iterator.

    NodeInterpolatedIterator& operator = (const self_type& it) {
        if (this == &it)
            return *this;
        m_buff = it.m_buff;
        m_index = it.m_index;
		m_step = it.m_step;
        return *this;
    }

	// Dereferencing operators.  Like the non-interpolated version of this iterator, it will calculate the values as needed.
	// This happens within the operator[] method of Node, rather than in this class itself.

    value_type operator * () const { return m_buff->interpolate(m_index); }
	pointer operator -> () const { return &(operator*()); }

	// Difference of two iterators (return the difference in indices)
    double operator - (const self_type& it) const { return m_index - it.m_index; }

	// Increment and decrement are typically integer operators.  We define their behavior here to change the index by a predetermined
	// step size, set at construction but changeable.

    self_type& operator ++ () {			// ++it
		m_index += m_step;
		return *this;
	}
	self_type operator ++ (int) {		// it++
		self_type tmp = *this;
		m_index += m_step;
		return tmp;
	}
	self_type& operator -- () {			// --it
		m_index -= m_step;
		return *this;
	}
	self_type operator -- (int) {		// it--
		self_type tmp = *this;
		m_index -= m_step;
		return tmp;
	}

	// These methods change the iterator location by a fractional amount.  Notice that this is NOT scaled by m_step.
    self_type& operator += (double n) {		// it += n
		m_index += n;
        return *this;
    }
    self_type& operator -= (double n) {		// it -= n
		m_index -= n;
        return *this;
    }

	self_type operator + (double n) const { return NodeInterpolatedIterator<T>(*this) += n; }
	self_type operator - (double n) const { return NodeInterpolatedIterator<T>(*this) -= n; }

	reference operator [] (double n) const { return *(*this + n); }


	// ***** Comparisons *****
	//
	// The comparison behavior is the same as for NodeIterator: even if two iterators point to different buffers,
	// they can be compared on the basis of the indices.  Of course, this is only meaningful if the two buffers are synchronized
	// in time.

	template<class OutputType0>
    bool operator == (const NodeInterpolatedIterator<OutputType0>& it) const { return m_index == it.m_index; }

	template<class OutputType0>
    bool operator != (const NodeInterpolatedIterator<OutputType0>& it) const { return m_index != it.m_index; }

	template<class OutputType0>
	bool operator < (const NodeInterpolatedIterator<OutputType0>& it) const { return m_index < it.m_index; }

	template<class OutputType0>
	bool operator > (const NodeInterpolatedIterator<OutputType0>& it) const { return m_index > it.m_index; }

	template<class OutputType0>
	bool operator <= (const NodeInterpolatedIterator<OutputType0>& it) const { return !(it < *this); }

	template<class OutputType0>
	bool operator >= (const NodeInterpolatedIterator<OutputType0>& it) const { return !(*this < it); }

	// We can also compare interpolated and non-interpolated iterators.

    template <class OutputType0>
    bool operator == (const NodeIterator<OutputType0>& it) const { return m_index == (double)it.index(); }

    template <class OutputType0>
    bool operator != (const NodeIterator<OutputType0>& it) const { return m_index != (double)it.index(); }

    template <class OutputType0>
    bool operator < (const NodeIterator<OutputType0>& it) const { return m_index < (double)it.index(); }

    template <class OutputType0>
    bool operator > (const NodeIterator<OutputType0>& it) const { return m_index > (double)it.index(); }

	template <class OutputType0>
    bool operator <= (const NodeIterator<OutputType0>& it) const { return m_index <= (double)it.index(); }

    template <class OutputType0>
    bool operator >= (const NodeIterator<OutputType0>& it) const { return m_index >= (double)it.index(); }

	// ***** Special Methods *****

	// Round a fractional index up, down, or to the nearest integer

    self_type& roundDown() {
		m_index = floor(m_index);
		return *this;
	}
    self_type& roundUp() {
		m_index = ceil(m_index);
		return *this;
	}
    self_type& round() {
		m_index = floor(m_index + 0.5);
		return *this;
	}

	// Increment the iterator by a difference in time rather than a difference in index.  This is less efficient to
	// compute, but can be useful for buffers whose samples are not regularly spaced in time.

	self_type& incrementTime(timestamp_diff_type ts) {
		if(ts > 0) {
			size_type afterIndex = (size_type)ceil(m_index);
			if(afterIndex >= m_buff->endIndex()) {
				m_index = (double)m_buff->endIndex();
				return *this;
			}
			timestamp_type target = timestamp() + ts;
			timestamp_type after = m_buff->timestampAt(afterIndex);

			// First of all, find the first integer index with a timestamp greater than our new target time.
			while(after < target) {
				afterIndex++;
				if(afterIndex >= m_buff->endIndex()) {
					m_index = (double)m_buff->endIndex();
					return *this;
				}
				after = m_buff->timestampAt(afterIndex);
			}

			// Then find the timestamp immediately before that.  We'll interpolate between these two to get
			// the adjusted index.
			timestamp_type before = m_buff->timestampAt(afterIndex-1);
			m_index = ((target - before) / (after - before)) + (double)(afterIndex - 1);
		}
		else if(ts < 0) {
			size_type beforeIndex = (size_type)floor(m_index);
			if(beforeIndex < m_buff->beginIndex()) {
				m_index = (double)m_buff->beginIndex()-1.0;
				return *this;
			}
			timestamp_type target = timestamp() + ts;
			timestamp_type before = m_buff->timestampAt(beforeIndex);

			// Find the first integer index with a timestamp less than our new target time.
			while(before > target) {
				beforeIndex--;
				if(beforeIndex < m_buff->beginIndex()) {
					m_index = (double)m_buff->beginIndex()-1.0;
					return *this;
				}
				before = m_buff->timestampAt(beforeIndex);
			}

			// Now find the timestamp immediately after that.  Interpolated to get the adjusted index.
			timestamp_type after = m_buff->timestampAt(beforeIndex+1);
			m_index = ((target - before)/(after - before)) + (double)beforeIndex;
		}
		// if(ts == 0), do nothing
		return *this;
	}


	// Return (or change) the step size
	double& step() { return m_step; }

	// Return the index within the buffer.  The index is an always-increasing sample number (which means that as the
	// buffer fills, an index will continue to point to the same piece of data until that data is overwritten, at which
	// point that index is no longer valid for accessing the buffer at all.)
	double index() const { return m_index; }

	// Return the timestamp for this particular location in the buffer (using linear interpolation).
	timestamp_type timestamp() const { return m_buff->interpolatedTimestampAt(m_index); }

	// Tells us whether the index points to a valid place in the buffer.
	bool isValid() const { return (m_index >= m_buff->beginIndex() && m_index <= (m_buff->endIndex() - 1)); }
};


template<class T>
class Node : public NodeBase {
public:
	typedef size_t size_type;
	typedef size_t capacity_type;
	typedef T return_value_type;

	typedef NodeIterator<T> const_iterator;
	typedef const_iterator iterator;
	typedef NodeReverseIterator<const_iterator> const_reverse_iterator;
	typedef const_reverse_iterator reverse_iterator;

	explicit Node(capacity_type capacity) :
			insertMissingLastTimestamp_(0), buffer_(0), numSamples_(0), firstSampleIndex_(
					0)
	{
		buffer_ = new juniper::circular_buffer<T>(capacity);
		timestamps_ = new juniper::circular_buffer<timestamp_type>(capacity);
	}

	size_type beginIndex()
	{
		return firstSampleIndex_;
	}					// Index of the first sample we still have in the buffer

	size_type endIndex()
	{
		return firstSampleIndex_ + buffer_->size();
	}	// Index just past the end of the buffer

	timestamp_type timestampAt(size_type index)
	{
		return (*timestamps_)[(index - this->firstSampleIndex_)];
	}

	size_type indexNearestBefore(timestamp_type t) {
//		juniper::circular_buffer<timestamp_type>::iterator it = std::find_if(timestamps_->begin(), timestamps_->end(), t < boost::lambda::_1);
		juniper::circular_buffer<timestamp_type>::iterator it = std::find_if(timestamps_->begin(), timestamps_->end(), [t](timestamp_type n) {return t < n; });

		if(it == timestamps_->end())
			return timestamps_->size()-1+this->firstSampleIndex_;
		if(it - timestamps_->begin() == 0)
			return this->firstSampleIndex_;
		return (size_type)((--it) - timestamps_->begin()) + this->firstSampleIndex_;
	}
	size_type indexNearestAfter(timestamp_type t) {
//		juniper::circular_buffer<timestamp_type>::iterator it = std::find_if(timestamps_->begin(), timestamps_->end(), t < juniper::lambda::_1);
		juniper::circular_buffer<timestamp_type>::iterator it = std::find_if(timestamps_->begin(), timestamps_->end(), [t](timestamp_type n) {return t < n; });

		return std::min<size_type>((it - timestamps_->begin()), timestamps_->size()-1) + this->firstSampleIndex_;
	}
	size_type indexNearestTo(timestamp_type t) {
//		juniper::circular_buffer<timestamp_type>::iterator it = std::find_if(timestamps_->begin(), timestamps_->end(), t < juniper::lambda::_1);
		juniper::circular_buffer<timestamp_type>::iterator it = std::find_if(timestamps_->begin(), timestamps_->end(), [t](timestamp_type n) {return t < n; });

		if(it == timestamps_->end())
			return timestamps_->size()-1+this->firstSampleIndex_;
		if(it - timestamps_->begin() == 0)
			return this->firstSampleIndex_;
		timestamp_diff_type after = *it - t;		// Calculate the distance between the desired timestamp and the before/after values,
		timestamp_diff_type before = t - *(it-1);	// then return whichever index gets closer to the target.
		if(after < before)
			return (size_type)(it - timestamps_->begin()) + this->firstSampleIndex_;
		return (size_type)((--it) - timestamps_->begin()) + this->firstSampleIndex_;
	}

	void clear()
	{
		timestamps_->clear();
		buffer_->clear();
		numSamples_ = 0;
		firstSampleIndex_ = 0;
	}

	size_type size()
	{
		return buffer_->size();
	}					// Size: how many elements are currently in the buffer
	bool empty()
	{
		return buffer_->empty();
	}
	bool full()
	{
		return buffer_->full();
	}
	return_value_type operator [](size_type index)
	{
		return (*(this->buffer_))[index - this->firstSampleIndex_];
	}

	iterator begin()
	{
		return iterator(this, buffer_->begin());
	}
	iterator end()
	{
		return iterator(this, buffer_->end());
	}
	reverse_iterator rbegin()
	{
		return reverse_iterator(end());
	}
	reverse_iterator rend()
	{
		return reverse_iterator(begin());
	}

	// Two more convenience methods to avoid confusion about what front and back mean!
	return_value_type earliest()
	{
		return this->buffer_->front();
	}
	return_value_type latest()
	{
		return this->buffer_->back();
	}

	timestamp_type latestTimestamp()
	{
		return timestamps_->back();
	}
	timestamp_type earliestTimestamp()
	{
		return timestamps_->front();
	}


	size_type reserve() { return buffer_->reserve(); }				// Reserve: how many elements are left before the buffer is full
	size_type capacity() const { return buffer_->capacity(); }		// Capacity: how many elements could be in the buffer


	// Insert a new item into the buffer
	void insert(const T& item, timestamp_type timestamp)
	{
		if (this->buffer_->full())
			this->firstSampleIndex_++;
		this->timestamps_->push_back(timestamp);
		this->buffer_->push_back(item);
		this->numSamples_++;

		// Notify anyone who's listening for a trigger
//		this->sendTrigger(timestamp);
	}
private:
	timestamp_type insertMissingLastTimestamp_;
	juniper::circular_buffer<T>* buffer_;
	juniper::circular_buffer<timestamp_type>* timestamps_;
	size_type numSamples_;// How many samples total we've stored in this buffer
	size_type firstSampleIndex_;// Index of the first sample that still remains in the buffer
//	pthread_mutex_t bufferAccessMutex_ = PTHREAD_MUTEX_INITIALIZER;

};

}
#endif /* NODE_H_ */
