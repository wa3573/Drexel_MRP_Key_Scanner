/*
 * TestApp.cpp
 *
 *  Created on: Dec 28, 2018
 *      Author: William Anderson
 */

#include <signal.h>
#include <stdint.h>
#include <pthread.h>
#include <memory>
#include <assert.h>
#include <pthread.h>
#include <cstring>

#include "AnalogFrame.h"
#include "KeyPositionTracker.h"
#include "SerialInterface.h"
#include "StatusFrame.h"

struct thread_args_t {
	juniper::circular_buffer<char>* buf;
};

struct analog_callback_args_t {
	AnalogFrame* analogFrame;
};

const bool VERBOSE = true;
const int SERIAL_BUFFER_SIZE = 1024;
const int ITERATIONS_STATUS_LOOP = 10;
const int WAIT_STATUS_LOOP_US = 1E6;
const int WAIT_MAIN_LOOP_US = 50000;
const int TOUCHKEY_COMMAND_LENGTH = 5;

int gXenomaiInited = 0; // required by libbelaextra
unsigned int gAuxiliaryTaskStackSize = 1 << 17; // required by libbelaextra
int gShouldStop;
int gShouldSendScans;
bool gShouldConsumeSerial = true;
pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gIsDataAvailable;

//BoardsTopology bt;
//Keys* keys;
SerialInterface serialInterface = SerialInterface();
KeyBuffers keyBuffers;
std::vector<KeyBuffer> keyBuffer;
std::vector<KeyPositionTracker> keyPositionTracker;

extern "C" int rt_printf(const char *format, ...);
void* serialProducerThread(void* args);
void* serialConsumerThread(void* args);
void* controlThread(void* args);

