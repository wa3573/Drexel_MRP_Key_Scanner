/*
 * CircularBuffer.cpp
 *
 *  Created on: Jan 4, 2019
 *      Author: William Anderson
 *
 *  Based on:
 *  https://embeddedartistry.com/blog/2017/4/6/circular-buffers-in-cc
 *  and
 *  https://accu.org/index.php/journals/389
 */

#include <cstddef>
#include <memory>
#include <pthread.h>
#include <iostream>

namespace juniper {

template<class T>
class circular_buffer {
public:
	class iterator {
	public:
		typedef circular_buffer<T> container_type;
		typedef T contained_type;
		typedef iterator self_type;
		typedef size_t size_type;
		typedef T value_type;
		typedef std::ptrdiff_t difference_type;
		typedef value_type& reference;
		typedef value_type& pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(container_type* buf, size_t start_pos) :
				buf_(buf), pos_(start_pos)
		{
		}

		contained_type& operator*()
		{
			return (*buf_)[pos_];
		}

		contained_type& operator->()
		{
			return &(operator*());
		}

		self_type& operator++()
		{
			++pos_;
			return *this;
		}

		self_type& operator++(int)
		{
			self_type tmp(*this);

			++(*this);
			return tmp;
		}

		//	SelfType operator+(difference_type n)
		//	{
		//		SelfType tmp(*this);
		//		tmp.pos_ += n;
		//		return tmp;
		//	}
		//
		//	SelfType &operator+=(difference_type n)
		//	{
		//		pos_ += n;
		//		return *this;
		//	}

		self_type& operator--()
		{
			--pos_;
			return *this;
		}

//		self_type& operator+(difference_type n)
//		{
//			self_type tmp(*this);
//			tmp.pos_ += n;
//			return tmp;
//		}
//
//		self_type& operator-(difference_type n)
//		{
//			self_type tmp(*this);
//			tmp.pos_ -= n;
//			return tmp;
//		}

		self_type& operator += (difference_type n) {		// it += n
			pos_ += n;
	        return *this;
	    }
		self_type& operator -= (difference_type n) {		// it -= n
			pos_ -= n;
	        return *this;
	    }

		self_type operator + (difference_type n) const { return self_type(*this) += n; }
		self_type operator - (difference_type n) const { return self_type(*this) -= n; }

		bool operator==(const self_type& other)
		{
			if (this->pos_ == other.pos_) {
				return true;
			}

			return false;
		}

		bool operator!=(const self_type& other)
		{
			if (this->pos_ != other.pos_) {
				return true;
			}

			return false;
		}

	private:
		container_type* buf_;
		size_t pos_;
	};

	class const_iterator {
	public:
		typedef const circular_buffer<T> container_type;
		typedef const T contained_type;
		typedef const_iterator self_type;
		typedef size_t size_type;
		typedef T value_type;
		typedef std::ptrdiff_t difference_type;
		typedef value_type& reference;
		typedef value_type& pointer;

		const_iterator(container_type* buf, size_t start_pos) :
				buf_(buf), pos_(start_pos)
		{
		}

		contained_type& operator*()
		{
			return (*buf_)[pos_];
		}

		contained_type& operator->()
		{
			return &(operator*());
		}

		self_type& operator++()
		{
			++pos_;
			return *this;
		}

		self_type& operator++(int)
		{
			self_type tmp(*this);

			++(*this);
			return tmp;
		}

		self_type operator+(difference_type n)
		{
			self_type tmp(*this);
			tmp.pos_ += n;
			return tmp;
		}

		self_type operator-(difference_type n)
		{
			self_type tmp(*this);
			tmp.pos_ -= n;
			return tmp;
		}

		self_type &operator+=(difference_type n)
		{
			pos_ += n;
			return *this;
		}

		self_type& operator--()
		{
			--pos_;
			return *this;
		}

		bool operator==(const self_type& other)
		{
			if (this->pos_ == other.pos_) {
				return true;
			}

			return false;
		}

		bool operator!=(const self_type& other)
		{
			if (this->pos_ != other.pos_) {
				return true;
			}

			return false;
		}

	private:
		container_type* buf_;
		size_t pos_;
	};

	class reverse_iterator {
	public:
		typedef circular_buffer<T> container_type;
		typedef T contained_type;
		typedef reverse_iterator self_type;
		typedef ptrdiff_t difference_type;
		typedef size_t size_type;
//		typedef std:: iterator_category;

		reverse_iterator(container_type* buf, size_t start_pos) :
				buf_(buf), pos_(start_pos)
		{
		}

		contained_type& operator*()
		{
			return (*buf_)[pos_];
		}

		contained_type& operator->()
		{
			return &(operator*());
		}

		self_type& operator++()
		{
			--pos_;
			return *this;
		}

