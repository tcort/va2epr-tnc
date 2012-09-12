/*
 * This file implements the KISS protocol, a simple Host-to-TNC
 * communications protocol.
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

#include "kiss.h"

/*
 * Buffer incoming KISS frames for transmission via AFSK.
 * Why? We aren't guaranteed use of the AFSK channel immediately
 * (for example, someone else is transmitting a long message),
 * so data from the PC needs to be buffered. Even if we always
 * have a clear channel there is the additional problem of
 * the AFSK baud rate being 1200 and the UART buad rate being
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
 */
void kiss_init(void) {

	UBRR0H = (KISS_UBRR_VAL >> 8);
	UBRR0L = (KISS_UBRR_VAL & 0xFF);

	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
	UCSR0C |= (1<<UCSZ00)|(1<<UCSZ01);
}

/*
 * Check if the buffer is empty or not by comparing
 * head and tail.
 */
inline unsigned char kiss_rx_buffer_empty(void) {

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
	 * we want to prevent kiss_rx_buffer_head from passing
	 * kiss_rx_buffer_tail.
	 */
	if (kiss_rx_buffer_empty()) {

		/*
		 * Return the AX.25 frame delimiter character. Any number of these
		 * can be sent between frames. If we're out of data in the buffer,
		 * we are likely between AX.25 frames.
		 */
		return 0x7e;
	}

	/* Dequeue a byte from the circular buffer */
	c = kiss_rx_buffer[kiss_rx_buffer_head++];

	/* Keep kiss_tx_buffer_tail in the range 0-8191 */
	kiss_rx_buffer_head &= 0x1fff;

	return c;
}

/*
 * Receive Data from the PC and insert it
 * into kiss_rx_buffer[].
 */
ISR(USART0_RX_vect) {

	/* TODO - KISS decoding before insert */

	/* Insert byte into circular buffer */
	kiss_rx_buffer[kiss_rx_buffer_tail++] = UDR0;

	/* TODO - should I check if buffer is full? If it is full, what can we do about it? */

	/* Keep kiss_tx_buffer_tail in the range 0-8191 */
	kiss_rx_buffer_tail &= 0x1fff;
}
