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
 * This file implements 1200 baud AFSK NRZI with 1200 Hz and 2200 Hz tones.
 *
 * Current Status
 * --------------
 * Can differentiate between 1200 Hz, 2200 Hz, and tones outside of
 * a given tolerance. Can output waveforms at 1200 Hz and 2200 Hz. This is
 * enough to show that the circuits used in the schematic are viable.
 *
 * Code to capture incoming data and output it via USART0 is written
 * but untested.
 *
 * The code needs to be enhanced to output data from a buffer. This can probably be
 * done by using Timer 3 to periodically sample data or periodically
 * change afsk_output_frequency to TONE_1200HZ or TONE_2200HZ. When in TX mode,
 * consider using a watchdog timer to help prevent getting stuck in TX mode.
 *
 * Ports and Peripherals Used
 * --------------------------
 *
 * Timer 0 (AFSK Decoding)
 * Timer 1 (Input Capture)
 * Timer 2 (Waveform Generation)
 *
 * PORTB (AFSK Output)
 * PD6 (Input Capture Input)
 * PD7 (Push to Talk Output)
 */

#include <avr/interrupt.h>
#include <avr/io.h>

#include "afsk.h"
#include "ax25.h"
#include "csma.h"
#include "kiss.h"

/*
 * For a 32 sample 2200 Hz tone, I need to change the output 70400
 * (i.e. 2200 * 32) times per second. For a 32 sample 1200 Hz tone,
 * I need to change the output 38400 (i.e. 1200 * 32) times per second.
 * Using a pre-scalar of 4, I should set TCNT2 to the following values
 * via the 'tone' variable:
 *
 *   For 1200 Hz, 14745600/8/38400 - 1 = 47				Exactly 1200 Hz
 *   For 2200 Hz, 14745600/8/70400 - 1 = 25.18 (25)			Approx  2215 Hz (0.6% error)
 */
#define TONE_1200HZ 47
#define TONE_2200HZ 25

/*
 * These are the period thresholds that are used to decide if a
 * captured input is 1200 Hz or 2200 Hz. You can compute these
 * the same as above...
 *
 *  For  200 Hz, 14745600/8/ 200 - 1 = 9216
 *  For 1700 Hz, 14745600/8/1700 - 1 = 1083
 *  For 4700 Hz, 14745600/8/4700 - 1 = 391
 */
#define PERIOD_2200_MIN  391 /* 2.7 kHz */
#define PERIOD_2200_MAX 1083 /* 1.7 kHz */
#define PERIOD_1200_MIN 1083 /* 1.7 kHz */
#define PERIOD_1200_MAX 9216 /* 0.2 kHz */

/*
 * This is the counter value for the AFSK decoding interrupt
 * running on Timer 0.
 *
 * 14745600/8/9600 - 1 = 191
 */
#define TIMER_9600 191 /* 9600 Hz */

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

/*
 * carrier sesne timeout -- used by RX to identify when a carrier is
 * no longer present. It counts down from a predefined value to 0. When
 * it becomes 0, the carrier is no longer present.
 */
unsigned char carrier_sense_timeout = 0;

/* precomputed sinewave from tools/sine.c */
unsigned char sinewave[32] = {
	254,  25,  13,  99, 155, 151, 175,  63, 
	255,  63, 175, 151, 155,  99,  13,  25, 
	254, 102, 114,  28, 164, 168, 144,  64, 
	  0,  64, 144, 168, 164,  28, 114, 102
};

/*
 * House keeping for the RX code. This flag used to
 * indicate that a start-of-frame KISS_FEND has been
 * sent via USART0, I am sending an AX25 frame, and
 * a KISS_FEND will be needed to complete the frame.
 */
unsigned char in_kiss_frame = 0;

/* setup ports and timers */
void afsk_init(void) {

	/* -- Output DAC (TX) -- */
	/* do all setup except enabling the compare match interrupt (done in tx()) */

	/* AFSK DAC is on PORTB */
	DDRB |= 0xFF; /* AFSK output */
	PORTB = 0x00; /* initialize */

	/* Timer 2 CTC, pre-scalar 8 */
	TCCR2A |= (1<<WGM21); /* CTC */
	TCCR2B |= (1<<CS21); /* pre-scalar = 8 */
	TCNT2 = 0x00; /* initialize counter to 0 */
	OCR2A = afsk_output_frequency; /* set with some default value */

	/* -- Input Capture (RX) -- */
	/* do all setup except enabling of input capture interrupt (done in rx()) */

	/* Input Capture on PD6 */
	DDRD &= ~(1<<PD6); /* ICP as Input */
	PORTD |= (1<<PD6); /* enable pull-up on ICP1 */

	/* Timer 1 ICP */
	TCCR1A = 0x00; /* Normal Mode */
	TCCR1B |= ((1<<ICNC1)|(1<<ICES1)|(1<<CS11)); /* noise canceler, rising edge, pre-scalar = 8 */

	/* -- AFSK Decoding -- */
	/* do all setup except enabling of afsk decoding interrupts (done in rx()) */

	/* Timer 0 CTC, pre-scalar 8 */
	TCCR0A |= (1<<WGM01); /* CTC */
	TCCR0B |= (1<<CS01); /* pre-scalar = 8 */
	TCNT0 = 0x00; /* initialize counter to 0 */
	OCR0A = TIMER_9600; /* 9600 times per second */

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

	/* Turn off RX interrupts */
	TIMSK0 &= ~(1<<OCIE0A); /* disable afsk decoder */
	TIMSK1 &= ~(1<<ICIE1); /* disable input capture interrupt */

	/*
	 * If it goes into TX without finishing receiving the current AX.25 frame,
	 * then it must be cleaned up and an ending KISS_FEND must be sent. This
         * situation could happen (maybe) if the sending station drops out or
	 * something else goes wrong.
	 */
	if (in_kiss_frame) {
		/* End the current KISS frame. */
		kiss_tx(AX25_FLAG);
		kiss_tx_raw(KISS_FEND);
		in_kiss_frame = 0;
	}

	/* Turn on TX interrupts/pins */
	PORTD |= (1<<PD7); /* PTT ON */
	TIMSK2 |= (1<<OCIE2A); /* enable compare match interrupt */
}

