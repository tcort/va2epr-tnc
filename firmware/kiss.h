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
 * Prototypes
 */

void kiss_init(void);
inline unsigned char kiss_rx_buffer_empty(void);
unsigned char kiss_rx_buffer_dequeue(void);

#endif
