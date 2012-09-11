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
 * GGA, GSV, GSA, and RMC sentences in NMEA mode.
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

#include "gps.h"

void gps_init(void) {

	/* TODO setup USART1 */
	/* detect if GPS module is installed */
}

/*
 * Things to be determined...
 *
 * Do we have enough processing power to continuously parse
 * the gps module's output? If so, we can keep track of
 * position information in real time and have the data
 * available immediately when it is needed for APRS beacons.
 * Additionally, continuous parsing could be used to create
 * an alarm system (at a specific time, do something).
 *
 * What data is needed for APRS? i.e. can some sentences be
 * completely ignored.
 */
