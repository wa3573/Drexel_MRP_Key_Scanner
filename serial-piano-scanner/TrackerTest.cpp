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
#include <signal.h>
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
//const int WAIT_STATUS_LOOP_US = 1E6;
//const int WAIT_MAIN_LOOP_US = 50000;
const int TOUCHKEY_COMMAND_LENGTH = 5;
//const int WAIT_STAGE_US = 5E6;
const int WAIT_SHUTDOWN_US = 5E6;
const int WAIT_THREAD_US = 10E6;

int gXenomaiInited = 0; // required by libbelaextra
unsigned int gAuxiliaryTaskStackSize = 1 << 17; // required by libbelaextra
//int gShouldStop;
//int gShouldSendScans;
//bool gShouldConsumeSerial = true;
//pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;

//BoardsTopology bt;
//Keys* keys;
SerialInterface serialInterface = SerialInterface();
std::vector<KeyPositionTracker> keyPositionTracker;

struct thread_shared_t {
	bool shouldConsumeSerial = true;
	bool recievedShutdownRequest = false;
	int bytesAvailable = 0;
	int threadsActive = 0;
	pthread_mutex_t mutexGeneral = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t mutexSerialInterface = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t isDataAvailable;
	pthread_cond_t threadExitCV;
} gThreadShared;

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

	sigset_t signals_to_block;
	sigemptyset(&signals_to_block);
	sigaddset(&signals_to_block, SIGQUIT);
	sigaddset(&signals_to_block, SIGINT);
	sigaddset(&signals_to_block, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &signals_to_block, NULL);

	printf("Creating controlThread\n");
	ret = pthread_create(&control_thread, NULL, controlThread,
			(void*) thread_args);
	if (ret) {
		fprintf(stderr, "Error - pthread_create() return code: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	printf("Creating serialConsumerlThread\n");
	ret = pthread_create(&serial_consumer, NULL, serialConsumerThread,
			(void*) thread_args);
	if (ret) {
		fprintf(stderr, "Error - pthread_create() return code: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	printf("Creating serialProducerThread\n");
	ret = pthread_create(&serial_producer, NULL, serialProducerThread,
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
	pthread_mutex_lock(&gThreadShared.mutexSerialInterface);
	int ret = serialInterface.serialWrite(frame_buffer,
			TOUCHKEY_COMMAND_LENGTH);
	pthread_mutex_unlock(&gThreadShared.mutexSerialInterface);

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

	pthread_mutex_lock(&gThreadShared.mutexSerialInterface);
	int ret = serialInterface.serialWrite(frame_buffer,
			TOUCHKEY_COMMAND_LENGTH);
	pthread_mutex_unlock(&gThreadShared.mutexSerialInterface);

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

	pthread_mutex_lock(&gThreadShared.mutexSerialInterface);
	int ret = serialInterface.serialWrite(frame_buffer,
			TOUCHKEY_COMMAND_LENGTH);
	pthread_mutex_unlock(&gThreadShared.mutexSerialInterface);

	if (ret < 0) {
		printf("Failed writing, breaking\n");

		return -1;
	}

	return TOUCHKEY_COMMAND_LENGTH;
}

void insertBufferChunk(juniper::circular_buffer<char>* buffer,
		char* intermediate_buffer, int len)
{
	int count = 0;

	while (count > len) {
		buffer->push_back(intermediate_buffer[count]);
		count++;
	}
}

void* serialProducerThread(void* args)
{
	pthread_mutex_lock(&gThreadShared.mutexGeneral);
	gThreadShared.threadsActive++;
	pthread_mutex_unlock(&gThreadShared.mutexGeneral);
	char intermediate_bufffer[SERIAL_BUFFER_SIZE];
	thread_args_t* thread_args = (thread_args_t*) args;
	int ret;

	serialInterface.serialSetup(
			"/dev/serial/by-id/usb-APM_TouchKeys_8D73428C5751-if00");

	printf("serialProducerThread: requestStartScanning()\n");
	ret = requestStartScanning();

	/* Main loop */
	while (true) {
		/* Break if signal received from control thread */
		pthread_mutex_lock(&gThreadShared.mutexGeneral);
		if (gThreadShared.recievedShutdownRequest) {
			printf("serialProducerThread: got shutdown request, breaking\n");
			break;
		}
		pthread_mutex_unlock(&gThreadShared.mutexGeneral);

		/* Mutex not required since only the producer thread calls serialRead */
		ret = serialInterface.serialRead(intermediate_bufffer,
				SERIAL_BUFFER_SIZE, -1);

		if (ret > 0) {
			printf(
					"serialProducerThread: Inserting buffer chunk of length %d\n",
					ret);
			insertBufferChunk(thread_args->buf, intermediate_bufffer, ret);
		}

		if (!thread_args->buf->empty()) {
			printf("serialProducerThread: signaling consumer\n");
			pthread_cond_signal(&gThreadShared.isDataAvailable);
		}

		usleep(WAIT_THREAD_US);
	}

	printf("serialProducerThread: requestStopScanning()\n");
	ret = requestStopScanning();
	serialInterface.serialCleanup();

	pthread_mutex_lock(&gThreadShared.mutexGeneral);
	gThreadShared.threadsActive--;
	pthread_mutex_unlock(&gThreadShared.mutexGeneral);
	return NULL;
}

char getChar(juniper::circular_buffer<char>* buffer)
{
//	printf("getChar(): waiting for data in the buffer\n");
	pthread_mutex_lock(&gThreadShared.mutexGeneral);
	while (buffer->empty()) {
		pthread_cond_wait(&gThreadShared.isDataAvailable,
				&gThreadShared.mutexGeneral);
	}
	pthread_mutex_unlock(&gThreadShared.mutexGeneral);
//	printf("getChar(): found data in the buffer\n");

	return buffer->pop_front();
}

void* serialConsumerThread(void* args)
{
	pthread_mutex_lock(&gThreadShared.mutexGeneral);
	gThreadShared.threadsActive++;
	pthread_mutex_unlock(&gThreadShared.mutexGeneral);

	thread_args_t* thread_args = (thread_args_t*) args;
	unsigned char frame_buffer[TOUCHKEY_MAX_FRAME_LENGTH];
	unsigned char c;
	int count;
	bool frame_captured = false;
	StatusFrame current_status_frame;
	AnalogFrame current_analog_frame;

	printf("serialConsumerThread: started\n");

	/* Main loop */
	while (true) {
//		printf("serialConsumerThread: consuming serial data\n");
		c = getChar(thread_args->buf);
//		printf("serialConsumerThread: consumed '%X'\n", c);

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

	pthread_mutex_lock(&gThreadShared.mutexGeneral);
	gThreadShared.threadsActive--;
	pthread_mutex_unlock(&gThreadShared.mutexGeneral);
	return NULL;
}



void* controlThread(void* args)
{
	sigset_t signals_to_catch;
	sigemptyset(&signals_to_catch);
	sigaddset(&signals_to_catch, SIGQUIT);
	sigaddset(&signals_to_catch, SIGINT);
	sigaddset(&signals_to_catch, SIGTERM);

	int signals_caught;

	/* Request status frames first */
	printf("controlThread: requesting %d status frames \n",
			ITERATIONS_STATUS_LOOP);

	for (unsigned int n = 0; n < ITERATIONS_STATUS_LOOP; ++n) {
		requestStatusFrame();
	}

	/* Wait for signals */
	printf("conrolThread: waiting for signals...\n");
	sigwait(&signals_to_catch, &signals_caught);
	printf("controlThread: got signal, exiting...\n");

	/* Set shutdown flag */
	pthread_mutex_lock(&gThreadShared.mutexGeneral);
	gThreadShared.recievedShutdownRequest = true;
	pthread_mutex_unlock(&gThreadShared.mutexGeneral);

	/* Wait for other threads to complete */
	while (true) {
		pthread_mutex_lock(&gThreadShared.mutexGeneral);
		if (gThreadShared.threadsActive > 0) {
			pthread_mutex_unlock(&gThreadShared.mutexGeneral);
			break;
		}
		pthread_mutex_unlock(&gThreadShared.mutexGeneral);
		printf(
				"controlThread: Waiting %d microseconds for threads to complete\n",
				WAIT_SHUTDOWN_US);
		usleep(WAIT_SHUTDOWN_US);
	}

	exit(EXIT_SUCCESS);
	return NULL;
}

