/*
 * This file implements the KISS protocol, a simple Host-to-TNC
 * communications protocol. We setup USART0 @ 9600 baud, no parity,
 * 1 stop bit, and flow control disabled.
 *
 * Ports and Peripherals Used
 * --------------------------
 *
 * USART0 (Connected to the PC)
 *
 * PD0 (TXD)
 * PD1 (RXD)
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "afsk.h"
#include "ax25.h"
#include "conf.h"
#include "kiss.h"

/*
 * Buffer incoming KISS frames for transmission via AFSK.
 * Why? I'm not guaranteed use of the AFSK channel immediately
 * (for example, someone else is transmitting a long message),
 * so data from the PC needs to be buffered. Even if I always
 * have a clear channel there is the additional problem of
 * the AFSK baud rate being 1200 and the UART baud rate being
 * 9600.
 */
static unsigned char kiss_rx_buffer[8192];

/*
 * Index of the beginning of data in kiss_rx_buffer[].
 * If it equals kiss_rx_buffer_tail, then the buffer
 * is empty. Only kiss_rx_buffer_dequeue() should modify this.
 */
static unsigned int kiss_rx_buffer_head = 0;

 /*
  * Index of the end of data in kiss_rx_buffer[].
  * If it equals kiss_rx_buffer_head, then the buffer
  * is empty. Only USART0_RX_vect should modify this.
  */
static unsigned int kiss_rx_buffer_tail = 0;

/*
 * Initialize the Computer/TNC Interface (Serial via USB).
 * USART0 @ 9600 baud, no parity, 1 stop bit, flow control disabled
 */
void kiss_init(void) {

	/* 9600 Baud */
	UBRR0H = (KISS_UBRR_VAL >> 8);
	UBRR0L = (KISS_UBRR_VAL & 0xFF);

	/* enable TX & RX as well as RX Interrupt */
	UCSR0B |= ((1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0));

	/* 8 data bits, no parity, 1 stop bit, no flow control */
	UCSR0C |= ((1<<UCSZ00)|(1<<UCSZ01));
}

/*
 * Check if the buffer is empty or not by comparing
 * head and tail.
 */
unsigned char kiss_rx_buffer_empty(void) {

	return (kiss_rx_buffer_head == kiss_rx_buffer_tail);
}

/*
 * Dequeue a byte from kiss_rx_buffer[]. This is used by
 * the AFSK transmit code to get data to send.
 */
unsigned char kiss_rx_buffer_dequeue(void) {

	static unsigned char c;

	/*
	 * The caller should have called kiss_rx_buffer_empty().
	 * This isn't error handling as much as it is self preservation;
	 * I want to prevent kiss_rx_buffer_head from passing
	 * kiss_rx_buffer_tail.
	 */
	if (kiss_rx_buffer_empty()) {

		/*
		 * Return the AX.25 frame delimiter character. Any number of these
		 * can be sent between frames. If I'm out of data in the buffer,
		 * I am likely between AX.25 frames as well.
		 */
		return AX25_FLAG;
	}

	/* Dequeue a byte from the circular buffer */
	c = kiss_rx_buffer[kiss_rx_buffer_head++];

	/* Keep kiss_tx_buffer_tail in the range 0-8191 */
	kiss_rx_buffer_head &= 0x1fff;

	return c;
}

/*
 * Transmit Data to the PC. Do not escape special KISS characters.
 * Use this function to send KISS_FEND. Use kiss_tx() for everything else.
 */
void kiss_tx_raw(unsigned char c) {

	while ((UCSR0A & (1 << UDRE0)) == 0) {
		/* wait for send buffer to be clear */;
	}

	UDR0 = c;
}

/*
 * Transmit Data to the PC. Escape special KISS characters.
 * Use this function in afsk.c for everything except sending
 * KISS_FEND. Return the number of bytes sent so that the
 * calling code can estimate how long it took the function to
 * execute.
 */
unsigned char kiss_tx(unsigned char c) {

	unsigned char bytes_sent = 1;

	/* check for special characters */
	switch (c) {

		/* frame end */
		case KISS_FEND:
			/* send the escape character followed by a transposed frame end character */
			kiss_tx_raw(KISS_FESC);
			c = KISS_TFEND;
			bytes_sent++;
			break;

		/* frame escape */
		case KISS_FESC:
			/* send the escape character followed by a transposed frame end character */
			kiss_tx_raw(KISS_FESC);
			c = KISS_TFESC;
			bytes_sent++;
			break;
	}

	kiss_tx_raw(c);

	return bytes_sent;
}

/*
 * Receive Data from the PC, decode it as needed, and insert it
 * into kiss_rx_buffer[] when applicable.
 */
