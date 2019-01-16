/*
 * Thread.cpp
 *
 *  Created on: Jan 15, 2019
 *      Author: juniper
 */

#include "Thread.h"
#include <unistd.h>

Thread::Thread(std::string threadName)
{
	name_ = threadName;
}

//void Thread::startThread()
//{
//	int ret1 = pthread_create(getPthread(), NULL, (thread_function_ptr_t)&Thread::run, (void*) this);
//	if (ret1) {
//		fprintf(stderr, "Error - pthread_create() return code: %d\n", ret1);
//	} else {
//		init();
//	}
//}

void Thread::init()
{
	isRunning_ = true;
	threadShouldExit_ = false;
}

pthread_t* Thread::getPthread()
{
	return &pthread_;
}

void Thread::signalThreadShouldExit()
{
	threadShouldExit_ = true;
}

void Thread::stopThread(int timeOutMilliseconds)
{
	threadShouldExit_ = true;
}

void Thread::exit()
{
	isRunning_ = false;
}

bool Thread::waitForThreadToExit()
{
	while (isRunning_) {
		usleep(1000);
	}

	return true;
}

Thread::~Thread()
{

}

