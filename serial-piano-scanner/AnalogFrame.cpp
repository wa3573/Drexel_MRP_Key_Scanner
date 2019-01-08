/*
 * AnalogFrame.cpp
 *
 *  Created on: Dec 28, 2018
 *      Author: William Anderson
 */

#include "AnalogFrame.h"

#include <iostream>
#include <string.h>

AnalogFrame::AnalogFrame(char* frameBuffer)
{
	this->frameBuffer = frameBuffer;
	this->parseSuccessful = false;
	this->data = std::vector<int16_t>();
	this->parseFrame();
}

AnalogFrame::AnalogFrame()
{
	this->frameBuffer = frameBuffer;
	this->parseSuccessful = false;
	this->data = std::vector<int16_t>();
	this->timestamp = 0;
	this->octave = 0;
	this->bufferLength = 0;
}

AnalogFrame::~AnalogFrame()
{
}

int AnalogFrame::parseFrame()
{
	int count = 0;
	char c = 0x00;

	if (frameBuffer[count++] != kFrameTypeAnalog) {
		printf("Frame has wrong type, got '%d', breaking\n",
				this->frameBuffer[count]);

		return -1;
	}

	octave = frameBuffer[count++];
	memcpy(&timestamp, &frameBuffer[count], sizeof(timestamp));
	count += sizeof(timestamp);

	int16_t value;
	/* based on touchkeys source */

	for (int key = 0; key < 25; ++key) {
		c = frameBuffer[count];
		if (c == kControlCharacterFrameEnd) {
			printf("Reached end of frame, size of data: %d \n", data.size());
			parseSuccessful = true;
		}

		if (count > TOUCHKEY_MAX_FRAME_LENGTH) {
			printf("Maximum frame length exceeded \n");
			parseSuccessful = false;
		}

        // Every analog frame contains 25 values, however only the top board actually uses all 25
        // sensors. There are several "high C" values in the lower boards (i.e. key == 24) which
        // do not correspond to real sensors. These should be ignored.
        if(key == 24 && octave != 6)
            continue;

        // Pull the value out from the packed buffer (little endian 16 bit)
        value = (((signed char) frameBuffer[key * 2 + 6]) * 256 + frameBuffer[key * 2 + 5]);

        data.push_back(value);
        count += 2;
	}
//
//
//	while (true) {
//		c = frame_buffer[count];
//		if (c == kControlCharacterFrameEnd) {
//			printf("Reached end of frame, size of data: %d \n", data.size());
//			parse_successful = true;
//		}
//
//		if (count > TOUCHKEY_MAX_FRAME_LENGTH) {
//			printf("Maximum frame length exceeded \n");
//			parse_successful = false;
//		}
//
//		memcpy(&value, &frame_buffer[count], sizeof(value));
//		data.push_back(-1 * ((value / 4096.f) - 1));
//
//		count++;
//	}

	if (parseSuccessful) {
		return 0;
	} else {
		return -1;
	}
}

void AnalogFrame::printFrame()
{
	printf("== Analog Frame ==\n");
	if (this->parseSuccessful) {
		printf("Parse successful\n");
		printf("Octave = %d\n", this->octave);
		printf("Timestamp = %d\n", this->timestamp);
		printf("Values: \n");

		for (auto i : this->data) {
			printf("[%.3f] ", i);
		}

		printf("\n");

	} else {
		printf("Parse unsuccessful\n");
		printf("Octave = %d\n", this->octave);
		printf("Timestamp = %d\n", this->timestamp);
		printf("Values: \n");

		for (auto i : this->data) {
			printf("[%.3f] ", i);
		}
	}

}