ISR(USART0_RX_vect) {

	static unsigned char c = 0x00;

	/* persistent flags */
	static unsigned char cmd_follows = 0;
	static unsigned char val_follows = 0;
	static unsigned char esc_follows = 0;

	/* Read from UART */
	c = UDR0;

	/* check for special characters */
	switch (c) {

		/* frame end */
		case KISS_FEND:
			/* Frame Delimiter Found, next byte is either a frame delimiter or command */
			cmd_follows = 1;
			return;

		/* frame escape */
		case KISS_FESC:
			/* next byte will be and escaped character (either KISS_TFEND or KISS_TFESC) */
			esc_follows = 1;
			return;
	}

	if (esc_follows) {

		/* 'c' was preceded by an escape character (KISS_FESC) */
		switch (c) {

			/* transposed frame end */
			case KISS_TFEND:
				c = KISS_FEND;
				break;

			/* transposed frame escape */
			case KISS_TFESC:
				c = KISS_FESC;
				break;
		}

		esc_follows = 0;
	}

	/*
	 * if the last byte was a frame delimiter and this byte is not a frame delimiter,
	 * then it is a command code
	 */
	if (cmd_follows) {

		/*
		 * The first nibble is for the port number. I only have one port (port 0),
		 * so I assume all frames are meant for port 0. That's why I am and-ing with 0x0f.
		 * Data Frame is the only command without parameters and the only command
		 * with a zero lower nibble. In other words, (command & 0x0f) will be 1 when
		 * the next byte is a parameter.
		 */
		val_follows = (c & 0x0f);

		/* clear this flag */
		cmd_follows = 0;

		/*
		 * micro-optimization - this saves 1 jmp instruction
		 * (jumping to the end of the if/else block).
		 */
		return;

	} else if (val_follows) {

		/*
		 * Set the value in global config. All values are unsigned integers, so I
		 * store them directly. Pretty much every value could be valid, so I don't
		 * do any input validation.
		 */
		switch (val_follows) {

			case KISS_CMD_TX_DELAY:
				config.tx_delay = c;

			case KISS_CMD_P:
				config.p = c;

			case KISS_CMD_SLOT_TIME:
				config.slot_time = c;

			case KISS_CMD_TX_TAIL:
				/*
				 * This is an obsolete option. The value isn't used in this TNC.
				 */
				config.tx_tail = c;

			case KISS_CMD_FULL_DUPLEX:
				/*
				 * I implement this configuration option but ignore the value.
				 * The TNC only supports half duplex.
				 */
				config.full_duplex = c;

			case KISS_CMD_SET_HARDWARE:
				/* this command has no meaning for this TNC */
				break;

			case KISS_CMD_RETURN:
				/* only KISS mode is supported right now. ignore this command. */
				break;

			default:
				/* ignore invalid commands */
				break;
		}

		config_write();

		/* get the parameter from 'c' and put it in the right configuration variable */

		val_follows = 0;

		/*
		 * micro-optimization - this saves 1 jmp instruction
		 * (jumping to the end of the if/else block).
		 */
		return;

	} else {	/* If a character isn't a command nor parameter, add it to the buffer */

		/* Insert byte into circular buffer */
		kiss_rx_buffer[kiss_rx_buffer_tail++] = c;

		/* Keep kiss_tx_buffer_tail in the range 0-8191 */
		kiss_rx_buffer_tail &= 0x1fff;

		/*
		 * The tough part happens when the buffer gets full. Currently, I
		 * don't check for buffer overflows. Only in the most extreme use
		 * cases will the buffer become full. The default behavior when the
		 * buffer becomes full will essentially clear the buffer as tail
		 * becomes equal to head. This will either stop transmitting which
		 * is polling the buffer size, or if data is coming in from the PC,
		 * it will keep sending but it will have dropped 8k in the middle.
		 *
		 * The prescibed implementation is mentioned in the KISS specification.
		 * Accordingly, I should drop the last frame (i.e. the one busting the buffer).
		 * However, this will require a re-factoring of the planned transmit design which
		 * currently starts sending as soon as the first byte is added to kiss_rx_buffer[].
		 *
		 * TODO: implement kiss_rx_buffer[] bounds checking with proper frame drop.
		 * I'll need to do additional house keeping -- keeping track of how many frames
		 * are in the buffer (possibly where they start/end) as well as if I should discard
		 * the current frame coming in over the USART interface. The code polling
		 * the the buffer size should instead poll the number of complete frames in the buffer.
		 */

		/*
		 * micro-optimization - this saves 1 jmp instruction
		 * (jumping to the end of the if/else block).
		 */
		return;
	}
}
