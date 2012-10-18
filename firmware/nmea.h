#ifndef __NMEA_H
#define __NMHEA_H

/* Yes there are other message types, but they aren't used by the firmware (yet) */
enum nmea_sentence_type { GGA, RMC } sentence_type;

struct coordinate {

	unsigned char cardinal_direction; /* N, S, E, W */

	/* NULL terminated strings */
	unsigned char hours[4];
	unsigned char minutes[3];
	unsigned char decimal[3];
};

struct nmea_coordinates {

	struct coordinate latitude;
	struct coordinate longitude;

};

unsigned char nmea_checksum(unsigned char *s);
unsigned char nmea_extact_checksum(unsigned char *s);
unsigned char nmea_validate(unsigned char *s);
char nmea_extract_coordinates(unsigned char *s, struct nmea_coordinates *coordinates);

#endif