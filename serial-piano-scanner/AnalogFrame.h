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

class AnalogFrame {
public:
	AnalogFrame(unsigned char* frameBuffer);
	AnalogFrame();
	~AnalogFrame();

	void printFrame();

private:
	int parseFrame();

	int bufferLength;
	bool parseSuccessful;
	unsigned char* frameBuffer;
	int octave;
	uint32_t timestamp;
	std::vector<int16_t> data;
};

#endif /* ANALOGFRAME_H_ */
