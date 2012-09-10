/*
 * Implements the KISS protocol, a simple Host-to-TNC communications protocol
 */

#include "kiss.h"

/*
 * Transmit Buffer
 *
 * The host PC will be sending us data at 9600 baud via USART, but we can
 * only send AFSK encoded data at 1200 baud. Additionally, the computer could
 * send us data while another station is transmitting. Therefore, we need
 * to buffer outgoing data. The goal is to flush this buffer (i.e. empty
 * it by sending data to the radio every time there is a complete frame
 * in it AND there are no other stations sending.
 */
unsigned char tx_buffer[TX_BUFFER_SIZE];

/*
 * Receive Buffer
 *
 * Buffering incoming data isn't as crucial, but it allows us to do some
 * checksumming if we want and only forward 'good' frames to the host PC.
 * The goal is to flush this buffer (i.e. empty it by sending data to the
 * host PC) every time there is a full frame in it.
 */
unsigned char rx_buffer[RX_BUFFER_SIZE];

void kiss_init(void) {

	/* TODO setup USART0 */
	/* detect if computer is connected */
}

/*
 * TODO implement it!
 */
