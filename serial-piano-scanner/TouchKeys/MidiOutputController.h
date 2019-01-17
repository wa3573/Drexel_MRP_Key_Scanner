/*
  TouchKeys: multi-touch musical keyboard control software
  Copyright (c) 2013 Andrew McPherson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
  =====================================================================
 
  MidiOutputController.h: handles outgoing MIDI messages and manages output
  ports; uses Juce MIDI library functions.
*/

#ifndef MIDI_OUTPUT_CONTROLLER_H
#define MIDI_OUTPUT_CONTROLLER_H

#define NOT_ON_BELA

#include <map>
#include <vector>
#include <Midi.h>

#ifdef NOT_ON_BELA

inline Midi::Midi()
{

}

inline Midi::~Midi()
{

}

inline int Midi::writeTo(const char* port)
{
	return 0;
}

#endif

const std::string kMidiVirtualOutputName = "TouchKeys";

using namespace std;

class MidiOutputController {
	typedef char MidiOutput;

private:
    struct MidiOutputControllerRecord {
        int portNumber;
        // TODO: MidiOutput
//        MidiOutput *output;
//        const char* output;
    };
    
public:
    enum {
        kMidiVirtualOutputPortNumber = -2,
        kMidiOutputNotOpen = -1
    };
	
	// Constructor
	MidiOutputController();
	
	// Query available devices
	std::vector<pair<int, string> > availableMidiDevices();
	
	// Methods to connect/disconnect from a target port
    bool enablePort(int identifier, int deviceNumber);
#ifndef JUCE_WINDOWS
    bool enableVirtualPort(int identifier, const char *name);
#endif
	void disablePort(int identifier);
	void disableAllPorts();
    
    int enabledPort(int identifier);
    std::vector<std::pair<int, int> > enabledPorts();
	
    // Get the name of a particular port index
    std::string deviceName(int portNumber);
    
    // Find the index of a device with a given name; return -1 if not found
    int indexOfDeviceNamed(std::string const& name);
    
	// Send MIDI messages
	void sendNoteOn(int port, unsigned char channel, unsigned char note, unsigned char velocity);
    void sendNoteOff(int port, unsigned char channel, unsigned char note, unsigned char velocity = 64);
	void sendControlChange(int port, unsigned char channel, unsigned char control, unsigned char value);
	void sendProgramChange(int port, unsigned char channel, unsigned char value);
	void sendAftertouchChannel(int port, unsigned char channel, unsigned char value);
	void sendAftertouchPoly(int port, unsigned char channel, unsigned char note, unsigned char value);
	void sendPitchWheel(int port, unsigned char channel, unsigned int value);
	void sendReset(int port);
	
	// TODO: Generic pre-formed messages
//	void sendMessage(int port, const MidiMessage& message);
	
	// Destructor
	~MidiOutputController() { disableAllPorts(); }
	
private:
	/* Bela specific data members */
	const char* midiPort0_ = "hw:1,0,0";
	Midi midi_;

    std::map<int, MidiOutputControllerRecord> activePorts_;              // Destinations for MIDI data
};

#endif /* MIDI_OUTPUT_CONTROLLER_H */
