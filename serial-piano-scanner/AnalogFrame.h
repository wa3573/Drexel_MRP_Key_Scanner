/*
 * AnalogFrame.hpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Juniper
 */

#ifndef ANALOGFRAME_H_
#define ANALOGFRAME_H_

#include <vector>
#include <cstdint>
#include "TouchkeyDevice.h"

class AnalogFrame {
public:
	AnalogFrame(char* frameBuffer, int len);
	AnalogFrame();
	~AnalogFrame();

	void printFrame();

private:
	int parseFrame();

	int buffer_length;
	bool parse_successful;
	char* frame_buffer;
	unsigned int octave;
	uint32_t timestamp;
	int offset;
	std::vector<float> data;
};

#endif /* ANALOGFRAME_H_ */
