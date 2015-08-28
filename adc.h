#ifndef ADC_H
#define ADC_H

#include <avr/io.h>

void adc_init(void);
uint16_t read_adc(uint8_t portbit);



#endif