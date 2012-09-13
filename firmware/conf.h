#ifndef __CONF_H
#define __CONF_H

/*
 * The version of the struct conf. This might be used in the future if I make changes to
 * struct conf and wish to support old EEPROMs.
 */
#define CONF_VERSION (0x54)

struct conf {
	unsigned char version; /* version of 'struct conf', used if I need to make changes to this struct in the future. */
	unsigned char tx_delay; /* the transmitter key-up delay in 10 ms units. */
	unsigned char p; /* the  persistence parameter,  p, scaled to the range 0 - 255 with the following formula: P = p * 256 - 1 */
	unsigned char slot_time; /* the slot interval in 10 ms units. */
	unsigned char tx_tail; /* OBSOLETE - the time to hold up the TX after the FCS has been sent, in 10 ms units. */
	unsigned char full_duplex; /* 0 for half duplex mode, non-zero for full duplex mode. */
};

extern struct conf config;

void config_read(void);
void config_write(void);

#endif
