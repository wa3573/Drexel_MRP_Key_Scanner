/*
 * AnalogFrame.hpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Juniper
 */

#ifndef ANALOGFRAME_HPP_
#define ANALOGFRAME_HPP_

#include <vector>
#include "TouchkeyDevice.h"

class AnalogFrame {
public:
	AnalogFrame(char* frameBuffer);
	~AnalogFrame();

	void printFrame();

private:
	int parseFrame();

	unsigned int frameDataLength = 25;
	bool parseSuccessful;
	char* frameBuffer;
	unsigned int octave;
	uint32_t timestamp;
	int offset;
	float* data;
};

#endif /* ANALOGFRAME_HPP_ */
