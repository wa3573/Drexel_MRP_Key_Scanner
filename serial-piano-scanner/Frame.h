/*
 * Frame.h
 *
 *  Created on: Jan 4, 2019
 *      Author: juniper
 */

#include <vector>

#ifndef FRAME_H_
#define FRAME_H_

class Frame {
public:
	Frame();
	Frame(std::vector<char> frameData);
	virtual ~Frame();
};

#endif /* FRAME_H_ */
