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
	this->keysConnected = NULL;
	this->parseFrame();
}

StatusFrame::StatusFrame()
{
	this->frameBuffer = frameBuffer;
	this->parseSuccessful = false;
	this->keysConnected = NULL;
	this->lowestHardwareNote = 0;
	this->octaves = 0;
	this->softwareVersionMajor = 0;
	this->softwareVersionMinor = 0;
	this->hardwareVersion = 0;
	this->flags = 0;
}

StatusFrame::~StatusFrame() {
	delete this->keysConnected;
}

unsigned int StatusFrame::numKeysConnected()
{
	unsigned int count = 0;

	for (unsigned int n = 0; n < (this->octaves * 2); ++n) {
		std::bitset<4> bitset(this->keysConnected[n]);
		count += bitset.count();
	}

	return count;
}

int StatusFrame::parseFrame()
{
	int len = 0;

	if (this->frameBuffer[len++] != ESCAPE_CHARACTER) {
		printf("Frame missing escape character, breaking\n");
		return -1;
	}

	if (this->frameBuffer[len++] != kControlCharacterFrameBegin) {
		printf("Frame missing frame begin character, breaking\n");
		return -1;
	}

	if (this->frameBuffer[len++] != kFrameTypeStatus) {
		printf("Frame has wrong type, breaking\n");
		return -1;
	}

	this->hardwareVersion = this->frameBuffer[len++];
	this->softwareVersionMajor = this->frameBuffer[len++];
	this->softwareVersionMinor = this->frameBuffer[len++];
	this->flags = this->frameBuffer[len++];
	this->octaves = this->frameBuffer[len++];
	this->lowestHardwareNote = this->frameBuffer[len++];

	this->keysConnected = new char[this->octaves * 2]();
	for (unsigned int n = 0; n < (this->octaves * 2); ++n) {
		this->keysConnected[n] = this->frameBuffer[len++];
	}

	if (this->frameBuffer[len++] != ESCAPE_CHARACTER) {
		printf("Frame missing escape character, breaking\n");
		return -1;
	}

	if (this->frameBuffer[len++] != kControlCharacterFrameEnd) {
		printf("Frame missing frame end character, breaking\n");
		return -1;
	}

	this->parseSuccessful = true;
	return 0;
}

void StatusFrame::printFrame()
{
	printf("== Status Frame ==\n");
	if (this->parseSuccessful) {
		printf("Hardware Version = %d\n", this->hardwareVersion);
		printf("Software Version Major = %d\n", this->softwareVersionMajor);
		printf("Software Version Minor = %d\n", this->softwareVersionMinor);
		printf("Flags = %d\n", this->flags);
		printf("Octaves = %d\n", this->octaves);
		printf("Lowest Hardware Note = %d\n", this->lowestHardwareNote);
		printf("Keys Connected (HEX) = %X\n", this->keysConnected);
	} else {
		printf("Parse unsuccessful\n");
	}

}

