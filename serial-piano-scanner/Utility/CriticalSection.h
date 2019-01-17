/*
 * CriticalSection.h
 *
 *  Created on: Jan 16, 2019
 *      Author: juniper
 */

#ifndef UTILITY_CRITICALSECTION_H_
#define UTILITY_CRITICALSECTION_H_

#include <pthread.h>

class CriticalSection {

public:
	inline CriticalSection() noexcept
	{
		mutex_ = PTHREAD_MUTEX_INITIALIZER;
	}

	inline virtual ~CriticalSection() noexcept
	{

	}

	inline void enter() const noexcept
	{
		pthread_mutex_lock(&mutex_);
	}

	inline bool tryEnter() const noexcept
	{
		int ret = pthread_mutex_trylock(&mutex_);

		if (ret == 0) {
			return true;
		} else {
			return false;
		}
	}

	inline void exit() const noexcept
	{
		pthread_mutex_unlock(&mutex_);
	}

private:
	mutable pthread_mutex_t mutex_;
};



#endif /* UTILITY_CRITICALSECTION_H_ */