		self_type& operator++(int)
		{
			self_type tmp(*this);

			--(*this);
			return tmp;
		}

		self_type operator+(difference_type n)
		{
			self_type tmp(*this);
			tmp.pos_ += n;
			return tmp;
		}

		self_type operator-(difference_type n)
		{
			self_type tmp(*this);
			tmp.pos_ -= n;
			return tmp;
		}

		self_type &operator+=(difference_type n)
		{
			pos_ += n;
			return *this;
		}

		self_type& operator--()
		{
			++pos_;
			return *this;
		}

		bool operator==(const self_type& other)
		{
			if (this->pos_ == other.pos_) {
				return true;
			}

			return false;
		}

		bool operator!=(const self_type& other)
		{
			if (this->pos_ != other.pos_) {
				return true;
			}

			return false;
		}

	private:
		container_type* buf_;
		size_t pos_;
	};

//	typedef std::reverse_iterator<iterator> reverse_iterator;
//	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	typedef size_t size_type;

	explicit circular_buffer(std::size_t size) :
			array_(std::unique_ptr<T[]>(new T[size])), array_size_(size), head_(
					0), tail_(0), contents_size_(0)
	{
	}

	void push_back(T item);
	void push_front(T item);
	T pop_front();
	T pop_back();

	T front();
	T back();
	void clear();
	bool empty() const;
	bool full() const;
	size_t capacity() const;
	size_t size() const;
	size_type reserve() { return capacity() - size(); }
	void resize(size_t size);
	void printHeadTail();
	T& operator[](size_t pos)
	{
		return this->array_[pos];
	}

//	const T &operator[](size_t);

	iterator begin()
	{
		return iterator(this, 0);
	}

	iterator end()
	{
		return iterator(this, this->size());
	}

	reverse_iterator rbegin()
	{
		return reverse_iterator(this, this->size() - 1);
	}

	/* FIXME: rend() should be one-past-the-first */
	reverse_iterator rend()
	{
		return reverse_iterator(this, -1);
	}

private:
	std::unique_ptr<T[]> array_;
	size_t array_size_;
	size_t head_;
	size_t tail_;
	size_t contents_size_;
	bool is_full_ = false;
	pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;

	void increment_tail()
	{
		++tail_;
		++contents_size_;

		if (tail_ == array_size_) {
			tail_ = 0;
		}
	}

	void increment_head()
	{
		// precondtion: !empty()
		++head_;
		--contents_size_;

		if (head_ == array_size_) {
			head_ = 0;
		}
	}
};

template<class T>
inline void circular_buffer<T>::resize(size_t size)
{
	array_ = std::make_unique<T[]>(size);
	array_size_ = size;
}

template<class T>
inline void circular_buffer<T>::push_back(T item)
{
	pthread_mutex_lock(&mutex_);
//	array_[head_] = item;
//
//	if (is_full_) {
//		tail_ = (tail_ + 1) % array_size_;
//	}
//
//	head_ = (head_ + 1) % array_size_;
//	is_full_ = (head_ == tail_);

//	increment_tail();
//
//	if (contents_size_ == array_size_) {
//		increment_head();
//	}
//
//	array_[tail_] = item;

	if (!contents_size_) {
		array_[head_] = item;
		tail_ = head_;
		++contents_size_;
	} else if (contents_size_ != array_size_) {
		increment_tail();
		array_[tail_] = item;
	} else {
		// We always accept data when full
		// and lose the front()
		increment_head();
		increment_tail();
		array_[tail_] = item;
	}

	pthread_mutex_unlock(&mutex_);
}

template<class T>
inline T circular_buffer<T>::pop_front()
{
	if (empty()) {
		return (T) NULL;
	}

	pthread_mutex_lock(&mutex_);
	auto value = front();
	increment_head();
	pthread_mutex_unlock(&mutex_);

	return value;
}

template<class T>
inline void circular_buffer<T>::clear()
{
	pthread_mutex_lock(&mutex_);
	head_ = 0;
	tail_ = 0;
	pthread_mutex_unlock(&mutex_);
}

template<class T>
inline bool circular_buffer<T>::empty() const
{
	return (!is_full_ && (head_ == tail_));
}

template<class T>
inline bool circular_buffer<T>::full() const
{
	return (contents_size_ == array_size_);
}

template<class T>
inline size_t circular_buffer<T>::capacity() const
{
	return array_size_;
}

template<class T>
inline size_t circular_buffer<T>::size() const
{
	return contents_size_;
}

template<class T>
inline T circular_buffer<T>::front()
{
	return array_[head_];
}

template<class T>
inline T circular_buffer<T>::back()
{
	return array_[tail_];
}

template<class T>
inline void circular_buffer<T>::printHeadTail()
{
	std::cout << "head_ = " << head_ << ", tail_ = " << tail_ << std::endl;

}

}
