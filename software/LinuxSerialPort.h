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

#ifndef __LINUXSERIALPORT_H
#define __LINUXSERIALPORT_H

#include <list>
#include <string>

#include "SerialPort.h"

class LinuxSerialPort : public SerialPort {

	public:
		LinuxSerialPort(void);
		~LinuxSerialPort(void);

		void open(void);
		void close(void);

	protected:
		int getFd() { return _fd; }
		void setFd(int fd) { _fd = fd; }

	private:
		int _fd;
};

#endif
