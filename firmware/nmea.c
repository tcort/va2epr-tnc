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
 * This file provides some simple NMEA 0183 parsing and checksumming functions.
 * It does just enough to validate NMEA sentences and extract GPS coordinates.
 * No point in filling up program memory with more features than are needed.
 */

#include "nmea.h"

/*
 * Produce a checksum of the NMEA 0183 sentence (stuff between $ and *)
 *
 * Remember: the number after the * in NMEA is in hex.
 */
unsigned char nmea_checksum(unsigned char *s) {

	unsigned char checksum = 0x00;

	/* Skip the '$' if it's present */
	if (*s == '$') {
		s++;
	}

	/* Go until end of string is reached or '*' is reached */
	while (*s && *s != '*') {
		checksum ^= *s;
		s++;
	}

	return checksum;
}

/*
 * Utility function to convert between ascii hex and decimal
 */
static unsigned char hex2deci(unsigned char c) {

	if (c >= '0' && c <= '9') {

		return c - '0';

	} else if (c >= 'A' && c <= 'F') {

		return c - 'A' + 10;

	} else if (c >= 'a' && c <= 'f') {

		return c - 'a' + 10;

	} else {

		/* If I don't see 0-9, a-f, or A-F I return 0. */
		return 0;
	}
}

/*
 * Extract the NMEA checksum from the end of the sentence, converting
 * the hex digits to decimal.
 */
unsigned char nmea_extact_checksum(unsigned char *s) {

	unsigned char checksum = 0x00;

	/* Go until end of string is reached or '*' is reached */
	while (*s && *s != '*') {
		s++;
	}

	/* if '*' at pointer and we have 2 more characters in the string, convert from hex to deci */
	if (*s == '*' && *(s + 1) && *(s + 2)) {

		s++; /* move to first hex digit */
		checksum = hex2deci(*s) << 4;

		s++; /* move to second hex digit */
		checksum |= hex2deci(*s);
	}

	return checksum;
}

/*
 * Compute the checksum of an NMEA sentence and compare the result
 * to the checksum provided at the end of the sentence. If the checksums
 * match, return 1, else return 0.
 *
 * Note: a bad checksum is an indication of an error. A good checksum is
 * an indication that there "probably" isn't an error.
 */
unsigned char nmea_validate(unsigned char *s) {

	unsigned char checksum_expected;
	unsigned char checksum_actual;

	checksum_expected = nmea_extact_checksum(s);
	checksum_actual = nmea_checksum(s);

	return (checksum_expected == checksum_actual);
}

/*
 * Advance the pointer to the character after the next comma
 * or NULL.
 */
static unsigned char *nmea_next_field(unsigned char *s) {

	while (*s && *s != ',') {
		s++;
	}

	if (*s == ',') {
		s++;
	}

	return s;
}

/*
 * Extacts the coordinates from a GGA or RMC NMEA sentence.
 * Returns 0 for OK, -1 for parse error (or invalid sentence)
 */
char nmea_extract_coordinates(unsigned char *s, struct nmea_coordinates *coordinates) {

	/* Does the checksum check out? */
	if (!nmea_validate(s)) {

		return -1;
	}

	/* Skip the '$', we don't care about it */
	if (*s == '$') {
		s++;
	}

	/* Parse Message Type */
	if (s[2] == '\0' || s[3] == '\0' || s[4] == '\0') {
		return -1;
	} else if (s[2] == 'G' && s[3] == 'G' && s[4] == 'A') {
		sentence_type = GGA;
	} else if (s[2] == 'R' && s[3] == 'M' && s[4] == 'C') {
		sentence_type = RMC;
	} else {
		/* no coordinates here, so just return */
		return -1;
	}

	s = nmea_next_field(s); /* Move past GPGGA/GPRMC field */
	if (sentence_type == RMC) {
		s = nmea_next_field(s); /* Move on to status */
		if (s[0] == ',' || s[0] == 'V') { /* is field empty or void ('V')? */
			return -1;
		}
	}

	s = nmea_next_field(s); /* Move on to longitude */
	if (s[0] == ',') {
		return -1;
	}
	coordinates->latitude.hours[0] = s[0];
	coordinates->latitude.hours[1] = s[1];
	coordinates->latitude.hours[2] = '\0';

	coordinates->latitude.minutes[0] = s[2];
	coordinates->latitude.minutes[1] = s[3];
	coordinates->latitude.minutes[2] = '\0';

	if (s[4] == '.' && s[5] != ',') {
		coordinates->latitude.decimal[0] = s[5];
		if (s[6] != ',') {
			coordinates->latitude.decimal[1] = s[6];
		} else {
			coordinates->latitude.decimal[1] = '0';
		}
	
	} else {
		coordinates->latitude.decimal[0] = '0';
		coordinates->latitude.decimal[1] = '0';
	}
	coordinates->latitude.decimal[2] = '\0';

	s = nmea_next_field(s); /* Move on to cardinal direction */
		if (s[0] == ',') {
		return -1;
	}

	coordinates->latitude.cardinal_direction = s[0];

	s = nmea_next_field(s); /* Move on to longitude */
	if (s[0] == ',') {
		return -1;
	}
	coordinates->longitude.hours[0] = s[0];
	coordinates->longitude.hours[1] = s[1];
	coordinates->longitude.hours[2] = s[2];
	coordinates->longitude.hours[3] = '\0';

	coordinates->longitude.minutes[0] = s[3];
	coordinates->longitude.minutes[1] = s[4];
	coordinates->longitude.minutes[2] = '\0';

	if (s[5] == '.' && s[6] != ',') {
		coordinates->longitude.decimal[0] = s[6];
		if (s[7] != ',') {
			coordinates->longitude.decimal[1] = s[7];
		} else {
			coordinates->longitude.decimal[1] = '0';
		}
	} else {
		coordinates->longitude.decimal[0] = '0';
		coordinates->longitude.decimal[1] = '0';
	}

	coordinates->longitude.decimal[2] = '\0';

	s = nmea_next_field(s);  /* Move on to cardinal direction */
	if (s[0] == ',') {
		return -1;
	}
	coordinates->longitude.cardinal_direction = s[0];

	if (sentence_type == GGA) {
		s = nmea_next_field(s); /* Move on to Fix Quality */
		if (s[0] == ',' || s[0] == '0') { /* is empty field or invalid (0)? */
			return -1;
		}
	}

	/* if we get to the end without returning due to error, then the coordinates are valid */
	return 0;
}