/*
 * Disable output DAC and Push-to-Talk line, enable input capture
 */
void rx(void) {

	/* Turn off TX interrupts/pins */
	TIMSK2 &= ~(1<<OCIE2A); /* disable compare match interrupt */
	PORTD &= ~(1<<PD7); /* PTT OFF */

	/* Turn on RX interrupts */
	TIMSK1 |= (1<<ICIE1); /* enable input capture interrupt */
	TIMSK0 |= (1<<OCIE0A); /* enable afsk decoder */
}

/*
 * The algorithm used to receive and decode the AFSK NRZI encoded
 * data is not something I came up with. Several projects make use
 * of it. I'm having a hard time tracking down the originator to give
 * him or her the proper credit. I first read about the algorithm on
 * Gary N. Dion's WhereAVR project page: http://garydion.com/projects/whereavr/
 * Gary links to Mike Berg's Zero Cross Detector decoder project page
 * http://www.ringolake.com/pic_proj/zcd/zcdmodem.html Mike's site
 * links to Bob Ball's QEX Article "WB8WGA's Simple, inexpensive ($25) TNC"
 * http://www.n9cx.net/BobBall.htm
 *
 * Author's note: none of the code in this project is directly copied
 * from any of the above sources, but I did implement it using their
 * descriptions of the zero cross detector decoding algorithm with
 * some changes to suit this project's needs and hardware.
 */

/*
 * decode incoming AFSK data, forward it to the computer
 * over USART0 using the KISS protocol.
 *
 * There are some design decisions made which impact this code.
 * I could write the bytes to a buffer and CRC the frames before
 * sending them to the computer. Instead I chose to forward each
 * byte as it comes in. This saves CPU time and memory on the
 * microcontroller at the expense of forwarding some possibly
 * invalid frames to the computer (which should do it's own CRC
 * checks on the incoming frames anyway).
 */
