/*
 * SerialInterface.hpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Juniper
 */

#ifndef SERIALINTERFACE_H_
#define SERIALINTERFACE_H_

#include <unistd.h>
#include <array>

//#include "circular_buffer.h"
#include "TouchKeys/TouchkeyDevice.h"

class SerialInterface {
public:
	SerialInterface();
	SerialInterface(size_t BUFFER_SIZE);
	~SerialInterface();

	void setPostCallback(
			void (*postCallback)(void* arg, float* buffer, unsigned int length),
			void* arg);
	int set_interface_attribs(int fd, int speed);
	void set_mincount(int fd, int mcount);
	int initSerial(const char *portname, int speed);

//	int serialRead(juniper::circular_buffer<char>& buf, int timeoutMs);
	int serialRead(char* buf, size_t len, int timeoutMs);
	int serialWrite(const char* buf, size_t len);
	void serialCleanup();
	bool serialSetup(const char* device);

private:
	size_t BUFFER_SIZE = 1000;
};

#endif /* SERIALINTERFACE_H_ */
