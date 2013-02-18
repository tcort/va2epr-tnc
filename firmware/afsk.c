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
 * This file implements 1200 baud AFSK NRZI with 1200 Hz and 2200 Hz tones.
 *
 * Ports and Peripherals Used
 * --------------------------
 *
 * Timer 0 (AFSK Decoding)
 * Timer 1 (Input Capture)
 * Timer 2 (Waveform Generation)
 * Timer 3 (AFSK Encoding)
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
 *  For  200 Hz, 14745600/8/ 800 - 1 = 2303
 *  For 1700 Hz, 14745600/8/1700 - 1 = 1083
 *  For 4700 Hz, 14745600/8/2600 - 1 = 708
 */
#define PERIOD_2200_MIN  708 /* 2.6 kHz */
#define PERIOD_2200_MAX 1083 /* 1.7 kHz */
#define PERIOD_1200_MIN 1083 /* 1.7 kHz */
#define PERIOD_1200_MAX 2303 /* 0.8 kHz */

/*
 * This is the counter value for the AFSK decoding interrupt
 * running on Timer 0.
 *
 * 14745600/8/9600 - 1 = 191
 */
#define TIMER_9600 191 /* 9600 Hz */

/*
 * This is the counter value for the AFSK encoding interrupt
 * running on Timer 3.
 *
 * 14745600/8/1200 - 1 = 1535
 */
#define TIMER_1200 1535 /* 1200 Hz */

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

	/* Timer 3 CTC, pre-scalar 8 */
	TCCR3A |= (1<<WGM31); /* CTC */
	TCCR3B |= (1<<CS31); /* pre-scalar = 8 */
	TCNT3 = 0x00; /* initialize counter to 0 */
	OCR3A = TIMER_1200; /* 1200 times per second */

	/* -- Input Capture (RX) -- */
	/* do all setup except enabling of input capture interrupt (done in rx()) */

	/* Input Capture on PD6 */
	DDRD &= ~(1<<PD6); /* ICP as Input */
	PORTD |= (1<<PD6); /* enable pull-up on ICP1 */
	ACSR &= ~(1<<ACIC); /* select ICP1 input for source for input capture (default) */

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
	TIMSK3 |= (1<<OCIE3A); /* enable afsk encoder */
}

/*
 * This is used when a GPS is connected to disable RX when not transmitting.
 */
void notxrx(void) {

	/* Turn off RX interrupts */
	TIMSK0 &= ~(1<<OCIE0A); /* disable afsk decoder */
	TIMSK1 &= ~(1<<ICIE1); /* disable input capture interrupt */

	/* Turn off TX interrupts/pins */
	TIMSK2 &= ~(1<<OCIE2A); /* disable compare match interrupt */
	TIMSK3 &= ~(1<<OCIE3A); /* disable afsk encoder */
	PORTD &= ~(1<<PD7); /* PTT OFF */
}

/*
 * Disable output DAC and Push-to-Talk line, enable input capture
 */
void rx(void) {

	/* Turn off TX interrupts/pins */
	TIMSK2 &= ~(1<<OCIE2A); /* disable compare match interrupt */
	TIMSK3 &= ~(1<<OCIE3A); /* disable afsk encoder */
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
	static signed char next_sample = 8;

	/* track bytes sent by kiss_tx so that next_sample can be adjusted properly */
	static unsigned char extra_bytes_sent = 0;

	if (shift_detect) {

		/* clear shift_detect so we don't re-sample the same bit */
		shift_detect = 0;

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
		next_sample = 12;

		ones_count = 0; /* clear consecutive 1's counter */

	} else if (--next_sample <= 0) {

		/* no frequency shift decodes to '1' */
		bits = ((0x80) | (bits >> 1)); /* insert a 1 from the left */
		bits_count++;

		/* 9600/1200 == 8 */
		next_sample = 8;

		ones_count++; /* keep a tally of consecutive 1's received */
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
	if (bits == AX25_FLAG || bits_count >= 8) {

		extra_bytes_sent = kiss_tx(bits) - 1;

		/* We additional escape characters sent? */
		if (extra_bytes_sent) {

			/* Each additional byte sent delays this ISR (43us @ 230400 baud). */
			next_sample -= (unsigned char) ((1.0/9600) / ((extra_bytes_sent*1.0*(8+1+1)) / KISS_BAUDRATE));
		}

		/* clear bit counter */
		bits_count = 0;
	}


	/* TODO test if this code works, then re-add KISS framing bytes */

}

/* capture the period of an input waveform (rising edge triggered) */
ISR(TIMER1_CAPT_vect) {

	static unsigned int low; /* low bits of capture period */
	static unsigned int high; /* high bits of capture period */
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
	low = ICR1L;
	high = ICR1H;
	capture_period = ((high << 8) | low);

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
		shift_detect |= (carrier_sense != TONE_1200HZ);

		/* 1200 Hz Carrier Present */
		carrier_sense = TONE_1200HZ;

	} else if (capture_period >= PERIOD_2200_MIN && capture_period < PERIOD_2200_MAX) {

		/* If the last frequency was 2200 Hz, then there wasn't a shift. */
		shift_detect |= (carrier_sense != TONE_2200HZ);

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

/* vary afsk_output_frequency to encode data */
ISR(TIMER3_COMPA_vect) {

	/* byte to send (LSB first) */
	static unsigned char bits = 0;

	/* number of bits in 'bits' sent */
	static unsigned char bit_count = 0;

	/* count the 1's sent - used for AX.25 bit stuffing */
	static unsigned char ones_count = 0;

	/* Does 'bits' contain the AX.25 Flag? Used to avoid bit stuffing in that case. */
	static unsigned char sending_ax25_flag = 0;

	/* list of possible tones */
	static unsigned char tones[2] = {
		TONE_1200HZ, TONE_2200HZ
	};

	/* index in the tones array */
	static unsigned char tones_index = 0;

	/* Is the buffer 'bits' empty? If so, get more bits */
	if (bit_count == 0) {

		/* Do we have any bytes that need to be sent? */
		if (kiss_rx_buffer_empty()) {

			/* The buffer is empty. We can stop sending now.
			 * The loop in main() will detect an empty buffer
			 * and disable this interrupt for us.
			 */
			return;
		}

		bits = kiss_rx_buffer_dequeue();
		sending_ax25_flag = (bits == AX25_FLAG);
	}

	if (!(bits & 0x01)) { /* is current bit 0? */

		/* if the current bit is a 0, then toggle */
		tones_index = !tones_index;
		afsk_output_frequency = tones[tones_index];
		ones_count = 0;

	} else if (ones_count++ >= 5 && !sending_ax25_flag) {

		/* bit stuff */
		tones_index = !tones_index;
		afsk_output_frequency = tones[tones_index];
		ones_count = 0;

		bits <<= 1;
		bit_count = (bit_count-1) & 0x07;
	}

	bits >>= 1;
	bit_count = (bit_count+1) & 0x07;

}
