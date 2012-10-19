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
 * Implement p-persistent Carrier Sense Multiple Access (CSMA) with collision avoidance
 * as described in part 6 of the KISS protocol specification.
 */

#include <stdlib.h>

#include <util/delay.h>

#include "conf.h"
#include "csma.h"

/*
 * Set this flag when a carrier is detected.
 * Clear this flag when there is no carrier.
 *
 * Note: when setting, use the TONE_1200HZ and TONE_2200HZ macros.
 */
volatile unsigned int carrier_sense = 0;

/*
 * Block until it's OK to transmit.
 */
void csma_obtain_slot(void) {
	
	unsigned char dice;
	unsigned char i;

	while (1) {
		
		/* is the channel clear? */
		if (carrier_sense) {
			
			/* nope, check again */
			continue;
		}
		
		dice = (rand() & 0xff); /* try our luck */
		
		if (dice <= config.p) {

			/*
			 * if the random number is less than or equal to P,
			 * then it is OK to transmit.
			 */
			break;
			
		} else {

			/* wait for next slot time to try again */
			for (i = 0; i < config.slot_time; i++) {
				_delay_ms(10);
			}
		}		
	}
}
