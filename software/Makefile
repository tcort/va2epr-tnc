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

# Name of program
TARGET=va2epr-tnc

# Compiler and Options
CXX=g++
CXX_WIN32=i486-mingw32-g++
CFLAGS=-Os -pedantic -Wall -Werror

all: $(TARGET) $(TARGET).exe

$(TARGET).exe: SerialPort.win32.o main.win32.o
	$(CXX_WIN32) $(CFLAGS) SerialPort.win32.o main.win32.o -o $(TARGET).exe

$(TARGET): LinuxSerialPort.o SerialPort.o main.o
	$(CXX) $(CFLAGS) SerialPort.o LinuxSerialPort.o main.o -o $(TARGET)

main.win32.o: main.cpp
	$(CXX_WIN32) $(CFLAGS) main.cpp -c -o main.win32.o

main.o: main.cpp
	$(CXX) $(CFLAGS) main.cpp -c -o main.o

LinuxSerialPort.o: LinuxSerialPort.cpp LinuxSerialPort.h
	$(CXX) $(CFLAGS) LinuxSerialPort.cpp -c -o LinuxSerialPort.o

SerialPort.o: SerialPort.cpp SerialPort.h
	$(CXX) $(CFLAGS) SerialPort.cpp -c -o SerialPort.o

SerialPort.win32.o: SerialPort.cpp SerialPort.h
	$(CXX_WIN32) $(CFLAGS) SerialPort.cpp -c -o SerialPort.win32.o

clean:
	rm -f $(TARGET) $(TARGET).exe *.o
