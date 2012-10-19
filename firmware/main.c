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
