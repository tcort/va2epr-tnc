# Name of program
TARGET=va2epr-tnc

# Settings for programming an Arduino Duemilanove (ATMega328p) on Linux
MCU=atmega1284p
PART=m1284p
F_CPU=14745600
PROGRAMMER=dragon_isp
BAUD=57600
DEVICE=/dev/ttyUSB0

# Compiler and Uploader options
CC=avr-gcc
CFLAGS=-Os -pedantic -Wall -Werror -mcall-prologues -mmcu=$(MCU) -DF_CPU=$(F_CPU) 
OBJ2HEX=avr-objcopy
AVRDUDE=avrdude

$(TARGET).hex: $(TARGET).obj
	$(OBJ2HEX) -R .eeprom -O ihex $(TARGET).obj $(TARGET).hex

$(TARGET).obj: ax25.o afsk.o conf.o csma.o gps.o kiss.o main.o nmea.o
	$(CC) $(CFLAGS) ax25.o afsk.o conf.o csma.o gps.o kiss.o main.o nmea.o -o $(TARGET).obj

afsk.o: afsk.c afsk.h ax25.h csma.h
	$(CC) $(CFLAGS) afsk.c -c -o afsk.o

ax25.o: ax25.c ax25.h
	$(CC) $(CFLAGS) ax25.c -c -o ax25.o

conf.o: conf.c conf.h
	$(CC) $(CFLAGS) conf.c -c -o conf.o

csma.o: csma.c csma.h conf.h
	$(CC) $(CFLAGS) csma.c -c -o csma.o

gps.o: gps.c gps.h nmea.h
	$(CC) $(CFLAGS) gps.c -c -o gps.o

kiss.o: kiss.c kiss.h afsk.h ax25.h conf.h
	$(CC) $(CFLAGS) kiss.c -c -o kiss.o

main.o: main.c afsk.h conf.h csma.h gps.h kiss.h
	$(CC) $(CFLAGS) main.c -c -o main.o

nmea.o: nmea.c nmea.h
	 $(CC) $(CFLAGS) nmea.c -c -o nmea.o

upload: $(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(PART) -P $(DEVICE) -b $(BAUD) -U flash:w:$(TARGET).hex 

clean:
	rm -f $(TARGET).hex $(TARGET).obj *.o
