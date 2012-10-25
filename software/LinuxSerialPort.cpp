/*
 * va2epr-tnc software - terminal node controller software for va2epr-tnc
 * Copyright (C) 2012 Thomas Cort <va2epr@rac.ca>
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

#include "LinuxSerialPort.h"

LinuxSerialPort::LinuxSerialPort(void) {

	setPort(std::string("/dev/ttyUSB0"));
	setFd(-1);
	setOpen(false);
}

LinuxSerialPort::~LinuxSerialPort(void) {

	if (isOpen()) {
		close();
	}
}
void LinuxSerialPort::open(void) {

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

	if (cfsetispeed(&config, B9600) < 0 || cfsetospeed(&config, B9600) < 0) {

		errno_save = errno;
		close();
		throw errno_save;
	}

	if (tcsetattr(getFd(), TCSAFLUSH, &config) < 0) {

		errno_save = errno;
		close();
		throw errno_save;
	}

#ifdef __TEST_CODE__
	while (1) {
		unsigned char c;
		read(getFd(), &c, 1);
		write(1, &c, 1);
	}
#endif
}

void LinuxSerialPort::close(void) {

	if (isOpen()) {
		::close(getFd());
		setOpen(false);
		setFd(-1);
	}
}

