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

#ifndef __CONF_H
#define __CONF_H

/*
 * The version of the struct conf. This might be used in the future if
 * I make changes to struct conf and wish to support old EEPROMs.
 *
 * List version history here:
 *
 *  - 0x54  initial version
 */
#define CONF_VERSION (0x54)

struct conf {
	unsigned char version; /* version of 'struct conf', used if I need to make changes to this struct in the future. */
	unsigned char tx_delay; /* the transmitter key-up delay in 10 ms units. */
	unsigned char p; /* the  persistence parameter,  p, scaled to the range 0 - 255 with the following formula: P = p * 256 - 1 */
	unsigned char slot_time; /* the slot interval in 10 ms units. */
	unsigned char tx_tail; /* OBSOLETE - the time to hold up the TX after the FCS has been sent, in 10 ms units. */
	unsigned char full_duplex; /* 0 for half duplex mode, non-zero for full duplex mode. */
	unsigned char callsign[7]; /* '\0' terminated call sign (for APRS Tracker) */
};

extern struct conf config;

void config_read(void);
void config_write(void);

#endif
