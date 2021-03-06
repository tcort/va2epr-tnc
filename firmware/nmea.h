/*
 * va2epr-tnc firmware - terminal node controller firmware for va2epr-tnc
 * Copyright (C) 2012, 2013 Thomas Cort <linuxgeek@gmail.com>
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

#ifndef __NMEA_H
#define __NMEA_H

/* Yes there are other message types, but they aren't used by the firmware (yet) */
enum nmea_sentence_type { GGA = 1, RMC = 2 } sentence_type;

struct coordinate {

	unsigned char cardinal_direction; /* N, S, E, W */

	/* NULL terminated strings */
	unsigned char hours[4];
	unsigned char minutes[3];
	unsigned char decimal[3];
};

struct nmea_coordinates {

	unsigned char valid;
	unsigned char gpstime[7]; /* HHMMSS\0 */
	struct coordinate latitude;
	struct coordinate longitude;

};

unsigned int atoui(unsigned char *s);
int gpstime_diff(unsigned int x, unsigned int y);

unsigned char nmea_checksum(unsigned char *s);
unsigned char nmea_extact_checksum(unsigned char *s);
unsigned char nmea_validate(unsigned char *s);
char nmea_extract_coordinates(unsigned char *s, struct nmea_coordinates *coordinates);

#endif
