/*
 * CircularBuffer.cpp
 *
 *  Created on: Jan 4, 2019
 *      Author: William Anderson
 *
 *  Based on:
 *  https://embeddedartistry.com/blog/2017/4/6/circular-buffers-in-cc
 *
 */

#include <cstddef>
#include <memory>
#include <pthread.h>

template<class T, typename E>
class CircularBufferIterator {
public:
	typedef T CircularBuffer;
	typedef CircularBufferIterator SelfType;

	CircularBufferIterator(CircularBuffer* buf, size_t start_pos) :
			buf_(buf), pos_(start_pos)
	{
	}

	E& operator*()
	{
		return (*buf_)[pos_];
	}

	E& operator->()
	{
		return &(operator*());
	}

	SelfType& operator++()
	{
		++pos_;
		return *this;
	}

	SelfType& operator++(int)
	{
		SelfType tmp(*this);

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

private:
	CircularBuffer* buf_;
	size_t pos_;
};

template<class T>
class CircularBuffer {
public:
	typedef std::reverse_iterator<CircularBufferIterator<CircularBuffer, T>> ReverseIterator;

	explicit CircularBuffer(std::size_t size) :
			buf_(std::unique_ptr<T[]>(new T[size])), max_size_(size)
	{
	}

	void put(T item);
	T get();
	void reset();
	bool is_empty() const;
	bool is_full() const;
	size_t capacity() const;
	size_t size() const;
	void resize(size_t size);
//	TODO: implement operator[] ?
	T& operator[](size_t pos)
	{
		return this->buf_[pos];
	}
//	const T &operator[](size_t);
	CircularBufferIterator<CircularBuffer, T> begin()
	{
		return CircularBufferIterator<CircularBuffer, T>(this, 0);
	}

	CircularBufferIterator<CircularBuffer, T> end()
	{
		return CircularBufferIterator<CircularBuffer, T>(this, this->size());
	}

	CircularBufferIterator<CircularBuffer, T> rbegin()
	{
		return CircularBufferIterator<CircularBuffer, T>(this, this->size());
	}

	/* FIXME: rend() should be one-past-the-first */
	CircularBufferIterator<CircularBuffer, T> rend()
	{
		return CircularBufferIterator<CircularBuffer, T>(this, 0);
	}


private:
	std::unique_ptr<T[]> buf_;
	size_t head_ = 0;
	size_t tail_ = 0;
	const size_t max_size_;
	bool is_full_ = false;
	pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;
};

template<class T>
inline void CircularBuffer<T>::resize(size_t size)
{
	buf_ = std::make_unique<T[]>(size);
	max_size_ = size;
}

template<class T>
inline void CircularBuffer<T>::put(T item)
{
	pthread_mutex_lock(&mutex_);
	buf_[head_] = item;

	if (is_full_) {
		tail_ = (tail_ + 1) % max_size_;
	}

	head_ = (head_ + 1) % max_size_;
	is_full_ = (head_ == tail_);
	pthread_mutex_unlock(&mutex_);
}

template<class T>
inline T CircularBuffer<T>::get()
{
	if (is_empty()) {
		return T();
	}

	pthread_mutex_lock(&mutex_);
	auto value = buf_[tail_];
	is_full_ = false;
	tail_ = (tail_ + 1) % max_size_;
	pthread_mutex_unlock(&mutex_);

	return value;
}

template<class T>
inline void CircularBuffer<T>::reset()
{
	pthread_mutex_lock(&mutex_);
	head_ = tail_;
	is_full_ = false;
	pthread_mutex_unlock(&mutex_);
}

template<class T>
inline bool CircularBuffer<T>::is_empty() const
{
	return (!is_full_ && (head_ == tail_));
}

template<class T>
inline bool CircularBuffer<T>::is_full() const
{
	return is_full_;
}

template<class T>
inline size_t CircularBuffer<T>::capacity() const
{
	return max_size_;
}

template<class T>
inline size_t CircularBuffer<T>::size() const
{
	size_t size = max_size_;

	if (!is_full_) {
		if (head_ >= tail_) {
			size = head_ - tail_;
		} else {
			size = max_size_ + head_ - tail_;
		}
	}

	return size;
}
