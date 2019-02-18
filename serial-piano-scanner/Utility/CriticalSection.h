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
	}

	inline virtual ~CriticalSection() noexcept
	{

	}

	inline void enter() noexcept
	{
		pthread_mutex_lock(&mutex_);
	}

	inline bool tryEnter() noexcept
	{
		int ret = pthread_mutex_trylock(&mutex_);

		if (ret == 0) {
			return true;
		} else {
			return false;
		}
	}

	inline void exit() noexcept
	{
		pthread_mutex_unlock(&mutex_);
	}

private:
	pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;;
};

class ScopedLock : public CriticalSection {

public:
	inline ScopedLock() {
		this->enter();
	}

	inline ScopedLock(CriticalSection section) {
		section.enter();
	}

	inline ~ScopedLock() {
		this->exit();
	}
};


class WaitableEvent {
public:
	inline WaitableEvent(bool manualReset = false) noexcept
	{

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
