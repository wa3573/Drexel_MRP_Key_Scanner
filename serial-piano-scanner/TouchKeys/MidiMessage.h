/*
 * MidiMessage.h
 *
 *  Created on: Feb 4, 2019
 *      Author: juniper
 */

#ifndef TOUCHKEYS_MIDIMESSAGE_H_
#define TOUCHKEYS_MIDIMESSAGE_H_

// MIDI standard messages
enum {
	kMidiMessageNoteOff = 0x80,
	kMidiMessageNoteOn = 0x90,
	kMidiMessageAftertouchPoly = 0xA0,
	kMidiMessageControlChange = 0xB0,
	kMidiMessageProgramChange = 0xC0,
	kMidiMessageAftertouchChannel = 0xD0,
	kMidiMessagePitchWheel = 0xE0,
	kMidiMessageSysex = 0xF0,
	kMidiMessageSysexEnd = 0xF7,
	kMidiMessageActiveSense = 0xFE,
	kMidiMessageReset = 0xFF
};

enum {
	kMidiControlAllSoundOff = 120,
	kMidiControlAllControllersOff = 121,
	kMidiControlLocalControl = 122,
	kMidiControlAllNotesOff = 123
};

class MidiMessage {
public:
	MidiMessage(int channel, int type, int note, int velocity);
	MidiMessage();
	~MidiMessage();
	int getChannel() const;
	void setChannel(int channel);
	int getNote() const;
	int getNoteNumber() const;
	void setNote(int note);
	int getType() const;
	void setType(int type);
	int getVelocity() const;
	void setVelocity(int velocity);
	bool isNoteOff() const;
	bool isNoteOn() const;
	bool isAftertouch() const;
	bool isAllNotesOff() const;
	bool isAllSoundOff() const;
	bool isController() const;
	int getControllerNumber() const;
	int getControllerValue() const;
	bool isChannelPressure() const;
	int getChannelPressureValue() const;
	bool isPitchWheel() const;
	int getPitchWheelValue() const;
	static MidiMessage noteOn(int channel, int note, int velocity);
	static MidiMessage noteOff(int channel, int note);
	static MidiMessage noteOff(int channel, int note, int);
	static MidiMessage aftertouchChange(int channel, int note, int value);
	int getAfterTouchValue() const;
	int getRawDataSize() const;
	const unsigned char* getRawData() const;

private:
	int channel_;
	int type_;
	int note_;
	int velocity_;
	int controller_;
	int controllerValue_;
	int pitchWheelValue_;
};


#endif /* TOUCHKEYS_MIDIMESSAGE_H_ */
