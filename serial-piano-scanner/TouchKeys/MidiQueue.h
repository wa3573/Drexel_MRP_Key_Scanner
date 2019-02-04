/*
 * MidiQueue.h
 *
 *  Created on: Feb 3, 2019
 *      Author: juniper
 */

#ifndef MIDIQUEUE_H_
#define MIDIQUEUE_H_

#include <boost/circular_buffer.hpp>
#include "MidiMessage.h"

#undef DEBUG_MIDIQUEUE

class MidiQueue {
public:
	MidiQueue();

	static MidiQueue* get_instance();
	static void push_back(MidiMessage message);
	static MidiMessage pop_front();
	static void process();
	static void prettyPrint(MidiMessage message);

private:
	static MidiQueue* instance_;
	static boost::circular_buffer<MidiMessage> queue_;
};

#endif /* MIDIQUEUE_H_ */
