/*
 * StatusFrame.hpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Juniper
 */

#ifndef STATUSFRAME_H_
#define STATUSFRAME_H_

#include <vector>

#include "TouchKeys/TouchkeyDevice.h"

class StatusFrame {
public:
	StatusFrame(unsigned char* frameBuffer);
	StatusFrame();
	~StatusFrame();

	unsigned int numKeysConnected();
	void printFrame();
	bool isRunning();

private:
	int parseFrame();
	bool parseSuccessful;
	unsigned char* frameBuffer;
	int hardwareVersion;
	int softwareVersionMajor;
	int softwareVersionMinor;
	int flags;
	int octaves;
	int lowestHardwareNote;
	std::vector<unsigned int> keysConnected;
	bool hasTouchSensors;
	bool hasAnalogSensors;
	bool hasRGBLEDs;
};

#endif /* STATUSFRAME_H_ */
