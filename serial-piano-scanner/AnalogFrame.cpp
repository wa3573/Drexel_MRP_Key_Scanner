/*
 * AnalogFrame.cpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Juniper
 */

#include "AnalogFrame.h"

#include <iostream>
#include <string.h>

AnalogFrame::AnalogFrame(char* frameBuffer, int len)
{
	this->frame_buffer = frameBuffer;
	this->parse_successful = false;
	this->data = std::vector<float>();
	this->buffer_length = len;
	this->parseFrame();
}

AnalogFrame::AnalogFrame()
{
	this->frame_buffer = frame_buffer;
	this->parse_successful = false;
	this->data = std::vector<float>();
	this->timestamp = 0;
	this->octave = 0;
	this->offset = 0;
	this->buffer_length = 0;
}

AnalogFrame::~AnalogFrame()
{
}

int AnalogFrame::parseFrame()
{
	int count = 0;
	char c = 0x00;

	if (frame_buffer[count++] != kFrameTypeAnalog) {
		printf("Frame has wrong type, got '%d', breaking\n",
				this->frame_buffer[count]);

		return -1;
	}

	octave = frame_buffer[count++];
	memcpy(&timestamp, &frame_buffer[count], sizeof(timestamp));
	count += sizeof(timestamp);

	int16_t value;
	while (true) {
		c = frame_buffer[count];
		if (c == kControlCharacterFrameEnd) {
			printf("Reached end of frame, size of data: %d \n", data.size());
			parse_successful = true;
		}

		if (count > TOUCHKEY_MAX_FRAME_LENGTH) {
			printf("Maximum frame length exceeded \n");
			parse_successful = false;
		}

		memcpy(&value, &frame_buffer[count], sizeof(value));
		data.push_back(-1 * ((value / 4096.f) - 1));

		count++;
	}

	if (parse_successful) {
		return 0;
	} else {
		return -1;
	}
}

void AnalogFrame::printFrame()
{
	printf("== Analog Frame ==\n");
	if (this->parse_successful) {
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

