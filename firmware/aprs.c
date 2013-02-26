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
#include "conf.h"
#include "kiss.h"

/*
 * Compute the Frame-Check Sequence (FCS) for AX.25
 *
 * AX.25 frames arrive as a series of bytes. Call this function
 * with each new byte. The initial CRC parameter should be 0xffff.
 * The BYTE parameter is the data that you wish to checksum.
 * Use the result of this function in the next crc16().
 *
 * Example (updating the crc as you process data):
 *
 *	unsigned int crc = 0xffff;
 *	// ... get a byte
 *	crc = crc16(crc, byte);
 *	// ... get another byte
 *	crc = crc16(crc, byte);
 *
 * This isn't the most efficient algorithm, but to me it's the
 * most understandable. Faster lookup table base methods exist, but two
 * of my aims are to maintain readability and reduce program size.
 * Far too many implementations have huge lookup thats or are
 * completely incomprehensible (do people really think that throwing
 * in inline assembly fragments will beat gcc's optimizer while
 * keeping the code readable/portable/maintainable/etc.?). For more
 * info to help understand the algorithm I implemented, please see
 * "Polynomial division with bytewise message XORing" on Wikipedia
 * http://en.wikipedia.org/wiki/Computation_of_CRC#Implementation
 *
 * Author's note: the AX.25 v2.2 spec doesn't explicitly mention
 * how to compute the frame check sequence. It just refers to
 * ISO 3309. I can't find that document anywhere for free and
 * iso.org says it's "Withdrawn". Enough of the sources I found
 * mentioned the same polynomial, so I went with that. I tested
 * my implementation against another and found the results to be
 * the same, so I'm pretty sure this is correct ;)
 *
 * Future work: it may be desirable to compute the CRC of a whole
 * frame at once instead of a byte at a time. That would save
 * a lot of function call overhead at the expense of having to
 * store the entire frame in memory. Let's keep this for now and
 * let this idea simmer the back burner until it is needed.
 */
static unsigned int crc16(unsigned int crc, unsigned char byte) {

	unsigned char i;

	crc ^= byte;

	/* for each bit in 'byte' */
	for (i = 0; i < 8; i++) {

		/* if LSB of 'crc' is HIGH (1) */
		if (crc & 0x0001) {
			/*
			 * Optimization Note:
			 * avr-gcc generates fewer instructions for
			 * "if (crc & 0x0001)" than for "if (crc << 15)"
			 */

			/* Shift Right by 1 and XOR the result with the polynomial */
			crc = (crc>>1) ^ CRC16CCITT_POLYNOMIAL;
		} else {

			/* Shift Right  by 1 */
			crc >>= 1;
		}
	}

	return crc;
}

static inline unsigned int send_byte(unsigned int crc, unsigned char c) {

	crc = crc16(crc, c);
	kiss_rx_buffer_queue(c);

	return crc;
}

static inline unsigned int send_address(unsigned int crc, unsigned char *s, unsigned char last_bit_one) {

	unsigned int i;
	unsigned char c;

	for (i = 0; s[i]; i++) {

		if (s[i + 1] == '\0' && last_bit_one) {
			c = (s[i] << 1) | 0x01;
		} else {
			c = s[i] << 1;
		}

		crc = send_byte(crc, c);
	}

	return crc;
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
