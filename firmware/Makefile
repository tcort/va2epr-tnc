# va2epr-tnc firmware - terminal node controller firmware for va2epr-tnc
# Copyright (C) 2012, 2013 Thomas Cort <va2epr@rac.ca>
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

# Settings for programming an ATMega1284p on Linux with an AVR Dragon
MCU=atmega1284p
PART=m1284p
F_CPU=14745600
PROGRAMMER=dragon_jtag
BAUD=57600
DEVICE=usb

# Compiler and Uploader options
CC=avr-gcc
CFLAGS=-Os -pedantic -Wall -Werror -mcall-prologues -mmcu=$(MCU) -DF_CPU=$(F_CPU) 
OBJ2HEX=avr-objcopy
AVRDUDE=avrdude

all: $(TARGET).hex $(TARGET).eep
	avr-size $(TARGET).hex
	avr-size $(TARGET).eep

$(TARGET).hex: $(TARGET).obj
	$(OBJ2HEX) -R .eeprom -O ihex $(TARGET).obj $(TARGET).hex

$(TARGET).eep: $(TARGET).obj
	$(OBJ2HEX) -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex $(TARGET).obj $(TARGET).eep

$(TARGET).obj: afsk.o aprs.o conf.o csma.o gps.o uart.o main.o nmea.o
	$(CC) $(CFLAGS) afsk.o aprs.o conf.o csma.o gps.o uart.o main.o nmea.o -o $(TARGET).obj

afsk.o: afsk.c afsk.h aprs.h csma.h uart.h
	$(CC) $(CFLAGS) afsk.c -c -o afsk.o

aprs.o: aprs.c afsk.h aprs.h conf.h gps.h nmea.h
	$(CC) $(CFLAGS) aprs.c -c -o aprs.o

conf.o: conf.c conf.h
	$(CC) $(CFLAGS) conf.c -c -o conf.o

csma.o: csma.c csma.h conf.h
	$(CC) $(CFLAGS) csma.c -c -o csma.o

gps.o: gps.c gps.h nmea.h uart.h
	$(CC) $(CFLAGS) gps.c -c -o gps.o

main.o: main.c afsk.h aprs.h conf.h csma.h gps.h uart.h nmea.h
	$(CC) $(CFLAGS) main.c -c -o main.o

nmea.o: nmea.c nmea.h
	 $(CC) $(CFLAGS) nmea.c -c -o nmea.o

uart.o: uart.c uart.h afsk.h aprs.h conf.h
	$(CC) $(CFLAGS) uart.c -c -o uart.o

write-flash: $(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(PART) -P $(DEVICE) -b $(BAUD) -U flash:w:$(TARGET).hex

write-eeprom: $(TARGET).eep
	$(AVRDUDE) -c $(PROGRAMMER) -p $(PART) -P $(DEVICE) -b $(BAUD) -U eeprom:w:$(TARGET).eep

write-fuses:
	$(AVRDUDE) -c $(PROGRAMMER) -p $(PART) -P $(DEVICE) -b $(BAUD) -U lfuse:w:$(TARGET).lfuse:r -U hfuse:w:$(TARGET).hfuse:r -U efuse:w:$(TARGET).efuse:r

read-fuses:
	$(AVRDUDE) -c $(PROGRAMMER) -p $(PART) -P $(DEVICE) -b $(BAUD) -U lfuse:r:$(TARGET).lfuse:r -U hfuse:r:$(TARGET).hfuse:r -U efuse:r:$(TARGET).efuse:r

clean:
	rm -f $(TARGET).eep $(TARGET).hex $(TARGET).obj *.o
