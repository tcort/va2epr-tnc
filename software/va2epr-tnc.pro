# va2epr-tnc software - terminal node controller software for va2epr-tnc
# Copyright (C) 2012 Thomas Cort <va2epr@rac.ca>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

TEMPLATE = app
TARGET = va2epr-tnc
DEPENDPATH += .
INCLUDEPATH += .

QT += webkit

# Code

HEADERS += AboutDialog.h   Console.h   main.h   SerialPort.h   va2epr_tnc.h
SOURCES += AboutDialog.cpp Console.cpp main.cpp SerialPort.cpp va2epr_tnc.cpp

win32 {
	HEADERS += WindowsSerialPort.h
	SOURCES += WindowsSerialPort.cpp
}

unix {
	HEADERS += LinuxSerialPort.h
	SOURCES += LinuxSerialPort.cpp
}
