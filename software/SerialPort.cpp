/*
 * va2epr-tnc software - terminal node controller software for va2epr-tnc
 * Copyright (C) 2012, 2013 Thomas Cort <va2epr@rac.ca>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "SerialPort.h"

/**
 * Default Constructor
 */
SerialPort::SerialPort(void) {

	setPort(std::string("/dev/ttyUSB0"));
	setFd(-1);
	setOpen(false);
}

/**
 * Default Destructor
 */
SerialPort::~SerialPort(void) {

	// Close the connection in the event that the user forgot to.
	if (isOpen()) {
		close();
	}
}

/**
 * Get the serial port used
 *
 * @return the serial port name (example: "/dev/ttyUSB0")
 */
std::string SerialPort::getPort(void) {

	return _port;
}

/**
 * Set the serial port to use
 *
 * @param port the name of the serial port (example "/dev/ttyUSB0")
 */
void SerialPort::setPort(std::string port) {

	_port = port;
}

/**
 * Check if the serial connection is open
 *
 * @return the state of the serial port (true==open)
 */
bool SerialPort::isOpen(void) {

	return _open;
}

/**
 * Set the state of the serial port (open or closed)
 *
 * @param open set to true for open
 */
void SerialPort::setOpen(bool open) {

	_open = open;
}

/**
 * Open the serial port (i.e. connect to the device
 * This throws an exception when there is an error passing the value of errno.
 */
void SerialPort::open(void) {

	int fd, errno_save;
	const char *device;
	struct termios config;

	device = getPort().c_str();

	fd = ::open(device, O_RDWR | O_NOCTTY); // | O_NDELAY);

	setFd(fd);
	setOpen(getFd() != -1);

	if (!isOpen()) {

		throw errno;
	}

	if (!isatty(getFd())) {

		errno_save = errno;
		close();
		throw errno_save;
	}

	if (!tcgetattr(getFd(), &config) < 0) {

		errno_save = errno;
		close();
		throw errno_save;
	}

	config.c_iflag &= ~(
				IGNBRK |
				BRKINT |
				ICRNL  |
				INLCR  |
				PARMRK |
				INPCK  |
				ISTRIP |
				IXON
			);

	config.c_oflag &= ~(
				OCRNL  |
				ONLCR  |
				ONLRET |
				ONOCR  |
				OFILL  |
				OLCUC  |
				OPOST
			);

	config.c_lflag &= ~(
				ECHO   |
				ECHONL |
				ICANON |
				IEXTEN |
				ISIG
			);

	config.c_cflag &= ~(
				CSIZE  |
				PARENB
			);

	config.c_cflag |= CS8;

	config.c_cc[VMIN]  = 1;
	config.c_cc[VTIME] = 0;

	if (cfsetispeed(&config, B230400) < 0 || cfsetospeed(&config, B230400) < 0) {

		errno_save = errno;
		close();
		throw errno_save;
	}

	if (tcsetattr(getFd(), TCSAFLUSH, &config) < 0) {

		errno_save = errno;
		close();
		throw errno_save;
	}

	// if we made it here, the port is open an ready to recv/send

#ifdef __TEST_CODE__
	while (1) {
		unsigned char c;
		read(getFd(), &c, 1);
		write(1, &c, 1);
	}
#endif
}

/**
 * Close down the connection to the serial port
 */
void SerialPort::close(void) {

	if (isOpen()) {
		::close(getFd());
		setOpen(false);
		setFd(-1);
	}
}

