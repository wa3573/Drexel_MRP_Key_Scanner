/*
 * CriticalSection.h
 *
 *  Created on: Jan 16, 2019
 *      Author: juniper
 */

#ifndef UTILITY_CRITICALSECTION_H_
#define UTILITY_CRITICALSECTION_H_

#include <pthread.h>
#include "Time.h"
#include <time.h>

class CriticalSection {

public:
	inline CriticalSection() noexcept
	{
		mutex_ = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
		attr_ = (pthread_mutexattr_t*) malloc(sizeof(pthread_mutexattr_t));

		pthread_mutexattr_init(attr_);
		pthread_mutexattr_settype(attr_, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(mutex_, attr_);
	}

	inline ~CriticalSection() noexcept
	{
		pthread_mutex_destroy(mutex_);
		free(mutex_);
		free(attr_);
	}

	inline void enter() noexcept
	{
		pthread_mutex_lock(mutex_);
	}

	inline bool tryEnter() noexcept
	{
		int ret = pthread_mutex_trylock(mutex_);

		if (ret == 0) {
			return true;
		} else {
			return false;
		}
	}

	inline void exit() noexcept
	{
		pthread_mutex_unlock(mutex_);
	}

private:
	pthread_mutex_t* mutex_;
	pthread_mutexattr_t* attr_;

};

class ScopedLock {

public:
	inline ScopedLock(CriticalSection& section)
		: criticalSection_(section)
	{
		criticalSection_.enter();
	}

	inline ~ScopedLock() {
		criticalSection_.exit();
	}

private:

	CriticalSection& criticalSection_;
};


class WaitableEvent {
public:
	inline WaitableEvent(bool manualReset = false) noexcept
	{
		pthread_cond_init(&cv_, NULL);
	}

	inline ~WaitableEvent()
	{

	}

	inline bool wait(int timeOutMilliseconds = -1) noexcept
	{
		struct timespec timeToWait;
		int ret = 0;
		clock_gettime(CLOCK_REALTIME, &timeToWait);

		timeToWait.tv_nsec += timeOutMilliseconds * 1000;
		timeToWait.tv_sec = (timeToWait.tv_nsec + 1000UL * timeOutMilliseconds) * 1000UL;

		pthread_mutex_lock(&mutex_);
		while (!signaled_ && ret == 0) {
			ret = pthread_cond_timedwait(&cv_, &mutex_, &timeToWait);
		}
		pthread_mutex_unlock(&mutex_);

		return true;
	}

	inline void signal() noexcept
	{
		signaled_ = true;
		pthread_cond_signal(&cv_);
	}

	inline void reset() noexcept
	{
		signaled_ = false;
	}

private:
	bool signaled_ = false;
	pthread_cond_t cv_;
	pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;

	inline double currentTime()
	{
		return Time::getMillisecondCounterHiRes();
	}
};

#endif /* UTILITY_CRITICALSECTION_H_ */
