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

#ifndef __GPS_H
#define __GPS_H

/*
 * Baud Rate
 */

#define GPS_BAUDRATE 4800
#define GPS_UBRR_VAL (((F_CPU / (GPS_BAUDRATE * 16UL))) - 1)

/*
 * Prototypes
 */

void gps_init(void);
void gps_enable(void);
void gps_disable(void);
char gps_is_connected(void);
struct nmea_coordinates	*gps_get_coords(void);

#endif
