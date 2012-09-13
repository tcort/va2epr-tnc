/*
 * This file takes care of setting up the execution environment
 * and the main control functions.
 */

#include <avr/interrupt.h>
#include <avr/io.h>

#include "afsk.h"
#include "conf.h"
#include "csma.h"
#include "gps.h"
#include "kiss.h"

int main(void) {

	volatile int i = 0;

	config_read();
	
	afsk_init();
	kiss_init();
	/* gps_init(); */
	
	sei();
	
	rx();
	
	while (1) {

		/* do we have any data to transmit? */
		if (!kiss_rx_buffer_empty()) {
			
			/* wait for the channel to become clear */
			csma_obtain_slot();
			
			tx(); /* enter transmit mode */
			
			/* wait until everything has been sent */
			while (!kiss_rx_buffer_empty()) {
				i++; 
			}
			
			rx(); /* go back to receive mode */
		}
		
		/* TODO occasionally get some data from the GPS */
	}

	return 0;
}
