#ifndef __KISS_H
#define __KISS_H

/*
 * Baud Rate
 */

#define KISS_BAUDRATE 9600
#define KISS_UBRR_VAL (((F_CPU / (KISS_BAUDRATE * 16UL))) - 1)

/*
 * Special KISS Bytes
 */

/* Frame End */
#define KISS_FEND (0xc0)

/* Frame Escape */
#define KISS_FESC (0xdb)

/* Transposed Frame End */
#define KISS_TFEND (0xdc)

/* Transposed Frame Escape */
#define KISS_TFESC (0xdd)

/*
 * KISS Commands
 */

/* Data Follows */
#define KISS_CMD_DATA_FRAME 0x00

/* the transmitter key-up delay in 10 ms units. */
#define KISS_CMD_TX_DELAY 0x01

/* the  persistence parameter, p, scaled to the range 0 - 255 with the following formula: P = p * 256 - 1 */
#define KISS_CMD_P 0x02

/* the slot interval in 10 ms units. */
#define KISS_CMD_SLOT_TIME 0x03

/* OBSOLETE - the time to hold up the TX after the FCS has been sent, in 10 ms units. */
#define KISS_CMD_TX_TAIL 0x04

/* 0 for half duplex mode, non-zero for full duplex mode. */
#define KISS_CMD_FULL_DUPLEX 0x05

/* Set something hardware specific. This command has no meaning for this TNC */
#define KISS_CMD_SET_HARDWARE 0x06

/* return control to a higher level program. spoiler: there is none for this TNC*/
#define KISS_CMD_RETURN 0xff

/*
 * Prototypes
 */

void kiss_init(void);

unsigned char kiss_rx_buffer_empty(void);
unsigned char kiss_rx_buffer_dequeue(void);

void kiss_tx(unsigned char c);
void kiss_tx_raw(unsigned char c);

#endif
