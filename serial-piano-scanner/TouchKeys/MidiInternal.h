/*
 * MidiInternal.h
 *
 *  Created on: Feb 3, 2019
 *      Author: juniper
 */

#ifndef TOUCHKEYS_MIDIINTERNAL_H_
#define TOUCHKEYS_MIDIINTERNAL_H_

#include <vector>
#include "MidiQueue.h"

class MidiOutput {
public:
	inline MidiOutput(const char* name) :
			deviceName_(name)
	{

	}

	inline MidiOutput()
	{

	}

	inline ~MidiOutput()
	{

	}

	inline void sendMessageNow(const MidiMessage& message)
	{
		/* TODO: this should insert the MidiMessage into a queue
		 * to be handled in the audio thread
		 */

		midiQueue_->push_back(message);
	}

	static inline void setDeviceNames(std::vector<std::string>& devices)
	{
		deviceNames_ = devices;
	}

	static inline std::vector<std::string> getDevices()
	{
		return deviceNames_;
	}

	static inline MidiOutput createNewDevice(const char* name)
	{
		MidiOutput midiOutput = MidiOutput(name);
		midiOutput_ = midiOutput;
		return MidiOutput(name);
	}

	static inline MidiOutput openDevice(int deviceNumber)
	{
//		const char* deviceName = deviceNames_->at(deviceNumber).c_str();
		return midiOutput_;
	}

	inline static const MidiQueue* getMidiQueue()
	{
		return midiQueue_;
	}

	inline static void setMidiQueue(MidiQueue* midiQueue)
	{
		midiQueue_ = midiQueue;
	}

public:
	static std::vector<std::string> deviceNames_;
	/* We only support one midiOutput at the moment */
	static MidiOutput midiOutput_;
	static MidiQueue* midiQueue_;

private:
	const char* deviceName_;
};

class MidiInputCallback;

class MidiInput {
public:
	inline MidiInput()
	{

	}

	inline ~MidiInput()
	{

	}

	inline static std::vector<std::string> getDevices()
	{
		return std::vector<std::string>();
	}

	inline static MidiInput* openDevice(int deviceIndex, MidiInputCallback* callback)
	{
		return NULL;
	}

	inline void start()
	{

	}

	inline void stop()
	{

	}
};

class MidiInputCallback {
public:
	inline virtual ~MidiInputCallback()
	{

	}

	inline virtual void handleIncomingMidiMessage(MidiInput *source,
			const MidiMessage &message)
	{

	}

	inline virtual void handlePartialSysexMessage(MidiInput *source,
			const uint8_t *messageData, int numBytesSoFar, double timestamp)
	{

	}
};
#endif /* TOUCHKEYS_MIDIINTERNAL_H_ */
