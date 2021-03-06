/*
 * va2epr-tnc firmware - terminal node controller firmware for va2epr-tnc
 * Copyright (C) 2012, 2013 Thomas Cort <linuxgeek@gmail.com>
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
 * This file manages the configuration parameters stored in EEPROM.
 * The following configuration parameters stored are: TX Delay, p,
 * Slot Time, TX Tail, Full Duplex, and callsign. The device only supports
 * half duplex, so any value for Full Duplex is ignored. The version
 * parameter is reserved for future use (i.e. if I change the struct conf
 * in a future version, the software has to be able to know what fields
 * are present in EEPROM.
 */

#include <avr/io.h>
#include <avr/eeprom.h>

#include "conf.h"

/*
 * Global configuration (stored in RAM). This struct is filled in
 * during the initialization using config_read(). This helps reduce,
 * in a very minor way, the wear and tear on the EEPROM. When the
 * PC sends a configuration change followed by the write command,
 * this struct is written to EEPROM with config_write().
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
	0, /* full duplex */
	"VA2EPR"
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

	eeprom_write_block(&config, &config_in_eeprom, sizeof(struct conf));
}
