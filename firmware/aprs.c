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

#include <stdio.h>
#include <string.h>

#include "aprs.h"
#include "afsk.h"
#include "conf.h"
#include "gps.h"
#include "nmea.h"

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

/*
 * Update the crc for the byte 'c' and queue it up for transmission.
 * Returns the updated crc value.
 */
static inline unsigned int send_byte(unsigned int crc, unsigned char c) {

	crc = crc16(crc, c);
	tx_buffer_queue(c);

	return crc;
}

static inline unsigned int send_string(unsigned int crc, unsigned char *s) {

	while (*s) {
		crc = send_byte(crc, *s);
		s++;
	}

	return crc;
}

/*
 * Beacon an APRS message.
 */
void aprs_beacon(void) {

	unsigned int crc = INITIAL_CRC16_VALUE;

	unsigned char crcl;	/* low bits of crc */
	unsigned char crch;	/* high bits of crc */

	char value[256];	/* tmp buffer for string formatting output */
	unsigned int i;

	/*
	 * calculate delay values here so that we aren't using a lot
	 * of CPU when the device switches into TX mode and starts
	 * sending. We have to be able to fill the buffer faster
	 * than we can send.
	 */
	unsigned int txdelay = TXDELAY;
	unsigned int txtail = TXTAIL;

	struct nmea_coordinates *coords;

	coords = gps_get_coords();

	/*
	 * Start transmitting...
	 * Here we have a bit of a race. We need to queue up the data
	 * faster than it can be sent. In testing, buffer underfill wasn't
	 * an issue.
	 */

	tx_buffer_queue(AX25_FLAG);	/* must send at least one */

	for (i = 0; i < txdelay; i++) {
		tx_buffer_queue(AX25_FLAG);
	}

	tx_buffer_queue('{');
	tx_buffer_queue('m');
	tx_buffer_queue(':');

	memset(value, '\0', 256);
	snprintf(value, 256, "%s@%c %s %s.%s,%c %s %s.%s",
		config.callsign,
		coords->longitude.cardinal_direction,
		(char *) coords->longitude.hours,
		(char *) coords->longitude.minutes,
		(char *) coords->longitude.decimal,
		coords->latitude.cardinal_direction,
		(char *) coords->latitude.hours,
		(char *) coords->latitude.minutes,
		(char *) coords->latitude.decimal
	);

	crc = send_string(crc, (unsigned char *) value);

	/* Calculate the high and low parts of the final CRC */
	crcl = crc ^ 0xff;
	crch = (crc >> 8) ^ 0xff;

	/* Send the CRC bytes */
	memset(value, '\0', 256);
	snprintf(value, 256, "|%d,%d", crch, crcl);
	send_string(crc,(unsigned char *) value);

	tx_buffer_queue('}');

	for (i = 0; i < txtail; i++) {
		tx_buffer_queue(AX25_FLAG);
	}

	tx_buffer_queue(AX25_FLAG); /* must send at least one */

}
