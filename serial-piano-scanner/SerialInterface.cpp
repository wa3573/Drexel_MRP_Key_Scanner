/*
 * SerialInterface.cpp
 *
 *  Created on: Dec 28, 2018
 *      Author: William Anderson
 */

#include "SerialInterface.h"

#include <termios.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>

static int _handle;

SerialInterface::SerialInterface()
{

}

SerialInterface::SerialInterface(size_t BUFFER_SIZE)
{
	this->BUFFER_SIZE = BUFFER_SIZE;
}

SerialInterface::~SerialInterface()
{

}

/*
 com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo - UsbConfigurationDescriptor -
 com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo     .bLength = 9
 com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo     .bDescriptorType = 0x02
 com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo     .wTotalLength = 32
 com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo     .bNumInterfaces = 1
 com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo     .bConfigurationValue = 1
 com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo     .iConfiguration = 0
 com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo     .bmAttributes = 0x80
 com_silabs_driver_CP210xVCPDriver(<ptr>)::start Interface 0 - Sucessfully loaded the driver
 */
int SerialInterface::set_interface_attribs(int fd, int speed)
{
	struct termios tty;

	if (tcgetattr(fd, &tty) < 0) {
		printf("Error from tcgetattr: %s\n", strerror(errno));
		return -1;
	}

	cfsetospeed(&tty, (speed_t) speed);
	cfsetispeed(&tty, (speed_t) speed);

	tty.c_cflag |= (CLOCAL | CREAD); /* ignore modem controls */
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8; /* 8-bit characters */
	tty.c_cflag &= ~PARENB; /* no parity bit */
	tty.c_cflag &= ~CSTOPB; /* only need 1 stop bit */
	tty.c_cflag &= ~CRTSCTS; /* no hardware flowcontrol */

	/* setup for non-canonical mode */
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL
			| IXON);
	tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tty.c_oflag &= ~OPOST;

	/* fetch bytes as they become available */
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 1;

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		printf("Error from tcsetattr: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

void SerialInterface::set_mincount(int fd, int mcount)
{
	struct termios tty;

	if (tcgetattr(fd, &tty) < 0) {
		printf("Error tcgetattr: %s\n", strerror(errno));
		return;
	}

	tty.c_cc[VMIN] = mcount ? 1 : 0;
	tty.c_cc[VTIME] = 5; /* half second timer */

	if (tcsetattr(fd, TCSANOW, &tty) < 0) {
		printf("Error tcsetattr: %s\n", strerror(errno));
	}
}

int SerialInterface::initSerial(const char *portname, int speed)
{
	printf("Attempting to connect to %s\n", portname);
	_handle = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
	if (_handle < 0) {
		printf("Error opening %s: %s\n", portname, strerror(errno));
		return -1;
	} else {
		printf("Successfully opened %s with file descriptor %d\n", portname,
				_handle);
	}

	set_interface_attribs(_handle, speed);
	set_mincount(_handle, 0); /* set to pure timed read */
	return 0;
}

int SerialInterface::serialRead(CircularBuffer<char>& buf, int timeoutMs)
{
	char intermediate_buf[1000];
	struct pollfd pfd[1];
	pfd[0].fd = _handle;
	pfd[0].events = POLLIN;
	//printf("before poll\n");
	int result = poll(pfd, 1, timeoutMs);
	if (result < 0) {
		fprintf(stderr, "Error polling for serial: %d %s\n", errno,
				strerror(errno));
		return errno;
	} else if (result == 0) {
		printf("Timeout\n");
		return 0;
	} else if (pfd[0].revents & POLLIN) {
		int rdlen = read(_handle, intermediate_buf, this->BUFFER_SIZE);
		if (rdlen > 0) {
			printf("Serial read %d bytes \n", rdlen);
			int count = 0;

			/* Insert contents of intermediate buffer into circular buffer as space becomes available */
			while (count < rdlen) {
				if (!buf.is_full()) {
					buf.put(intermediate_buf[count]);
				}
			}

		} else if (rdlen < 0) {
			fprintf(stderr, "Error from read: %d: %s\n", rdlen,
					strerror(errno));
		}
		return rdlen;
	} else {
		fprintf(stderr, "unknown error while reading serial\n");
		return -1;
	}
}

int SerialInterface::serialRead(char* buf, size_t len, int timeoutMs)
{
	struct pollfd pfd[1];
	pfd[0].fd = _handle;
	pfd[0].events = POLLIN;
	//printf("before poll\n");
	int result = poll(pfd, 1, timeoutMs);
	if (result < 0) {
		fprintf(stderr, "Error polling for serial: %d %s\n", errno,
				strerror(errno));
		return errno;
	} else if (result == 0) {
		printf("Timeout\n");
		// timeout
		return 0;
	} else if (pfd[0].revents & POLLIN) {
		//printf("before read\n");
		int rdlen = read(_handle, buf, len);
		//printf("after read\n");
		if (rdlen > 0) {
			if (1 || rdlen != 5) {
				printf("Serial read %d bytes: ", rdlen);
//				for (int n = 0; n < rdlen; ++n) {
//					printf("%03d ", buf[n]);
//				}
				printf("\n");
			}
		} else if (rdlen < 0) {
			fprintf(stderr, "Error from read: %d: %s\n", rdlen,
					strerror(errno));
		}
		return rdlen;
	} else {
		fprintf(stderr, "unknown error while reading serial\n");
		return -1;
	}
}

int SerialInterface::serialWrite(const char* buf, size_t len)
{
	if (0) {
		printf("Writing %lu bytes: ", len);
		for (unsigned int n = 0; n < len; ++n)
			printf("%d ", buf[n]);
		printf("\n");
	}
	int ret = write(_handle, buf, len);
	if (ret < 0) {
		fprintf(stderr, "write failed: %d %s\n", errno, strerror(errno));
	}
	return ret;
}

void SerialInterface::serialCleanup()
{
	close(_handle);
}

bool SerialInterface::serialSetup(const char* device)
{
	return !initSerial(device, B115200);
}

