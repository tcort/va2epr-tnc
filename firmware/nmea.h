#ifndef __NMEA_H
#define __NMHEA_H

unsigned char nmea_checksum(unsigned char *s);
unsigned char nmea_extact_checksum(unsigned char *s);
unsigned char nmea_validate(unsigned char *s);

#endif
