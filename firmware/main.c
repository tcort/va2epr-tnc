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

/*
 * This file takes care of setting up the execution environment
 * and the main control functions.
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "afsk.h"
#include "aprs.h"
#include "conf.h"
#include "csma.h"
#include "gps.h"
#include "uart.h"
#include "nmea.h"

int main(void) {

	volatile char i = 0;
	void (*idle_mode)(void);
	unsigned int last_beacon_timestamp = 0;

	config_read();
	afsk_init();
	uart_init();
	gps_init();

	sei();

	/* When not transmitting the TNC is either receiving
	 * or getting position location from the GPS. If a GPS
	 * is connected, it is assumed that the user is a mobile
	 * and doesn't have a computer connected. Thus she has no
	 * need to receive.
	 */
	idle_mode = gps_is_connected() ? &notxrx : &rx;
	(idle_mode)();

	while (1) {
/*
		* Simple test code for indoor demo without GPS.
		* N 45 27.300 W 075 45.870

		tx_buffer_queue(AX25_FLAG);
		tx_buffer_queue(AX25_FLAG);
		tx_buffer_queue(AX25_FLAG);
		tx_buffer_queue('{');
		tx_buffer_queue('V'); tx_buffer_queue('A'); tx_buffer_queue('2'); tx_buffer_queue('E'); tx_buffer_queue('P'); tx_buffer_queue('R');
		tx_buffer_queue('@');
		tx_buffer_queue('N'); tx_buffer_queue(' '); tx_buffer_queue('4'); tx_buffer_queue('5'); tx_buffer_queue(' ');
		tx_buffer_queue('2'); tx_buffer_queue('7'); tx_buffer_queue('.'); tx_buffer_queue('3'); tx_buffer_queue('0'); tx_buffer_queue('0');
		tx_buffer_queue(',');
		tx_buffer_queue('W'); tx_buffer_queue(' '); tx_buffer_queue('0'); tx_buffer_queue('7'); tx_buffer_queue('5'); tx_buffer_queue(' ');
		tx_buffer_queue('4'); tx_buffer_queue('5'); tx_buffer_queue('.'); tx_buffer_queue('8'); tx_buffer_queue('7'); tx_buffer_queue('0');
		tx_buffer_queue('|');
		tx_buffer_queue('0');
		tx_buffer_queue(',');
		tx_buffer_queue('0');
		tx_buffer_queue('}');
		tx_buffer_queue(AX25_FLAG);
		tx_buffer_queue(AX25_FLAG);
		tx_buffer_queue(AX25_FLAG);
*/
		/* do we have any data to transmit? */
		if (!tx_buffer_empty()) {
			
			/* wait for the channel to become clear */
			csma_obtain_slot();
			
			tx(); /* enter transmit mode */

			/* wait until everything has been sent */
			while (!tx_buffer_empty()) {
				i++; 
			}

			(*idle_mode)(); /* go back to receive mode if no GPS */
		}

		for (i = 0; i < 50; i++) {
			_delay_ms(100);
		}

		if (idle_mode == &notxrx) {
			unsigned int current_timestamp;
			struct nmea_coordinates	*location;

			gps_enable();

			do {
				/*
				 * loop until we get valid coordinates. Note,
				 * this could take forever if the GPS is indoors.
				 */
				location = gps_get_coords();
				_delay_ms(100);

			} while (!location->valid);

			gps_disable();

			/* Beacon location every 10 minutes */
			current_timestamp = atoui(location->gpstime);
			if (gpstime_diff(current_timestamp, last_beacon_timestamp) >= 10) {


				aprs_beacon();
				last_beacon_timestamp = current_timestamp;
			}
		}
	}

	return 0;
}

