#ifndef __CSMA_H
#define __CSMA_H

extern volatile unsigned int carrier_sense;

void csma_obtain_slot(void);

#endif