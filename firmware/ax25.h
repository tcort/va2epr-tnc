#ifndef __AX25_H
#define __AX25_H

#define AX25_FLAG (0x7e)

#define CRC16CCITT_POLYNOMIAL (0x8408)

unsigned int crc16(unsigned int crc, unsigned char byte);

#endif
