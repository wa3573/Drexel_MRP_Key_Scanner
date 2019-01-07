/*
 * StatusFrame.cpp
 *
 *  Created on: Dec 28, 2018
 *      Author: William Anderson
 */

#include "StatusFrame.h"

#include <iostream>
#include <memory>
#include <bitset>

StatusFrame::StatusFrame(char* frameBuffer)
{
	this->frameBuffer = frameBuffer;
	this->parseSuccessful = false;
	this->keysConnected = std::vector();
	this->parseFrame();
}

StatusFrame::StatusFrame()
{
	this->frameBuffer = frameBuffer;
	this->parseSuccessful = false;
	this->keysConnected = std::vector();
	this->lowestHardwareNote = 0;
	this->octaves = 0;
	this->softwareVersionMajor = 0;
	this->softwareVersionMinor = 0;
	this->hardwareVersion = 0;
	this->flags = 0;
}

StatusFrame::~StatusFrame()
{
	delete this->keysConnected;
}

unsigned int StatusFrame::numKeysConnected()
{
	unsigned int count = 0;

	for (auto i : this->keysConnected) {
		count += i;
	}

	return count;
}

bool StatusFrame::isRunning()
{
	return this->flags & kStatusFlagRunning != 0;
}

int StatusFrame::parseFrame()
{
	int count = 0;
	char c = 0x00;

	if (frameBuffer[count++] != kFrameTypeStatus) {
		printf("Frame has wrong type, got '%d', breaking\n",
				this->frameBuffer[count]);

		return -1;
	}

	this->hardwareVersion = this->frameBuffer[count++]; // [1]
	this->softwareVersionMajor = this->frameBuffer[count++]; // [2]
	this->softwareVersionMinor = this->frameBuffer[count++]; // [3]
	this->flags = this->frameBuffer[count++]; // [4]
	this->octaves = this->frameBuffer[count++]; // [5]

	if (this->softwareVersionMajor >= 2) {
		// One extra byte holds lowest physical sensor
		this->lowestHardwareNote = this->frameBuffer[count++]; //[6]
		this->hasTouchSensors = ((frameBuffer[4] & kStatusFlagHasI2C) != 0);
		this->hasAnalogSensors = ((frameBuffer[4] & kStatusFlagHasAnalog) != 0);
		this->hasRGBLEDs = ((frameBuffer[4] & kStatusFlagHasRGBLED) != 0);
	} else {
		this->lowestHardwareNote = 0;
		this->hasTouchSensors = true;
		this->hasAnalogSensors = true;
		this->hasRGBLEDs = true;
		count++; // [6]
	}

	this->keysConnected.resize(this->octaves * 2);
	int oct = 0;
	while (oct <= this->octaves) {
		int16_t value = this->frameBuffer[count] * 256
				+ this->frameBuffer[count + 1];
		this->keysConnected[oct] = value;
		count += 2;
		oct++;
	}
//
//	unsigned int keyBytes = this->octaves * 2;
//
//	for (unsigned int n : keyBytes) {
//		this->keysConnected.push_back(this->frame_buffer[count++]);
//	}
//
//	if (this->frame_buffer[count++] != ESCAPE_CHARACTER) {
//		printf("Frame missing escape character, breaking\n");
//		return -1;
//	}
//
//	if (this->frame_buffer[count++] != kControlCharacterFrameEnd) {
//		printf("Frame missing frame end character, breaking\n");
//		return -1;
//	}

	this->parseSuccessful = true;
	return 0;
}

void StatusFrame::printFrame()
{
	printf("== Status Frame ==\n");
	if (this->parseSuccessful) {
		printf("Parse successful\n");
		printf("Hardware Version = %d\n", this->hardwareVersion);
		printf("Software Version Major = %d\n", this->softwareVersionMajor);
		printf("Software Version Minor = %d\n", this->softwareVersionMinor);
		printf("Flags = %d\n", this->flags);
		printf("Octaves = %d\n", this->octaves);
		printf("Lowest Hardware Note = %d\n", this->lowestHardwareNote);
		printf("Keys Connected (Per Octave) = ");
		for (auto key : this->keysConnected) {
			printf("[%d] ", key);
		}
		printf("Number of Keys Connected = %d\n", this->numKeysConnected());
	} else {
		printf("Parse unsuccessful\n");
		printf("Hardware Version = %d\n", this->hardwareVersion);
		printf("Software Version Major = %d\n", this->softwareVersionMajor);
		printf("Software Version Minor = %d\n", this->softwareVersionMinor);
		printf("Flags = %d\n", this->flags);
		printf("Octaves = %d\n", this->octaves);
		printf("Lowest Hardware Note = %d\n", this->lowestHardwareNote);
		printf("Keys Connected (Per Octave) = ");
		for (auto key : this->keysConnected) {
			printf("[%d] ", key);
		}
		printf("Number of Keys Connected = %d\n", this->numKeysConnected());
	}

}

