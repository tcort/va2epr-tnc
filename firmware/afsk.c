*
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

#include "afsk.h"
#include "csma.h"

/*
 * For a 16 sample 2200 Hz tone, I need to change the output 35200
 * (i.e. 2200 * 16) times per second. For a 16 sample 1200 Hz tone,
 * I need to change the output 19200 (i.e. 1200 * 16) times per second.
 * Using a pre-scalar of 4, I should set TCNT2 to the following values
 * via the 'tone' variable:
 *
 *   For 1200 Hz, 14745600/8/19200 - 1 = 95				Exactly 1200 Hz
 *   For 2200 Hz, 14745600/8/35200 - 1 = 51.36 (51)		Approx  2215 Hz (0.6% error)
 */
#define TONE_1200HZ 95
#define TONE_2200HZ 51

/*
 * These are the period thresholds that are used to decide if a
 * captured input is 1200 Hz or 2200 Hz. You can compute these
 * by dividing the period of the desired frequency by the
 * period of the counter.
 *
 * For 700 Hz, I computed a period of 2633 this way:
 *
 * 	(1/700) / (1/(F_CPU/8)) = 2633
 */
#define PERIOD_2200_MIN  682 /* 2.7 kHz */
#define PERIOD_2200_MAX 1084 /* 1.7 kHz */
#define PERIOD_1200_MIN 1084 /* 1.7 kHz */
#define PERIOD_1200_MAX 2633 /* 0.7 kHz */

/*
 * This is the value that is used to set OCR2A. It controls the
 * output frequency when in transmit mode by setting the correct
 * period. Set it to TONE_1200HZ or TONE_2200HZ.
 *
 * If you're looking for the variable that holds the RX frequency
 * search for carrier_sense.
 */
unsigned char afsk_output_frequency = TONE_1200HZ; /* initial frequency */

/*
 * frequency shift detected -- used by RX to identify changes in
 * carrier frequency. This indicates a logical '1' in the NRZI
 * flavour of AFSK. It also helps with synchronization.
 */
unsigned char shift_detect = 0;

/* precomputed sinewave from tools/sine.c */
/* TODO double or quadruple the size of the table */
unsigned char sinewave[16] = {
	127, 176, 217, 245, 255, 245, 217, 176,
	127,  78,  37,   9,   0,   9,  37,  78
};

/* timer1 input capture housekeeping */
unsigned int capture_last_time = 0;
unsigned int capture_overflows = 0;

/* setup ports and timers */
void afsk_init(void) {

	/* -- Output DAC (TX) -- */
	/* do all setup except enabling the compare match interrupt (done in tx()) */

	/* AFSK DAC is on PORTA */
	DDRA |= 0xFF; /* AFSK output */
	PORTA = 0x00; /* initialize */

	/* Timer 2 CTC, pre-scalar 8 */
	TCCR2A |= (1<<WGM21); /* CTC */
	TCCR2B |= (1<<CS21); /* pre-scalar = 8 */
	TCNT2 = 0x00; /* initialize counter to 0 */
	OCR2A = afsk_output_frequency; /* set with some default value */

	/* -- Input Capture (RX) -- */
	/* do all setup except enabling of overflow and input capture interrupts (done in rx()) */

	/* Input Capture on PD6 */
	DDRD &= ~(1<<PD6); /* ICP as Input */
	PORTD |= (1<<PD6); /* enable pull-up on ICP1 */

	/* Timer 1 ICP */
	TCCR1A = 0x00; /* Normal Mode */
	TCCR1B |= ((1<<ICNC1)|(1<<ICES1)|(1<<CS11)); /* noise canceler, rising edge, pre-scalar = 8 */

	/* -- Push to Talk -- */
	/* setup DDR and turn PTT OFF */

	/* Push to Talk line on PD7 */
	DDRD |= (1<<PD7); /* Push To Talk line */
	PORTD &= ~(1<<PD7); /* PTT OFF */

}

/*
 * Disable input capture, enable output DAC and Push-to-Talk line
 */
void tx(void) {

	TIMSK1 &= ~((1<<ICIE1)|(1<<TOIE1)); /* disable overflow and input capture interrupts */

	PORTD |= (1<<PD7); /* PTT ON */
	/* TODO do I want a delay here? */
	TIMSK2 |= (1<<OCIE2A); /* enable compare match interrupt */
}

