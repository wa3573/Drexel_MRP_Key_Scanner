/*
 * StatusFrame.hpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Juniper
 */

#ifndef STATUSFRAME_HPP_
#define STATUSFRAME_HPP_

#include <vector>
#include "TouchkeyDevice.h"

class StatusFrame {
public:
	StatusFrame(char* frameBuffer);
	~StatusFrame();

	void printFrame();

private:
	int parseFrame();
	bool parseSuccessful;
	char* frameBuffer;
	unsigned int hardwareVersion;
	unsigned int softwareVersionMajor;
	unsigned int softwareVersionMinor;
	unsigned int flags;
	unsigned int octaves;
	unsigned int lowestHardwareNote;
	unsigned char* keysConnected;
};

#endif /* STATUSFRAME_HPP_ */
