#ifndef __CONF_H
#define __CONF_H

struct conf {
	unsigned char tx_delay;
	unsigned char p;
	unsigned char slot_time;
	unsigned char tx_tail;
	unsigned char full_duplex;
};

extern struct conf config;

void config_init(void);
void config_read(void);
void config_write(void);

#endif
