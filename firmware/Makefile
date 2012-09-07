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
CFLAGS=-Os -Wall -Werror -mcall-prologues -mmcu=$(MCU) -DF_CPU=$(F_CPU) 
OBJ2HEX=avr-objcopy
AVRDUDE=avrdude

$(TARGET).hex: $(TARGET).obj
	$(OBJ2HEX) -R .eeprom -O ihex $(TARGET).obj $(TARGET).hex

$(TARGET).obj: afsk.o gps.o kiss.o main.o
	$(CC) $(CFLAGS) afsk.o gps.o kiss.o main.o -o $(TARGET).obj

afsk.o: afsk.c afsk.h
	$(CC) $(CFLAGS) afsk.c -c -o afsk.o

gps.o: gps.c gps.h
	$(CC) $(CFLAGS) gps.c -c -o gps.o

kiss.o: kiss.c kiss.h
	$(CC) $(CFLAGS) kiss.c -c -o kiss.o

main.o: main.c
	$(CC) $(CFLAGS) main.c -c -o main.o

upload: $(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(PART) -P $(DEVICE) -b $(BAUD) -U flash:w:$(TARGET).hex 

clean:
	rm -f $(TARGET).hex $(TARGET).obj *.o
