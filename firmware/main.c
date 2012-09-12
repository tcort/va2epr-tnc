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

	/* TODO test code remove me */
	OCR2A = 95; /* 1200 Hz */

	/* TODO put in receive mode -- rx(); */

	while (1) {

		/*
		 * do nothing (for now) as everything
		 * so far is interrupt based.
		 */
		i++;

		/* TODO -- something like this but full CSMA
		 * if (!carrier_sense && tx_ready) { tx(); rx(); }
		 */
	}

	return 0;
}
