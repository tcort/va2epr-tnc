#ifndef __GPS_H
#define __GPS_H

/*
 * Baud Rate
 */

#define GPS_BAUDRATE 4800
#define GPS_UBRR_VAL (((F_CPU / (GPS_BAUDRATE * 16UL))) - 1)

/*
 * Prototypes
 */

void gps_init(void);

#endif
