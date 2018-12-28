/*
 * SerialInterface.hpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Juniper
 */

#ifndef SERIALINTERFACE_HPP_
#define SERIALINTERFACE_HPP_

#include <unistd.h>
#include "TouchkeyDevice.h"

class SerialInterface {
public:
	SerialInterface();

	~SerialInterface();

	void setPostCallback(
			void (*postCallback)(void* arg, float* buffer, unsigned int length),
			void* arg);
	int set_interface_attribs(int fd, int speed);
	void set_mincount(int fd, int mcount);
	int initSerial(const char *portname, int speed);
	int serialRead(char* buf, size_t len, int timeoutMs);
	int serialWrite(const char* buf, size_t len);
	void serialCleanup();
	bool serialSetup(const char* device);
};

#endif /* SERIALINTERFACE_HPP_ */
