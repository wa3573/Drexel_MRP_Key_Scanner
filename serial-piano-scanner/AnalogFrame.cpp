/*
 * AnalogFrame.cpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Juniper
 */

#include <iostream>
#include <string.h>
#include "AnalogFrame.hpp"



AnalogFrame::AnalogFrame(char* frameBuffer)
{
	this->frameBuffer = frameBuffer;
	this->parseSuccessful = false;
	this->data = new float(frameDataLength);
	this->parseFrame();
}

AnalogFrame::~AnalogFrame() {
	delete this->data;
}

int AnalogFrame::parseFrame()
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

	if (this->frameBuffer[len++] != kFrameTypeAnalog) {
		printf("Frame has wrong type, breaking\n");
		return -1;
	}

	this->octave = this->frameBuffer[len++];


	memcpy(&(this->timestamp), &(this->frameBuffer)[len], sizeof(this->timestamp));

	len += sizeof(this->timestamp);

	for (unsigned int n = 0; n < this->frameDataLength; ++n) {
		int16_t value;
		memcpy(&value, &(this->frameBuffer)[len], sizeof(value));
		this->data[n] = -1 * ((value / 4096.f) - 1);
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

void AnalogFrame::printFrame()
{
	printf("== Analog Frame ==\n");
	if (this->parseSuccessful) {
		printf("Octave = %d\n", this->octave);
		printf("Timestamp = %d\n", this->timestamp);
		printf("Values: \n");

		for (unsigned int n = 0; n < this->frameDataLength; ++n) {
			printf("[%.3f] ", this->data[n]);
			if (n == 11) {
				printf("\n");
			}
		}

		printf("\n");

	} else {
		printf("Parse unsuccessful\n");
	}

}

