/*
 * This file takes care of setting up the execution environment.
 * and the main control functions.
 */

#include <avr/interrupt.h>
#include <avr/io.h>

#include "afsk.h"
#include "gps.h"
#include "kiss.h"

int main(void) {

	volatile int i = 0;

	afsk_init();

	/* Untested code, uncomment as you test */
	/* gps_init(); */
	/* kiss_init(); */
	sei();

	rx();
	
	while (1) {

		/*
		 * do nothing (for now) as everything
		 * so far is interrupt based.
		 */
		i++;

		/* TODO -- something like this but full CSMA
		if (!carrier_sense && !kiss_rx_buffer_empty()) {
			tx();
			while (!kiss_rx_buffer_empty()) {
				  ;
			}
			rx();
		}
		 */
		
		/* TODO when needed, refresh GPS data. */
	}

	return 0;
}
