/*
 * TestApp.cpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Juniper
 */

#include <signal.h>
#include <stdint.h>
#include <pthread.h>
#include <memory>
#include <assert.h>
#include "SerialInterface.hpp"
#include "StatusFrame.hpp"
#include "AnalogFrame.hpp"
//#include "Keys.h"

int gXenomaiInited = 0; // required by libbelaextra
unsigned int gAuxiliaryTaskStackSize  = 1 << 17; // required by libbelaextra


const bool VERBOSE = true;
const int SERIAL_BUFFER_SIZE = 1024;
const int ITERATIONS_STATUS_LOOP = 10;
const int WAIT_STATUS_LOOP_US = 1E6;
const int WAIT_MAIN_LOOP_US = 50000;

int gShouldStop;
int gShouldSendScans;
int frameDataLength = 25;

//BoardsTopology bt;
//Keys* keys;
SerialInterface serialInterface = SerialInterface();

void interrupt_handler(int)
{
	gShouldStop = 1;
}

int requestStatusFrame()
{
	char frameBuffer[TOUCHKEY_MAX_FRAME_LENGTH];
	int len = 0;

	frameBuffer[len++] = ESCAPE_CHARACTER;
	frameBuffer[len++] = kControlCharacterFrameBegin;
	frameBuffer[len++] = kFrameTypeStatus;
	frameBuffer[len++] = ESCAPE_CHARACTER;
	frameBuffer[len++] = kControlCharacterFrameEnd;

	int ret = serialInterface.serialWrite(frameBuffer, len);

	if (ret < 0) {
		printf("Failed writing, breaking\n");

		return -1;
	}

	return len;
}

int requestStartScanning()
{
	char frameBuffer[TOUCHKEY_MAX_FRAME_LENGTH];
	int len = 0;

	frameBuffer[len++] = ESCAPE_CHARACTER;
	frameBuffer[len++] = kControlCharacterFrameBegin;
	frameBuffer[len++] = kFrameTypeStartScanning;
	frameBuffer[len++] = ESCAPE_CHARACTER;
	frameBuffer[len++] = kControlCharacterFrameEnd;

	int ret = serialInterface.serialWrite(frameBuffer, len);

	if (ret < 0) {
		printf("Failed writing, breaking\n");

		return -1;
	}

	return len;
}

int requestStopScanning()
{
	char frameBuffer[TOUCHKEY_MAX_FRAME_LENGTH];
	int len = 0;

	frameBuffer[len++] = ESCAPE_CHARACTER;
	frameBuffer[len++] = kControlCharacterFrameBegin;
	frameBuffer[len++] = kFrameTypeStopScanning;
	frameBuffer[len++] = ESCAPE_CHARACTER;
	frameBuffer[len++] = kControlCharacterFrameEnd;

	int ret = serialInterface.serialWrite(frameBuffer, len);

	if (ret < 0) {
		printf("Failed writing, breaking\n");

		return -1;
	}

	return len;
}

int readStatusFrame()
{
	char serialBuffer[SERIAL_BUFFER_SIZE];
	int ret = serialInterface.serialRead(serialBuffer, SERIAL_BUFFER_SIZE, -1);

	if (ret > 0) {
		if (ESCAPE_CHARACTER == serialBuffer[0]) {
			if (kControlCharacterFrameBegin == serialBuffer[1]) {
				if (kFrameTypeStatus == serialBuffer[2]) {
					StatusFrame statusFrame = StatusFrame(serialBuffer);
					if (VERBOSE) {
						statusFrame.printFrame();
					}
				} else if (kFrameTypeAnalog == serialBuffer[2]) {
					printf(
							"Got analog frame instead of status frame, breaking\n");
					return -1;
				} else {
					printf("Did not recognize frame type, breaking\n");
					return -1;
				}
			}
		}
	} else {
		printf("Received raw byte: %d\n", serialBuffer[0]);
	}

	return ret;
}

int readAnalogFrame()
{
	char serialBuffer[SERIAL_BUFFER_SIZE];
	int ret = serialInterface.serialRead(serialBuffer, SERIAL_BUFFER_SIZE, -1);

	if (ret > 0) {
		if (ESCAPE_CHARACTER == serialBuffer[0]) {
			if (kControlCharacterFrameBegin == serialBuffer[1]) {
				if (kFrameTypeAnalog == serialBuffer[2]) {
					AnalogFrame analogFrame = AnalogFrame(serialBuffer);
					if (VERBOSE) {
						analogFrame.printFrame();
					}
				} else if (kFrameTypeStatus == serialBuffer[2]) {
					printf(
							"Got status frame instead of analog frame, breaking\n");
					return -1;
				} else {
					printf("Did not recognize frame type, breaking\n");
					return -1;
				}
			}
		}
	} else {
		printf("Received raw byte: %d\n", serialBuffer[0]);
	}

	return ret;
}

int main()
{
	/* FIXME: get correct device location */
	serialInterface.serialSetup("/dev/ttyGS0");
	signal(SIGINT, interrupt_handler);
	signal(SIGTERM, interrupt_handler);

	int ret;

	for (unsigned int n = 0; n < ITERATIONS_STATUS_LOOP; ++n) {
		ret = requestStatusFrame();

		if (ret > 0) {
			printf("Status frame request sent successfully\n");
		} else {
			/* TODO: message */
		}

		ret = readStatusFrame();

		if (ret > 0) {
			printf("Status frame read successfully\n");
		} else {
			/* TODO: message */
		}

		usleep(WAIT_STATUS_LOOP_US);
	}

	ret = requestStartScanning();

	if (ret > 0) {
		printf("Scanning start request sent successfully\n");
	} else {
		printf("Scanning start request failed. Exiting\n");
		return ret;
	}

	while (!gShouldStop) {
		ret = readAnalogFrame();
		usleep(WAIT_MAIN_LOOP_US);
	}

	ret = requestStopScanning();
	serialInterface.serialCleanup();

}
