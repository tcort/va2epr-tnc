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

#ifndef __APRS_H
#define __APRS_H

#include "conf.h"

/*
 * Equations to determine how many AX.25 flags to send for TX Delay
 * and TX Tail. Delays are in 10ms units. We convert that to millis
 * and divide by the time it takes to send 8 bits @ 1200 baud.
 */
#define TXDELAY ((config.tx_delay * 0.01) / (8.0 / 1200.0))
#define TXTAIL  ((config.tx_tail  * 0.01) / (8.0 / 1200.0))

#define AX25_FLAG (0x7e)
#define AX25_APRS_UI_FRAME (0x03)
#define AX25_PROTO_NO_LAYER3 (0xf0)

#define INITIAL_CRC16_VALUE (0xffff)

#define CRC16CCITT_POLYNOMIAL (0x8408)

#endif
