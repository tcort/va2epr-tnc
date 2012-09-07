#include <avr/interrupt.h>
#include <avr/io.h>

#include "afsk.h"

/* TEST CODE */
int main(void) {

	volatile int i = 0;
	afsk_init();
	sei();

	OCR2A = 95; /* 1200 Hz */

	while (1) {

		i++;
	}

	return 0;
}
