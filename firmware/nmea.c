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
static inline unsigned char hex2deci(unsigned char c) {

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

