/*
 * Thread.h
 *
 *  Created on: Jan 15, 2019
 *      Author: juniper
 */

#ifndef UTILITY_THREAD_H_
#define UTILITY_THREAD_H_

#include <iostream>
#include <vector>

namespace juniper {

inline pthread_t getCurrentThreadId()
{
	return pthread_self();
}

}

class Thread {
public:
	typedef void * (*thread_function_ptr_t)(void *);

	inline Thread(std::string threadName) : name_(threadName)
	{

	}

	virtual void* run()
	{
		printf("Thread::run()\n");
		return NULL;
	}

	pthread_t getThreadId()
	{
		return pthread_;
	}

//	void startThread(T* enclosing);
	void stopThread(int timeOutMilliseconds);
	void exit();
	void signalThreadShouldExit();
	bool waitForThreadToExit();
	bool wait(int timeOutMilliseconds) const;
	void notify();
	void init();
	bool isThreadRunning()
	{
		return isRunning_;
	}

	bool threadShouldExit()
	{
		return threadShouldExit_;
	}

	pthread_t* getPthread();

	virtual ~Thread();

private:
	pthread_t pthread_;
	std::string name_;
	bool threadShouldExit_;
	bool isRunning_;
};

class ExampleThread: public Thread {
public:
	ExampleThread(std::string threadName) :
			Thread(threadName)
	{
	}

	void startThread()
	{
		int ret1 = pthread_create(getPthread(), NULL,
				(thread_function_ptr_t) &ExampleThread::run, (void*) this);
		if (ret1) {
			fprintf(stderr, "Error - pthread_create() return code: %d\n", ret1);
		} else {
			init();
		}
	}

	void* run()
	{
		printf("Hello from an ExampleThread!\n");

		this->exit();

		return NULL;
	}
};

class ThreadHandler: Thread {

	void* run()
	{
		return NULL;
	}

	void startThread(Thread& thread);
	void stopAllThreads();
	void waitForAllThreadsToExit();

private:
	std::vector<Thread> threads_;
};

#endif /* UTILITY_THREAD_H_ */
