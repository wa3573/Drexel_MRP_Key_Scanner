/*
 * Time.h
 *
 *  Created on: Jan 15, 2019
 *      Author: juniper
 */

#ifndef UTILITY_TIME_H_
#define UTILITY_TIME_H_

#include <chrono>

namespace Time {
	inline double getMillisecondCounterHiRes()
	{
		return (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	}
};


#endif /* UTILITY_TIME_H_ */
