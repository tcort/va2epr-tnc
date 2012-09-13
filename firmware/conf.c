/*
 * This file manages the configuration parameters stored in EEPROM.
 * The following configuration parameters stored are: TX Delay, p,
 * Slot Time, TX Tail, and Full Duplex. TX Tail is obsolete according
 * to the kiss standard, so it is ignored. The device only supports
 * half duplex, so any value for Full Duplex is ignored. Set Hardware
 * is ignored as it has no meaning in this TNC.
 */

#include <avr/io.h>
#include <avr/eeprom.h>

#include "conf.h"

/*
 * Global configuration (stored in RAM).
 */
struct conf config;

/*
 * Global configuration (stored in EEPROM).
 *
 * The values below are written once when the device is programmed. 
 * The defaults are from the KISS specification.
 */
struct conf EEMEM config_in_eeprom = {
	CONF_VERSION, /* version */
	50, /* tx_delay */
	63, /* p */
	10, /* slot time */
	50, /* tx_tail */
	0 /* full duplex */
};

/*
 * Read the configuration values from EEPROM and store them in config global variable
 */
void config_read(void) {

	eeprom_read_block(&config, &config_in_eeprom, sizeof(struct conf));
}

/*
 * Write the configuration values from config global variable into EEPROM
 */
void config_write(void) {

	eeprom_write_block(&config_in_eeprom, &config, sizeof(struct conf));
}