ISR(TIMER0_COMPA_vect) {

	/* receive buffer holding the last 8 received bits */
	static unsigned char bits = 0x00;

	/* counter used to track the number of bits in 'bits' */
	static unsigned char bits_count = 0;

	/* count the 1's received - used for AX.25 bit stuffing */
	static unsigned char ones_count = 0;

	/* pseudo-timer used to decide when to sample */
	static unsigned char sample_timer = 1;

	/* Check if the timeout timer is active */
	if (carrier_sense_timeout) {

		carrier_sense_timeout--; /* decrement */

		/* check if we've reached the timeout */
		if (carrier_sense_timeout == 0) {

			/* No Carrier Present */
			carrier_sense = 0;
		}
	}

	/* is a signal being received? */
	if (carrier_sense) {

		/*
		 * synchronize the sample_timer on shift_detect; sample when the time comes.
		 */
		if (shift_detect) {

			carrier_sense_timeout = 96; /* reset timeout */

			/*
			 * Check for AX.25 bit stuffing. If stuffing, don't sample the '0'.
			 *
			 * From the AX.25 spec (v2.2):
			 *
			 *	"any time five contiguous “1” bits are received, a “0” bit
			 *	immediately following five “1” bits is discarded."
			 */
			if (ones_count != 5) {

				/*
				 * frequency shift decodes to '0'
				 *
				 * I can't find a document that explains NZRI anywhere
				 * (as it pertains to amateur radio), but I saw it in a comment
				 * in the BertOS source code, so I assume it's true :)
				 */
				bits >>= 1; /* insert a 0 from the left */
				bits_count++;
			}

			/* clear shift_detect so we don't re-sample the same bit */
			shift_detect = 0;

			/*
			 * Set the pseudo-timer to sample the next bit.
			 *
			 * If we are here, then the signal is at the leading edge of an
			 * AFSK clock (since a shift was detected and this interrupt is
			 * running 8 times faster than the AFSK baud rate of 1200).
			 *
			 * I want to make the next sample 1/1200 of a second from now,
			 * but I want some wiggle room. For example, I don't want to
			 * beat the AFSK clock to the next bit. Therefore, I will check
			 * the state half way through the next AFSK clock cycle.
			 *
			 * 9600/1200 == 8, 8*1.5 == 12
			 */
			sample_timer = 12;

			ones_count = 0; /* clear consecutive 1's counter */

		} else {

			/* when the time is right, sample! */
			if (--sample_timer == 0) {

				/* no frequency shift decodes to '1' */
				bits = ((0x80) | (bits >> 1)); /* insert a 1 from the left */
				bits_count++;

				/* 9600/1200 == 8 */
				sample_timer = 8;

				ones_count++; /* keep a tally of consecutive 1's received */
			}
		}

		/*
		 * The only time there is supposed to be six '1' bits in a row is the
		 * AX.25 flag (0b01111110). I use this property to synchronize the
		 * 'bits_count' variable. That variable is used to know when to sample
		 * the byte held in 'bits'.
		 *
		 * If the flag is found or the byte buffer is full, then send the byte
		 * and reset the bits_count to 0.
		 */
		if ((bits == AX25_FLAG) || (bits_count >= 8 && in_kiss_frame == 1)) {


			/*
			 * When AX25_FLAG is received, it begins and/or ends an
			 * AX.25 frame. KISS encapsulates AX.25 frames, so I need
			 * to send an unescaped KISS_FEND byte with kiss_tx_raw()
			 */
			if (bits == AX25_FLAG) {

				if (in_kiss_frame) {
					/*
					 * End the current KISS frame. I also send 'bits'
					 * (the AX25_FLAG) before KISS_FEND to ensure that
					 * the encapsulated frame ends with an AX25_FLAG. I
					 * do this because there isn't a 1 to 1 relationship
					 * between KISS_FENDS and AX25_FLAG. At best, this will
					 * comply with the spec. At worse, it will send empty
					 * AX.25 frames to the computer.
					 */
					kiss_tx(bits);
					kiss_tx_raw(KISS_FEND);
					sample_timer -= 2;
					in_kiss_frame = 0;
				}

				/* start the next frame */
				kiss_tx_raw(KISS_FEND);
				sample_timer--;
				in_kiss_frame = 1;
			}

			/*
			 * Send the byte over USART0. Use kiss_tx() to ensure that
			 * special characters are escaped properly.
			 *
			 * If more than 1 character is sent, the sample_time is
			 * adjusted accordingly. This is because the USART transmit
			 * is running at 9600 baud. If it has to wait to send a 2nd byte,
			 * then that wait time needs to be subtracted from the sample_time.
			 */
			sample_timer -= (kiss_tx(bits) - 1);

			/* clear bit counter */
			bits_count = 0;
		}

	} else {

		if (in_kiss_frame) {
			/* End the current KISS frame. */
			kiss_tx(AX25_FLAG);
			kiss_tx_raw(KISS_FEND);
			in_kiss_frame = 0;
		}

		/* Reset Internal State */
		bits = 0x00;
		bits_count = 0;
		ones_count = 0;
		sample_timer = 1;
	}
}

/* capture the period of an input waveform (rising edge triggered) */
ISR(TIMER1_CAPT_vect) {

	static unsigned int capture_period;

	/*
	 * Get the clock count since the last rising edge (i.e. the period of
	 * the waveform).
	 *
	 * To make this as simple as possible, so that it executes quickly,
	 * I have done away with the house keeping for counting overflows
	 * and computing the time since the last rising edge. Instead, the
	 * CPU just counts from zero. When control gets to this part of the
	 * code, the period is just ICR1.
	 */
	capture_period = ICR1;

	/*
	 * reset counter so that this function doesn't have to keep track of
	 * the prior ICR1 value. (e.x. capture_period = ICR1 - last_ICR1_value;
	 */
	TCNT1 = 0;

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

		/* 1200 Hz Carrier Present */
		carrier_sense = TONE_1200HZ;

	} else if (capture_period >= PERIOD_2200_MIN && capture_period < PERIOD_2200_MAX) {

		/* If the last frequency was 2200 Hz, then there wasn't a shift. */
		shift_detect = (carrier_sense == TONE_2200HZ) ? 0 : 1;

		/* 2200 Hz Carrier Present */
		carrier_sense = TONE_2200HZ;
	}
}

/* generate the output waveform */
ISR(TIMER2_COMPA_vect) {

	static unsigned char sinewave_index = 0;

	/*
	 * output sinewave to PORTB.
	 *
	 * PORTB is to be connected to an external R-2R ladder.
	 * The ladder output should be attenuated to 1Vpp and
	 * AC coupled to create line level audio.
	 */
	PORTB = sinewave[sinewave_index++];

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

	/* keep sinewave_index in the range 0-31 */
	sinewave_index &= 0x1f;
}
