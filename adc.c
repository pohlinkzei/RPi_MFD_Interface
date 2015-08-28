/*
 * adc.c
 *
 * Created: 31.07.2014 14:29:09
 *  Author: Hubert
 */ 

#include "adc.h"

/***********************************************************************************************/
void adc_init(void){
	ADMUX |= (1<<REFS0);										// VCC with external capacitor at AREF
	ADCSRA |= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); // ADC enabled, prescaler: 128
	//ADCSRB = 0x00;
}

/***********************************************************************************************/
uint16_t read_adc(uint8_t portbit){
	uint8_t i;
	uint16_t result = 0;
	
	ADCSRA &= ~(1<<ADEN);
	ADMUX |= portbit;
	ADCSRA |= (1<<ADSC) | (1<<ADEN);
	while(ADCSRA & (1<<ADSC)){;}
	result = ADCW; //dummy readout
	result = 0;
	for(i=0;i<4;i++){
		ADCSRA |= (1<<ADSC);
		while(ADCSRA & (1<<ADSC)){;}
		result += ADCW;
	}
	ADCSRA &= ~(1<<ADEN);
	ADMUX &= ~portbit;
	return (result>>2);
}
