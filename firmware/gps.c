/*
 * va2epr-tnc firmware - terminal node controller firmware for va2epr-tnc
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

/*
 * The GPS module being used in this project outputs data according to
 * NMEA0183 V2.2. It supports the following sentences:
 *
 *   GGA ~ Global Positioning System Fix Data. Time, Position and fix related data for a GPS receiver
 *   GSV ~ Satellites in view
 *   GSA ~ GPS DOP and active satellites
 *   RMC ~ Recommended Minimum Navigation Information
 *   VTG ~ Track Made Good and Ground Speed (optional)
 *   GLL ~ Geographic Position – Latitude/Longitude (optional)
 *
 * From what I've read, the GPS module continuously outputs
 * GPGGA, GPGSA, GPGSV, and GPRMC sentences in NMEA mode. From what I observed,
 * not all sentences appear all of the time. For example, GPGSV isn't sent
 * when no satellites are in view.
 *
 *
 * When I plugged it in for the first time I got this about every second...
 *
 *   $GPGGA,000425.035,0000.0000,N,00000.0000,E,0,00,,0.0,M,0.0,M,,0000*46
 *   $GPGSA,A,1,,,,,,,,,,,,,,,*1E
 *   $GPGSV,1,1,00*79
 *   $GPRMC,000425.035,V,0000.0000,N,00000.0000,E,,,150209,,,N*7C
 *
 * When no satellites were in view, there were no $GPGSV sentences.
 *
 * Coordinates use the WGS 84 DATUM
 *
 * Ports and Peripherals Used
 * --------------------------
 *
 * USART1 (Connected to the GPS)
 *
 * PD2 (RXD)
 * PD3 (TXD)
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "kiss.h"
#include "gps.h"
#include "nmea.h"

/*
 * Initialize the GPS interface
 */
void gps_init(void) {

	/* 4800 baud */
	UBRR1H = (GPS_UBRR_VAL >> 8);
	UBRR1L = (GPS_UBRR_VAL & 0xFF);

	UCSR1C |= (1 << UCSZ10) | (1 << UCSZ11);
}

void gps_enable(void) {

	/* Enable RX/TX and RX interrupt */
	UCSR1B |= ((1 <<  RXEN1) | (1 << TXEN1) | (1<<RXCIE0));

}

char gps_is_connected(void) {

	/* state of the GPS (-1 = unsure, 0 = not connected, 1 = connected) */
	static unsigned int status = -1;

	/* index */
	unsigned int x = 0;

	/* temp variable for read USART data */
	unsigned char c;

	/* number of bytes read from USART */
	unsigned char count = 0;

	if (status == -1) {

		status = 0;

		/* Turn on USART */
		UCSR1B |= ((1 <<  RXEN1) | (1 << TXEN1));

		for (x = 0; x < 2048; x++) {

			if (UCSR1A & (1<<RXC1)) {

				c = UDR1;
				c = c; /* avoid unused variable warning */

				count++;

				/*
				 * if I can read more than 16 bytes in 2
				 * seconds, then the GPS is probably
				 * connected and sending data.
				 */
				if (count >= 16) {
					status = 1;
					break;
				}
			}

			_delay_ms(1);

		}

		/* Turn OFF USART */
		UCSR1B &= ~((1 <<  RXEN1) | (1 << TXEN1));

	}

	return status;
}

void gps_disable(void) {

	/* Disable RX/TX and RX interrupt */
	UCSR1B &= ~((1 <<  RXEN1) | (1 << TXEN1) | (1<<RXCIE0));

}

/*
 * Receive Data from the GPS
 * For testing/debugging, just output it to the PC
 */
ISR(USART1_RX_vect) {

	static unsigned char c = 0x00;

	/* Read from UART */
	c = UDR1;

	c = c; /* avoid unused variable error */
	/* TODO buffer a line of data at a time */
}
