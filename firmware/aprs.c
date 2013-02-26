/*
 * va2epr-tnc firmware - terminal node controller firmware for va2epr-tnc
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

#include <string.h>

#include "aprs.h"
#include "ax25.h"
#include "conf.h"
#include "kiss.h"

static unsigned int send_address(unsigned int crc, unsigned char *s, unsigned char last_bit_one) {

	unsigned int i;
	unsigned char c;

	for (i = 0; s[i]; i++) {

		if (s[i + 1] == '\0' && last_bit_one) {
			c = (s[i] << 1) | 0x01;
		} else {
			c = s[i] << 1;
		}

		crc = crc16(crc, c);
		kiss_rx_buffer_queue(c);
	}

	return crc;
}

static unsigned int send_byte(unsigned int crc, unsigned char c) {

	crc = crc16(crc, c);
	kiss_rx_buffer_queue(c);

	return c;
}

void beacon(void) {

	unsigned int crc = INITIAL_CRC16_VALUE;
	unsigned char crcl;
	unsigned char crch;
	unsigned char addr[8];
	unsigned int i;

	/*
	 * calculate delay values here so that we aren't using a lot
	 * of CPU when the device switches into TX mode and starts
	 * sending. We have to be able to fill the buffer faster
	 * than we can send.
	 */

	unsigned int txdelay = TXDELAY;
	unsigned int txtail = TXTAIL;

	kiss_rx_buffer_queue(AX25_FLAG); /* must send at least one */

	for (i = 0; i < txdelay; i++) {
		kiss_rx_buffer_queue(AX25_FLAG);
	}

	/* addressing */

	strcpy((char *) addr, "APAVR00");
	crc = send_address(crc, addr, 0);

	strcpy((char *) addr, "VA2EPR0"); /* TODO use conf.callsign here */
	crc = send_address(crc, addr, 0);

	strcpy((char *) addr, "RELAY 0");
	crc = send_address(crc, addr, 0);

	strcpy((char *) addr, "WIDE2 2");
	crc = send_address(crc, addr, 1);

	/* control field */
	crc = send_byte(crc, AX25_APRS_UI_FRAME);

	/* protocol id */
	crc = send_byte(crc, AX25_PROTO_NO_LAYER3);

	/* TODO send payload */

	/* Calculate the high and low parts of the final CRC */
	crcl = crc ^ 0xff;
	crch = (crc >> 8) ^ 0xff;

	/* Send the CRC bytes */
	kiss_rx_buffer_queue(crcl);
	kiss_rx_buffer_queue(crch);

	for (i = 0; i < txtail; i++) {
		kiss_rx_buffer_queue(AX25_FLAG);
	}

	kiss_rx_buffer_queue(AX25_FLAG); /* must send at least one */
}
