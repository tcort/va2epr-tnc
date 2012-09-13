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