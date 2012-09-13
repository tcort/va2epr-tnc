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

#define KISS_CMD_DATA_FRAME 0x00
#define KISS_CMD_TX_DELAY 0x01
#define KISS_CMD_P 0x02
#define KISS_CMD_SLOT_TIME 0x03
#define KISS_CMD_TX_TAIL 0x04
#define KISS_CMD_FULL_DUPLEX 0x05
#define KISS_CMD_SET_HARDWARE 0x06
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
