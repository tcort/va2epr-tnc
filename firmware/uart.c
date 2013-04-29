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
 * This file implements a simple Host-to-TNC communications protocol.
 * We setup USART0 @ 56700 baud, no parity, 1 stop bit, and flow
 * control disabled.
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

#include <stdio.h>

#include "afsk.h"
#include "aprs.h"
#include "conf.h"
#include "uart.h"

/*
 * Initialize the Computer/TNC Interface (Serial via USB).
 * USART0 @ 56700 baud, no parity, 1 stop bit, flow control disabled
 */
void uart_init(void) {

	/* 56700 Baud */
	UBRR0H = (UART_UBRR_VAL >> 8);
	UBRR0L = (UART_UBRR_VAL & 0xFF);

	/* enable TX & RX as well as RX Interrupt */
	UCSR0B |= ((1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0));

	/* 8 data bits, no parity, 1 stop bit, no flow control */
	UCSR0C |= ((1<<UCSZ00)|(1<<UCSZ01));
}

/*
 * Transmit Data to the PC. Do not escape special characters.
 */
void uart_tx(unsigned char c) {

	while ((UCSR0A & (1 << UDRE0)) == 0) {

		/* Wait for buffer to clear */;
	}

	UDR0 = c;
}

/*
 * Receive Data from the PC, decode it as needed, and insert it
 * into uart_rx_buffer[] when applicable.
 */
ISR(USART0_RX_vect) {

	unsigned int i;

	/* input byte from UART */
	static unsigned char c = 0x00;

	static unsigned char cmd = 0x00;
	static unsigned char field = 0x00;
	static char value[256];
	static unsigned char vindex = 0;

	/* Read from UART */
	c = UDR0;

	switch (c)
	{
		case '{':
			cmd = 0x00;
			field = 0x00;
			for (i = 0; i < 256; i++)
				value[i] = '\0';
			vindex = 0;
			break;

		case 'G':
		case 'S':
		case 'W':
			if (cmd == 0x00) {
				cmd = c;
			} else {
				value[vindex++] = c;
			}
			break;

		case 'v':
			if (cmd == 'G') {
				snprintf(value, 256, "{v:%d}", config.version);
			} else {
				value[vindex++] = c;
			}
			if (field == 0x00) field = 'v';
			break;

		case 'd':
			if (cmd == 'G') {
				snprintf(value, 256, "{d:%d}", config.tx_delay);
			} else {
				value[vindex++] = c;
			}
			if (field == 0x00) field = 'd';
			break;

		case 'p':
			if (cmd == 'G') {
				snprintf(value, 256, "{p:%d}", config.p);
			} else {
				value[vindex++] = c;
			}
			if (field == 0x00) field = 'p';
			break;

		case 's':
			if (cmd == 'G') {
				snprintf(value, 256, "{s:%d}", config.slot_time);
			} else {
				value[vindex++] = c;
			}
			if (field == 0x00) field = 's';
			break;

		case 't':
			if (cmd == 'G') {
				snprintf(value, 256, "{t:%d}", config.tx_tail);
			} else {
				value[vindex++] = c;
			}
			if (field == 0x00) field = 't';
			break;

		case 'f':
			if (cmd == 'G') {
				snprintf(value, 256, "{f:%d}", config.full_duplex);
			} else {
				value[vindex++] = c;
			}
			if (field == 0x00) field = 'f';
			break;

		case 'c':
			if (cmd == 'G') {
				snprintf(value, 256, "{c:%s}", config.callsign);
			} else {
				value[vindex++] = c;
			}
			if (field == 0x00) field = 'c';
			break;

		case '}':
			if (cmd == 'G') {
				for (i = 0; value[i] && i < 256; i++)
					uart_tx(value[i]);
			} else if (cmd == 'S' && field != 0x00) {

				switch (field) {

					case 'v':
						config.version = value[0];
						break;
					case 'd':
						config.tx_delay = value[0];
						break;
					case 'p':
						config.p = value[0];
						break;
					case 's':
						config.slot_time = value[0];
						break;
					case 't':
						config.tx_tail = value[0];
						break;
					case 'f':
						config.full_duplex = value[0];
						break;
					case 'c':
						snprintf(config.callsign, 7, "%s", value);
						break;
				}
			} else if (cmd == 'W') {

				config_write();
			}
			break;

		case ':':
			for (i = 0; i < 256; i++)
				value[i] = '\0';
			vindex = 0;
			break;

		default:
			value[vindex++] = c;
			break;
	}

}

