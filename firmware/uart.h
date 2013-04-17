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

#ifndef __UART_H
#define __UART_H

/*
 * Baud Rate
 */

#define UART_BAUDRATE (57600)
#define UART_UBRR_VAL (((F_CPU / (UART_BAUDRATE * 16UL))) - 1)

/*
 * Special UART Bytes
 */

/* Frame End */
#define UART_FEND (0xc0)

/* Frame Escape */
#define UART_FESC (0xdb)

/* Transposed Frame End */
#define UART_TFEND (0xdc)

/* Transposed Frame Escape */
#define UART_TFESC (0xdd)

/*
 * UART Commands
 */

/* Data Follows */
#define UART_CMD_DATA_FRAME 0x00

/* the transmitter key-up delay in 10 ms units. */
#define UART_CMD_TX_DELAY 0x01

/* the  persistence parameter, p, scaled to the range 0 - 255 with the following formula: P = p * 256 - 1 */
#define UART_CMD_P 0x02

/* the slot interval in 10 ms units. */
#define UART_CMD_SLOT_TIME 0x03

/* OBSOLETE - the time to hold up the TX after the FCS has been sent, in 10 ms units. */
#define UART_CMD_TX_TAIL 0x04

/* 0 for half duplex mode, non-zero for full duplex mode. */
#define UART_CMD_FULL_DUPLEX 0x05

/* Set something hardware specific. See EPR Commands below. */
#define UART_CMD_SET_HARDWARE 0x06

/* return control to a higher level program. spoiler: there is none for this TNC*/
#define UART_CMD_RETURN 0xff

/*
 * EPR Commands (Commands that are specific to this TNC)
 * Used in conjunction with UART_CMD_SET_HARDWARE to implement extensions.
 * Note: custom commands that talk back to the PC will interrupt an RX in progress.
 */

/* Return the entire config struct stored in EEPROM */
#define EPR_CMD_READ_CONFIG 0x52

/*
 * Prototypes
 */

void uart_init(void);
void uart_tx(unsigned char c);

#endif
