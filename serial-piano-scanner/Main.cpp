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
 
  Main.cpp: main startup routines, connecting to Juce library
*/

#include "MainApplicationController.h"

#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
//#include "TouchKeys/MidiQueue.h"

bool programShouldStop_ = false;
int gXenomaiInited = 0; // required by libbelaextra
unsigned int gAuxiliaryTaskStackSize = 1 << 17; // required by libbelaextra
const int kCalibrationTimeSeconds = 10;
int gVerboseLevel = 0;

MidiQueue* gMidiQueue = MidiQueue::get_instance();
std::vector<std::string> MidiOutput::deviceNames_;
MidiOutput MidiOutput::midiOutput_;
MidiQueue* MidiOutput::midiQueue_;

static struct option long_options[] = {
	{"help", no_argument, NULL, 'h'},
	{"list", no_argument, NULL, 'l'},
	{"touchkeys", required_argument, NULL, 't'},
    {"midi-input", required_argument, NULL, 'i'},
    {"midi-output", required_argument, NULL, 'o'},
    {"virtual-midi-output", no_argument, NULL, 'V'},
    {"osc-input-port", required_argument, NULL, 'P'},
	{0,0,0,0}
};

void sigint_handler(int s){
    programShouldStop_ = true;
}

void usage(const char * processName)	// Print usage information and exit
{
	cerr << "Usage: " << processName << " [-h] [-l] [-t touchkeys] [-i MIDI-in] [-o MIDI-out]\n";
	cerr << "  -h:   Print this menu\n";
	cerr << "  -l:   List available TouchKeys and MIDI devices\n";
	cerr << "  -t:   Specify TouchKeys device path and autostart\n";
    cerr << "  -i:   Specify MIDI input device\n";
    cerr << "  -o:   Specify MIDI output device\n";
    cerr << "  -V:   Open virtual MIDI output\n";
    cerr << "  -P:   Specify OSC input port (default: " << kDefaultOscReceivePort << ")\n";
}

void list_devices(MainApplicationController& controller)
{
    std::vector<std::string> touchkeysDevices(controller.availableTouchkeyDevices());
//    std::vector<std::pair<int, std::string> > midiInputDevices(controller.availableMIDIInputDevices());
    std::vector<std::pair<int, std::string> > midiOutputDevices(controller.availableMIDIOutputDevices());
    
    cerr << "TouchKeys devices: \n";
    if(touchkeysDevices.empty())
        cerr << "  [none found]\n";
    else {
        for(std::vector<std::string>::iterator it = touchkeysDevices.begin(); it != touchkeysDevices.end(); ++it) {
            cerr << "  /dev/" << *it << "\n";
        }
    }

//    cerr << "\nMIDI input devices: \n";
//    if(midiInputDevices.empty())
//        cerr << "  [none found]\n";
//    else {
//        for(std::vector<std::pair<int, std::string> >::iterator it = midiInputDevices.begin();
//            it != midiInputDevices.end();
//            ++it) {
//            cerr << "  " << it->first << ": " << it->second << "\n";
//        }
//    }
    
    cerr << "\nMIDI output devices: \n";
    if(midiOutputDevices.empty())
        cerr << "  [none found]\n";
    else {
        for(std::vector<std::pair<int, std::string> >::iterator it = midiOutputDevices.begin();
            it != midiOutputDevices.end();
            ++it) {
            cerr << "  " << it->first << ": " << it->second << "\n";
        }
    }
}

