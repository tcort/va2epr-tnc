#ifndef __CRC_H
#define __CRC_H

#define CRC16CCITT_POLYNOMIAL (0x8408)

unsigned int crc16(unsigned int crc, unsigned char byte);

#endif
