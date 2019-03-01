/*
 * PianoKeyInfo.h
 *
 *  Created on: Feb 26, 2019
 *      Author: juniper
 */

#ifndef TOUCHKEYS_PIANOKEYINFO_H_
#define TOUCHKEYS_PIANOKEYINFO_H_

#include "../Utility/Thread.h"
//#include "PianoKeyboard.h"
#include "TouchkeyDevice.h"
#include <vector>
#include <map>

class PianoKeyInfo : public Thread {
private:
	TouchkeyDevice& touchkeyDevice_;
	PianoKeyboard& pianoKeyboard_;
	std::vector<PianoKey*> pianoKeys_;
	std::map<PianoKey*, int> noteNumbers_;
	int updatePeriod_ = 1e+6;


public:
	inline PianoKeyInfo(TouchkeyDevice& touchkeyDevice, PianoKeyboard& pianoKeyboard)
		: Thread("PianoKeyInfoThread"), touchkeyDevice_(touchkeyDevice), pianoKeyboard_(pianoKeyboard)
	{
	}

	inline void setPianoKeys(std::vector<int> pianoKeys)
	{
		for (auto key : pianoKeys) {
			pianoKeys_.push_back(pianoKeyboard_.key(key));
			noteNumbers_[pianoKeyboard_.key(key)] = key;
		}
	}

	inline void setUpdatePeriod(int period)
	{
		updatePeriod_ = period;
	}

	inline void startThread()
	{
//		thread_function_ptr_t p = (thread_function_ptr_t)&ExampleThread::run;
		int ret1 = pthread_create(getPthread(), NULL,
//				(thread_function_ptr_t) &ExampleThread::run, (void*) this);
				run_static, (void*) this);
		if (ret1) {
			fprintf(stderr, "Error - pthread_create() return code: %d\n", ret1);
		} else {
			init();
		}
	}

	inline static void* run_static(void* args)
	{
		PianoKeyInfo* p = (PianoKeyInfo*) args;
		p->run();

		return NULL;
	}

	inline void* run()
	{
		if (pianoKeys_.empty()) {
			std::cout << "PianoKeyInfo Thread: No keys to monitor, exiting..." << std::endl;
			return NULL;
		}

		while (!threadShouldExit()) {
			for (auto key : pianoKeys_) {
				int noteNumber = noteNumbers_[key];
				key_position latestPosition = key->buffer().latest();
//				key_position calibratedPosition = touchkeyDevice_.getCalibrator(noteNumber)->evaluate(latestPosition);
				std::cout << "Key " << noteNumber << " raw position = " << latestPosition << std::endl;
//						", calibrated position = " << calibratedPosition << std::endl;
			}

			usleep(updatePeriod_);
		}

		return NULL;
	}
};



#endif /* TOUCHKEYS_PIANOKEYINFO_H_ */
