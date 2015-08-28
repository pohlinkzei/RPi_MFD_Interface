#ifndef TWI_TASK_H
#define TWI_TASK_H

#include <avr/io.h>
#include "twi_slave.h"


#define CONFIG 0
#define AL 0
#define TFL 1


#define ZinnenH 1
#define ZinnenL 2
#define ZaussenH 3
#define ZaussenL 4
 
void twi_task(void);
uint8_t calculateID(char* name);
#endif