int main()
{
	juniper::circular_buffer<char> serial_buffer(SERIAL_BUFFER_SIZE);
	pthread_t serial_producer;
	pthread_t serial_consumer;
	pthread_t control_thread;

	thread_args_t* thread_args = new thread_args_t();
	int ret;
	(*thread_args).buf = &serial_buffer;

	ret = pthread_create(&control_thread, NULL, controlThread,
			(void*) thread_args);
	if (ret) {
		fprintf(stderr, "Error - pthread_create() return code: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	ret = pthread_create(&serial_producer, NULL, serialProducerThread,
			(void*) thread_args);
	if (ret) {
		fprintf(stderr, "Error - pthread_create() return code: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	ret = pthread_create(&serial_consumer, NULL, serialConsumerThread,
			(void*) thread_args);
	if (ret) {
		fprintf(stderr, "Error - pthread_create() return code: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	pthread_join(serial_producer, NULL);
	pthread_join(serial_consumer, NULL);
	pthread_join(control_thread, NULL);

	exit(EXIT_SUCCESS);
}

void interrupt_handler(int)
{
	gShouldStop = 1;
}

int requestStatusFrame()
{
	char frame_buffer[TOUCHKEY_COMMAND_LENGTH];
//	int len = 0;
//	frame_buffer[len++] = ESCAPE_CHARACTER;
//	frame_buffer[len++] = kControlCharacterFrameBegin;
//	frame_buffer[len++] = kFrameTypeStatus;
//	frame_buffer[len++] = ESCAPE_CHARACTER;
//	frame_buffer[len++] = kControlCharacterFrameEnd;
	memcpy(&frame_buffer, &kCommandStatus, TOUCHKEY_COMMAND_LENGTH);
	int ret = serialInterface.serialWrite(frame_buffer,
			TOUCHKEY_COMMAND_LENGTH);

	if (ret < 0) {
		printf("Failed writing, breaking\n");

		return -1;
	}

	return TOUCHKEY_COMMAND_LENGTH;
}

int requestStartScanning()
{
	char frame_buffer[TOUCHKEY_COMMAND_LENGTH];
//	int len = 0;
//	frame_buffer[len++] = ESCAPE_CHARACTER;
//	frame_buffer[len++] = kControlCharacterFrameBegin;
//	frame_buffer[len++] = kFrameTypeStartScanning;
//	frame_buffer[len++] = ESCAPE_CHARACTER;
//	frameBuffer[len++] = kControlCharacterFrameEnd;
	memcpy(&frame_buffer, &kCommandStartScanning, TOUCHKEY_COMMAND_LENGTH);
	int ret = serialInterface.serialWrite(frame_buffer,
			TOUCHKEY_COMMAND_LENGTH);

	if (ret < 0) {
		printf("Failed writing, breaking\n");

		return -1;
	}

	return TOUCHKEY_COMMAND_LENGTH;
}

int requestStopScanning()
{
	char frame_buffer[TOUCHKEY_COMMAND_LENGTH];
//	int len = 0;
//	frame_buffer[len++] = ESCAPE_CHARACTER;
//	frame_buffer[len++] = kControlCharacterFrameBegin;
//	frame_buffer[len++] = kFrameTypeStopScanning;
//	frame_buffer[len++] = ESCAPE_CHARACTER;
//	frame_buffer[len++] = kControlCharacterFrameEnd;
	memcpy(&frame_buffer, &kCommandStopScanning, TOUCHKEY_COMMAND_LENGTH);
	int ret = serialInterface.serialWrite(frame_buffer,
			TOUCHKEY_COMMAND_LENGTH);

	if (ret < 0) {
		printf("Failed writing, breaking\n");

		return -1;
	}

	return TOUCHKEY_COMMAND_LENGTH;
}

void insertBufferChunk(juniper::circular_buffer<char>* buffer, char* intermediate_buffer,
		int len)
{
	int count = 0;

	while (count > len) {
		if (!buffer->full()) {
			buffer->push_back(intermediate_buffer[count]);

			/* TODO: is this call to empty() necessary? */
			if (!buffer->empty()) {
				pthread_cond_signal(&gIsDataAvailable);
			}
		}

		count++;
	}
}

void* serialProducerThread(void* args)
{
	char intermediate_bufffer[SERIAL_BUFFER_SIZE];
	thread_args_t* thread_args = (thread_args_t*) args;
	int ret;

	/* TODO: change path to 'by-id' directory so port doesn't matter */
	serialInterface.serialSetup(
			"/dev/serial/by-path/platform-musb-hdrc.1.auto-usb-0:1:1.0");

	ret = requestStartScanning();

	while (gShouldConsumeSerial) {
		ret = serialInterface.serialRead(intermediate_bufffer,
				SERIAL_BUFFER_SIZE, -1);

		if (ret > 0) {
			insertBufferChunk(thread_args->buf, intermediate_bufffer, ret);
		}
	}

	ret = requestStopScanning();
	serialInterface.serialCleanup();

	return NULL;
}

char getChar(juniper::circular_buffer<char>* buffer)
{
	pthread_mutex_lock(&gMutex);
	while (buffer->empty()) {
		pthread_cond_wait(&gIsDataAvailable, &gMutex);
	}
	pthread_mutex_unlock(&gMutex);

	return buffer->pop_front();
}

//void analogCallback(void* args)
//{
//	analog_callback_args_t* callback_args = (analog_callback_args_t*) args;
//
//	/* first index is (octave * 12) */
//	unsigned int index = callback_args->analogFrame->octave * 12;
//
//	/* update each key in the buffer */
//	for (unsigned int n = index; n < index + 12; ++n)
//	{
//
//	}
//}
//
//void addToBufferFromFrame(AnalogFrame frame) {
//	/* first index is (octave * 12) */
//	unsigned int index = frame.octave * 12;
//
//	/* TODO: update each key in the buffer */
//	for (unsigned int n = index; n < index + 12; ++n)
//	{
////		keyBuffer[n]
//	}
//}

void* serialConsumerThread(void* args)
{
	thread_args_t* thread_args = (thread_args_t*) args;
	char frame_buffer[TOUCHKEY_MAX_FRAME_LENGTH];
	char c;
	int count;
	bool frame_captured = false;
	StatusFrame current_status_frame;
	AnalogFrame current_analog_frame;

	while (gShouldConsumeSerial) {
		c = getChar(thread_args->buf);
		printf("serialConsumerThread: consumed '%X'\n", c);

		if (c == ESCAPE_CHARACTER) {
			printf("serialConsumerThread: got escape character \n");
			c = getChar(thread_args->buf);

			if (c == kControlCharacterFrameBegin) {
				printf("serialConsumerThread: got frame begin character \n");
				/* Read the rest of the frame into the frame_buffer until either frame end or the maximum frame length */
				count = 0;
				while (true) {
					if (c == kControlCharacterFrameEnd) {
						printf(
								"serialConsumerThread: got frame end character \n");
						frame_buffer[count] = c;
						frame_captured = true;
						break;
					}

					if (count < TOUCHKEY_MAX_FRAME_LENGTH) {
						printf(
								"serialConsumerThread: frame exceeded max length \n");
						frame_captured = false;
						break;
					}

					c = getChar(thread_args->buf);
					frame_buffer[count] = c;

					count++;
				}

				if (frame_captured) {
					printf("serialConsumerThread: frame capture successful \n");

					if (frame_buffer[0] == kFrameTypeAnalog) {
						printf(
								"serialConsumerThread: detected analog frame \n");
						current_analog_frame = AnalogFrame(frame_buffer);

					} else if (frame_buffer[0] == kFrameTypeStatus) {
						printf(
								"serialConsumerThread: detected status frame \n");
						current_status_frame = StatusFrame(frame_buffer);
					} else {

					}

				} else {
					printf("serialConsumerThread: frame capture failed \n");
				}
			}
		}
	}

	return NULL;
}

void* controlThread(void* args)
{
	signal(SIGINT, interrupt_handler);
	signal(SIGTERM, interrupt_handler);

	/* Request status frames at first */
	for (unsigned int n = 0; n < ITERATIONS_STATUS_LOOP; ++n) {
		if (gShouldStop) {
			break;
		}

		requestStatusFrame();

		usleep(WAIT_STATUS_LOOP_US);
	}

	if (gShouldStop) {
		gShouldConsumeSerial = false;
	}

	return NULL;
}

