/*
 * This file implements 1200 baud AFSK NRZI with 1200 Hz and 2200 Hz tones.
 *
 * Current Status
 * --------------
 * Can differentiate between 1200 Hz, 2200 Hz, and tones outside of
 * a given tolerance. Can output waveforms at 1200 Hz and 2200 Hz. This is
 * enough to show that the circuits used in the schematic are viable.
 *
 * The code needs to be enhanced to capture incoming data.
 * The code needs to be enhanced to output data from a buffer. This can probably be
 * done by using Timer 0 or Timer 3 to periodically sample data or periodically
 * change OCR2A to TONE_1200HZ or TONE_2200HZ. Also, the code needs to disable
 * timer 1 when in TX mode and disable timer 2 when in RX mode. When in TX mode,
 * consider using a watchdog timer to help prevent getting stuck in TX mode.
 *
 * Ports and Peripherals Used
 * --------------------------
 *
 * Timer 1 (Input Capture)
 * Timer 2 (Waveform Generation)
 *
 * PORTA (AFSK Output)
 * PD6 (Input Capture Input)
 * PD7 (Push to Talk Output)
 */

#include <avr/interrupt.h>
#include <avr/io.h>

/*
 * For a 16 sample 2200 Hz tone, we need to change the output 35200
 * (i.e. 2200 * 16) times per second. For a 16 sample 1200 Hz tone,
 * we need to change the output 19200 (i.e. 1200 * 16) times per second.
 * Using a prescalar of 4, we should set TCNT2 to the following values
 * via the 'tone' variable:
 *
 *   For 1200 Hz, 14745600/8/19200 - 1 = 95				Exactly 1200 Hz
 *   For 2200 Hz, 14745600/8/35200 - 1 = 51.36 (51)		Approx  2215 Hz (0.6% error)
 */
#define TONE_1200HZ 95
#define TONE_2200HZ 51

/* precomputed sinewave from tools/sine.c */
/* TODO double or quadruple the size of the table */
unsigned char sinewave[16] = {
	127, 176, 217, 245, 255, 245, 217, 176,
	127,  78,  37,   9,   0,   9,  37,  78
};

/*
 * Set this flag when a carrier (1200 Hz or 2200 Hz tone) is detected.
 * Remove it when there is no carrier. This is used for P-persistent
 * Carrier Sense Multiple Access (CSMA).
 */
/* TODO set/unset this flag in the tx/rx code */
unsigned int carrier_sense = 0;

/* timer1 input capture housekeeping */
/* TODO choose better names */
unsigned int last = 0;
unsigned int overflows = 0;
unsigned int period = 0;

/* setup ports and timers */
void afsk_init(void) {

	/* AFSK DAC is on PORTA */
	DDRA |= 0xFF; /* AFSK output */
	PORTA = 0x00; /* initialize */

	/* Timer 2 CTC, pre-scalar 8 */
	TCCR2A |= (1<<WGM21); /* CTC */
	TCCR2B |= (1<<CS21); /* pre-scalar = 8 */
	TCNT2 = 0x00; /* initialize counter to 0 */
	OCR2A = 0xff; /* set with some default value */
	TIMSK2 |= (1<<OCIE2A); /* enable compare match interrupt */

	/* Input Capture on PD6 */
	DDRD &= ~(1<<PD6); /* ICP as Input */
	PORTD |= (1<<PD6); /* enable pull-up on ICP1 */

	/* Timer 1 ICP */
	TCCR2A = 0x00; /* Normal Mode */
	TCCR1B |= ((1<<ICNC1)|(1<<ICES1)|(1<<CS11)); /* noise canceler, rising edge, pre-scalar = 8 */
	TIMSK1 |= ((1<<ICIE1)|(1<<TOIE1)); /* enable overflow and input capture interrupts */

	/* Push to Talk line on PD7 */
	DDRD |= (1<<PD7); /* Push To Talk line */
	PORTD &= ~(1<<PD7); /* PTT OFF */

}

void tx(void) {

	PORTD |= (1<<PD7); /* PTT ON */
	/* TODO: disable timer1 */
	/* TODO: enable timer2 */
}

void rx(void) {

	/* TODO: enable timer1 */
	/* TODO: disable timer2 */
	PORTD &= ~(1<<PD7); /* PTT OFF */
}

/* capture the period of an input waveform */
ISR(TIMER1_CAPT_vect) {

	/* computer period taking into account overflows */
	period = ICR1 + (0xffff * overflows) - last;
	last = ICR1; /* keep track of when the last period ended */
	overflows = 0; /* reset overflow counter */


	/* TEST CODE

	14745600/8 = 1843200 Hz = 0.000000542534722 seconds per tick
	1/1200 = 0.000833333333 seconds per tick
	0.000833333333 / 0.000000542534722  = 1536 ticks
	0.00045454545 / 0.000000542534722 = 838 ticks
	(1536-838)/2 = 349

	*/
	if (period >= 1187 && period < 1885) {
		/* 1200 Hz +/- 500 Hz */
		/* empty */;
	} else if (period > 489 && period < 1187) {
		/* 2200 Hz +/- 500 Hz */
		/* empty */;
	} else {
		/* empty */;
	}
}

/* keep track of timer 1 overflows */
ISR(TIMER1_OVF_vect) {

	overflows++;
}

/* generate the output waveform */
ISR(TIMER2_COMPA_vect) {

	volatile static unsigned char sinewave_index = 0;

	/*
	 * output sinewave to PORTA.
	 * PORTB is to be connected to an external R-2R ladder.
	 * The ladder output should be attenuated to 1Vpp and
	 * AC coupled to create line level audio.
	 */
	PORTA = sinewave[sinewave_index++];

	/* keep sinewave_index in the range 0-15 */
	sinewave_index &= 0x0f;
}
