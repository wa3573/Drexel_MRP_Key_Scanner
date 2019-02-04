/*
 * MidiQueue.cpp
 *
 *  Created on: Feb 4, 2019
 *      Author: juniper
 */

#include "MidiQueue.h"

#ifdef ON_BELA
#include <libpd/z_libpd.h>
extern "C" {
#include <libpd/s_stuff.h>
};
#else
#include <z_libpd.h>
extern "C" {
#include <s_stuff.h>
};
#endif

MidiQueue* MidiQueue::instance_;
boost::circular_buffer<MidiMessage> MidiQueue::queue_;

MidiQueue::MidiQueue()
{

}

MidiQueue* MidiQueue::get_instance()
{
	if (instance_ == NULL) {
		instance_ = new MidiQueue();
	}

	return instance_;
}
void MidiQueue::push_back(MidiMessage message)
{
	queue_.push_back(message);
}

MidiMessage MidiQueue::pop_front()
{
	MidiMessage result = queue_.front();
	queue_.pop_front();
	return result;
}

void MidiQueue::process()
{
	MidiMessage currentMessage;

	for (unsigned int i = 0; i < queue_.size(); ++i) {
		currentMessage = queue_.front();
		queue_.pop_front();

		switch (currentMessage.getType()) {

#ifdef ON_BELA
		case kMidiMessageNoteOff:
			libpd_noteon(currentMessage.getChannel(), currentMessage.getNote(),
					0);
			break;
		case kMidiMessageNoteOn:
			libpd_noteon(currentMessage.getChannel(), currentMessage.getNote(),
					currentMessage.getVelocity());
			break;
#endif
		default:
			break;
		}
	}
}

void MidiQueue::prettyPrint(MidiMessage message)
{
	printf("Sending pd MIDI message: "
			"channel = %d, "
			"type = %d, "
			"pitch = %d, "
			"velocity = %d\n", message.getChannel(), message.getType(),
			message.getNote(), message.getVelocity());
}
