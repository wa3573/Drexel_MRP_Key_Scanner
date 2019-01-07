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
	bool isRunning();

private:
	int parseFrame();
	bool parseSuccessful;
	char* frameBuffer;
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
