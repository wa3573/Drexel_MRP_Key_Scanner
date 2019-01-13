/*
 * Node.h
 *
 *  Created on: Jan 9, 2019
 *      Author: William Anderson
 */

#ifndef NODE_H_
#define NODE_H_

#include "circular_buffer.h"
#include "Types.h"

namespace juniper {
template<class T>
class Node {
public:
	typedef size_t size_type;
	typedef size_t capacity_type;
	typedef T return_value_type;

	explicit Node(capacity_type capacity) : insertMissingLastTimestamp_(0), buffer_(0), numSamples_(0), firstSampleIndex_(0)
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

	timestamp_type timestampAt(size_type index) { return (*timestamps_)[(index-this->firstSampleIndex_)]; }

	void clear() {
		timestamps_->clear();
		buffer_->clear();
		numSamples_ = 0;
		firstSampleIndex_ = 0;
	}

	size_type size() { return buffer_->size(); }					// Size: how many elements are currently in the buffer
	bool empty() { return buffer_->empty(); }
	bool full() { return buffer_->full(); }
	return_value_type operator [] (size_type index) { return (*(this->buffer_))[index-this->firstSampleIndex_]; }
	// Two more convenience methods to avoid confusion about what front and back mean!
	return_value_type earliest() { return this->buffer_->front(); }
	return_value_type latest() { return this->buffer_->back(); }

	timestamp_type latestTimestamp() { return timestamps_->back(); }
	timestamp_type earliestTimestamp() { return timestamps_->front(); }

	// Insert a new item into the buffer
	void insert(const T& item, timestamp_type timestamp) {
		if(this->buffer_->full())
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
	size_type numSamples_;							// How many samples total we've stored in this buffer
	size_type firstSampleIndex_;					// Index of the first sample that still remains in the buffer
//	pthread_mutex_t bufferAccessMutex_ = PTHREAD_MUTEX_INITIALIZER;

};

}
#endif /* NODE_H_ */
