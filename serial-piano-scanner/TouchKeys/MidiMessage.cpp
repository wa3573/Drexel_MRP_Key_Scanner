/*
 * MidiMessage.cpp
 *
 *  Created on: Feb 4, 2019
 *      Author: juniper
 */

#include "MidiMessage.h"
#include <cstddef>

MidiMessage::MidiMessage(int channel, int type, int note, int velocity) :
		channel_(channel), type_(type), note_(note), velocity_(velocity)
{

}

MidiMessage::MidiMessage() :
		channel_(0), type_(0), note_(0), velocity_(0)
{

}

MidiMessage::~MidiMessage()
{

}

int MidiMessage::getChannel() const
{
	return channel_;
}

void MidiMessage::setChannel(int channel)
{
	channel_ = channel;
}

int MidiMessage::getNote() const
{
	return note_;
}

int MidiMessage::getNoteNumber() const
{
	return note_;
}

void MidiMessage::setNote(int note)
{
	note_ = note;
}

int MidiMessage::getType() const
{
	return type_;
}

void MidiMessage::setType(int type)
{
	type_ = type;
}

int MidiMessage::getVelocity() const
{
	return velocity_;
}

void MidiMessage::setVelocity(int velocity)
{
	velocity_ = velocity;
}

bool MidiMessage::isNoteOff() const
{
	return (type_ == kMidiMessageNoteOff);
}

bool MidiMessage::isNoteOn() const
{
	return (type_ == kMidiMessageNoteOn);
}

bool MidiMessage::isAftertouch() const
{
	return (type_ == kMidiMessageAftertouchPoly);
}

bool MidiMessage::isAllNotesOff() const
{
	return (type_ == kMidiControlAllNotesOff);
}

bool MidiMessage::isAllSoundOff() const
{
	return (type_ == kMidiControlAllSoundOff);
}

bool MidiMessage::isController() const
{
	return (type_ == kMidiControlLocalControl);
}

int MidiMessage::getControllerNumber() const
{
	return controller_;
}

int MidiMessage::getControllerValue() const
{
	return controllerValue_;
}

bool MidiMessage::isChannelPressure() const
{
	return false;
}

int MidiMessage::getChannelPressureValue() const
{
	return 0;
}

bool MidiMessage::isPitchWheel() const
{
	return (type_ == kMidiMessagePitchWheel);
}

int MidiMessage::getPitchWheelValue() const
{
	return pitchWheelValue_;
}

MidiMessage MidiMessage::noteOn(int channel, int note, int velocity)
{
	return MidiMessage(channel, kMidiMessageNoteOn, note, velocity);
}

MidiMessage MidiMessage::noteOff(int channel, int note)
{
	return MidiMessage(channel, kMidiMessageNoteOff, note, 0);
}

MidiMessage MidiMessage::noteOff(int channel, int note, int)
{
	return MidiMessage(channel, kMidiMessageNoteOff, note, 0);
}

MidiMessage MidiMessage::aftertouchChange(int channel, int note,
		int value)
{
	return MidiMessage(channel, kMidiMessageAftertouchPoly, note, value);
}

int MidiMessage::getAfterTouchValue() const
{
	return velocity_;
}

int MidiMessage::getRawDataSize() const
{
	return 0;
}

const unsigned char* MidiMessage::getRawData() const
{
	return NULL;
}
