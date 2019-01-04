/*
 * StatusFrame.hpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Juniper
 */

#ifndef STATUSFRAME_H_
#define STATUSFRAME_H_

#include <vector>
#include "TouchkeyDevice.h"

class StatusFrame {
public:
	StatusFrame(char* frameBuffer);
	StatusFrame();
	~StatusFrame();

	unsigned int numKeysConnected();
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
	char* keysConnected;
};

#endif /* STATUSFRAME_H_ */