/*
 * Disable output DAC and Push-to-Talk line, enable input capture
 */
void rx(void) {

	TIMSK2 &= ~(1<<OCIE2A); /* disable compare match interrupt */
	/* TODO do I want a delay here? */
	PORTD &= ~(1<<PD7); /* PTT OFF */

	TIMSK1 |= ((1<<ICIE1)|(1<<TOIE1)); /* enable overflow and input capture interrupts */
}

/* capture the period of an input waveform */
ISR(TIMER1_CAPT_vect) {

	unsigned int capture_period = 0;

	/* compute period taking into account overflows */
	capture_period = ICR1 + (0xffff * capture_overflows) - capture_last_time;
	capture_last_time = ICR1; /* keep track of when the last period ended */
	capture_overflows = 0; /* reset the overflow counter */

	/*
	 * capture_period is used to determine the input signal's frequnecy.
	 *
	 * With F_CPU = 14,745,600 and pre-scalar = 8, we can detemine the
	 * timer 1 frequency = 1,843,200 Hz and period 1/f = 0.000000542534722 seconds.
	 * 1200 Hz has a period of 0.000833333333 seconds. We can compute the
	 * count when that amount of time has passed by doing the following.
	 * 0.000833333333 / 0.000000542534722  = 1536 ticks. Similarly, it's
	 * 838 ticks for 2200 Hz.
	 *
	 * Since we're receiving AFSK with continuous phase, frequency changes (as
	 * observed by a zero cross detector) may appear as waveforms at some frequency
	 * between 1200 Hz and 2200 Hz. Additionally, some hardware might not be exactly
	 * at 1200 Hz and 2200 Hz, so we are very permissive. Any signals from 700 Hz to
	 * 1700 Hz are interpreted as 1200 Hz and any signals from 1700 Hz to 2700 Hz are
	 * interpreted as 2200 Hz.
	 */

	if (capture_period >= PERIOD_1200_MIN && capture_period < PERIOD_1200_MAX) {

		/* If the last frequency was 1200 Hz, then there wasn't a shift. */
		shift_detect = (carrier_sense == TONE_1200HZ) ? 0 : 1;

		/* 1200 Hz +/- 500 Hz Carrier Present */
		carrier_sense = TONE_1200HZ;

	} else if (capture_period >= PERIOD_2200_MIN && capture_period < PERIOD_2200_MAX) {

		/* If the last frequency was 2200 Hz, then there wasn't a shift. */
		shift_detect = (carrier_sense == TONE_2200HZ) ? 0 : 1;

		/* 2200 Hz +/- 500 Hz Carrier Present */
		carrier_sense = TONE_2200HZ;

	} else {

		/* No Carrier Present */
		carrier_sense = 0;
	}
}

/* keep track of timer 1 overflows */
ISR(TIMER1_OVF_vect) {

	capture_overflows++;

	/*
	 * if the timer has overflow twice, the count since the
	 * last zero crossing is at least 64k which is more than 32
	 * times the upper threshold. That means it definitely isn't
         * detecting an AFSK signal.
	 */
	if (capture_overflows > 1) {
		carrier_sense = 0;
	}
}

/* generate the output waveform */
ISR(TIMER2_COMPA_vect) {

	static unsigned char sinewave_index = 0;

	/*
	 * output sinewave to PORTA.
	 *
	 * PORTA is to be connected to an external R-2R ladder.
	 * The ladder output should be attenuated to 1Vpp and
	 * AC coupled to create line level audio.
	 */
	PORTA = sinewave[sinewave_index++];

	/*
	 * I set OCR2A here because setting it somewhere else
	 * while the timer is running could be potentially
	 * dangerous (well not that bad, but not 100% correct).
	 *
	 * For example, a switch from 1200 Hz (OCR2A=95) to
	 * 2200 Hz (OCR2A=51) when the count is between 51 and 95
	 * could lead to a skew in the output sine wave.
	 */
	OCR2A = afsk_output_frequency;

	/* keep sinewave_index in the range 0-15 */
	sinewave_index &= 0x0f;
}
