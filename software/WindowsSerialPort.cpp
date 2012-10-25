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

#include <iostream>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <windows.h>

#include "WindowsSerialPort.h"

WindowsSerialPort::WindowsSerialPort(void) {

	setPort(std::string("COM5"));
	setHandle(INVALID_HANDLE_VALUE);
	setOpen(false);
}

WindowsSerialPort::~WindowsSerialPort(void) {

	if (isOpen()) {
		close();
	}
}
void WindowsSerialPort::open(void) {

	HANDLE hSerial;
	DCB dcbSerialParams = {0};
	COMMTIMEOUTS timeouts = {0};

	hSerial = CreateFile(
				getPort().c_str(),
				GENERIC_READ | GENERIC_WRITE,
				0,
				0,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				0
			);

	setHandle(hSerial);
	setOpen(hSerial != INVALID_HANDLE_VALUE);

	if (!isOpen()) {

		throw 1;
	}

	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	if (!GetCommState(getHandle(), &dcbSerialParams)) {

		throw 1;
	}

	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity   = NOPARITY;

	if (!SetCommState(getHandle(), &dcbSerialParams)) {

		throw 1;
	}

	timeouts.ReadIntervalTimeout = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;

	if (!SetCommTimeouts(hSerial, &timeouts)) {

		throw 1;
	}

#ifdef __TEST_CODE__
	while (1) {
		DWORD dwBytesRead = 0;
		char szBuff[2] = {0};

		if (ReadFile(getHandle(), szBuff, 1, &dwBytesRead, NULL)) {

			std::cout << szBuff[0];
		}
	}
#endif

}

void WindowsSerialPort::close(void) {

	if (isOpen()) {
		CloseHandle(getHandle());
		setOpen(false);
		setHandle(INVALID_HANDLE_VALUE);
	}
}