int main (int argc, char* argv[])
{
    MainApplicationController controller;
    
    int ch, option_index;
    int midiInputNum = 0, midiOutputNum = 0;
    bool useVirtualMidiOutput = false;
    bool shouldStart = true;
    bool autostartTouchkeys = false;
    bool autoopenMidiOut = false, autoopenMidiIn = false;
    int oscInputPort = kDefaultOscReceivePort;
    string touchkeysDevicePath;
    string midiOutputName = "hw:0:0:0"; // Legacy MIDI
    string oscHost = "127.0.0.1"; // Address to transmit OSC messages to
    string oscPort = "8001"; // Port for that address
    size_t midiQueueSize = 1; // Will likely be unused

    printf("Touchkeys Bela Port v0.02\n");

    printf("Setting MidiQueue and resizing to %lu items\n", midiQueueSize);
    MidiOutput::setMidiQueue(gMidiQueue);
    gMidiQueue->resize(midiQueueSize);
    
    printf("Setting MidiOutput to '%s'\n", midiOutputName.c_str());
    MidiOutput::midiOutput_ = MidiOutput("hw:0:0:0");

    printf("Setting Midi Input Mode to Standalone\n");
    controller.disablePrimaryMIDIInputPort();
    controller.disableAllMIDIOutputPorts();
//    controller.midiTouchkeysStandaloneModeEnable();

    printf("Setting lowest midi note to 0\n");
    controller.touchkeyDeviceSetLowestMidiNote(0);

    printf("Setting verbose level to %d\n", gVerboseLevel);
    controller.touchkeyDeviceSetVerbosity(gVerboseLevel);

    printf("Calibrating for %d seconds\n", kCalibrationTimeSeconds);
    controller.startCalibration(kCalibrationTimeSeconds);
    controller.finishCalibration();

    printf("Updating queiscent values\n");
    controller.updateQuiescent();
//    printf("Setting Midi Output Mode to polyphonic and associating output controller\n");
//    controller.midiSegmentsSetMode(0);
//    controller.midiSegmentsSetMidiOutputController();

    printf("Setting OSC host to %s:%s and enabling output", oscHost.c_str(), oscPort.c_str());
    controller.oscTransmitClearAddresses();
    controller.oscTransmitAddAddress(oscHost.c_str(), oscPort.c_str());
    controller.oscTransmitSetEnabled(true);


	while((ch = getopt_long(argc, argv, "hli:o:t:VP:", long_options, &option_index)) != -1)
	{
        if(ch == 'l') { // List devices
            list_devices(controller);
            shouldStart = false;
            break;
        }
        else if(ch == 't') { // TouchKeys device
            touchkeysDevicePath = optarg;
            autostartTouchkeys = true;
        }
        else if(ch == 'i') { // MIDI input device
            midiInputNum = atoi(optarg);
            autoopenMidiIn = true;
        }
        else if(ch == 'o') { // MIDI output device
            midiOutputNum = atoi(optarg);
            autoopenMidiOut = true;
        }
        else if(ch == 'V') { // Virtual MIDI output
            useVirtualMidiOutput = true;
            autoopenMidiOut = true;
        }
        else if(ch == 'P') { // OSC port
            oscInputPort = atoi(optarg);
        }
        else {
            usage(basename(argv[0]));
            shouldStart = false;
            break;
		}
	}
    
    
    if(shouldStart) {
        // Main initialization: open TouchKeys and MIDI devices
        controller.initialise();
        
        // Always enable OSC input without GUI, since it is how we control
        // the system
        controller.oscReceiveSetPort(oscInputPort);
        controller.oscReceiveSetEnabled(true);
        
        try {
            // Open MIDI devices
            if(autoopenMidiIn) {
                cout << "Opening MIDI input device " << midiInputNum << endl;
                controller.enableMIDIInputPort(midiInputNum, true);
            }

            // TODO: enable multiple keyboard segments
            if(autoopenMidiOut) {
                if(useVirtualMidiOutput) {
#ifndef JUCE_WINDOWS
                    cout << "Opening virtual MIDI output\n";
                    controller.enableMIDIOutputVirtualPort(0, "TouchKeys");
#endif
                }
                else {
                    cout << "Opening MIDI output device " << midiOutputNum << endl;
                    controller.enableMIDIOutputPort(0, midiOutputNum);
                }
            }
            
            // Start the TouchKeys
            if(autostartTouchkeys) {
                cout << "Starting the TouchKeys on " << touchkeysDevicePath << " ... ";
                if(!controller.touchkeyDeviceStartupSequence(touchkeysDevicePath.c_str())) {
                    cout << "failed: " << controller.touchkeyDeviceErrorMessage() << endl;
                    throw new exception;
                }
                else
                    cout << "succeeded!\n";
            }
            
            // Set up interrupt catching so we can stop with Ctrl-C
            struct sigaction sigIntHandler;
            
            sigIntHandler.sa_handler = sigint_handler;
            sigemptyset(&sigIntHandler.sa_mask);
            sigIntHandler.sa_flags = 0;
            sigaction(SIGINT, &sigIntHandler, NULL);

            // Wait until interrupt signal is received
            while(!programShouldStop_) {
                usleep(50);
            }
        }
        catch(...) {
            
        }
        
        // Stop TouchKeys if still running
        if(controller.touchkeyDeviceIsRunning())
            controller.stopTouchkeyDevice();
    }
    
    return 0;
}
