/*
 * The GPS module being used in this project outputs data according to
 * NMEA0183 V2.2. It supports the following sentences:
 *
 *   GGA ~ Global Positioning System Fix Data. Time, Position and fix related data for a GPS receiver
 *   GSV ~ Satellites in view
 *   GSA ~ GPS DOP and active satellites
 *   RMC ~ Recommended Minimum Navigation Information
 *   VTG ~ Track Made Good and Ground Speed (optional)
 *   GLL ~ Geographic Position – Latitude/Longitude (optional)
 *
 * From what I've read, the GPS module continuously outputs
 * GPGGA, GPGSA, GPGSV, and GPRMC sentences in NMEA mode. From what I observed,
 * not all sentences appear all of the time. For example, GPGSV isn't sent
 * when no satellites are in view.
 *
 *
 * When I plugged it in for the first time I got this about every second...
 *
 *   $GPGGA,000425.035,0000.0000,N,00000.0000,E,0,00,,0.0,M,0.0,M,,0000*46
 *   $GPGSA,A,1,,,,,,,,,,,,,,,*1E
 *   $GPGSV,1,1,00*79
 *   $GPRMC,000425.035,V,0000.0000,N,00000.0000,E,,,150209,,,N*7C
 *
 * When no satellites were in view, there were no $GPGSV sentences.
 *
 * Coordinates use the WGS 84 DATUM
 *
 * Ports and Peripherals Used
 * --------------------------
 *
 * USART1 (Connected to the GPS)
 *
 * PD2 (RXD)
 * PD3 (TXD)
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "kiss.h"
#include "gps.h"
#include "nmea.h"

/*
 * Initialize the GPS interface
 */
void gps_init(void) {

	/* 4800 baud */
	UBRR1H = (GPS_UBRR_VAL >> 8);
	UBRR1L = (GPS_UBRR_VAL & 0xFF);

	UCSR1B |= (1 << RXEN1) | (1 << TXEN1) | (1<<RXCIE0);
	UCSR1C |= (1 << UCSZ10) | (1 << UCSZ11);
}

/*
 * Receive Data from the GPS
 * For testing/debugging, just output it to the PC
 */
ISR(USART1_RX_vect) {

	static unsigned char c = 0x00;

	/* Read from UART */
	c = UDR1;

	kiss_tx_raw(c);
}
