/*
 * twi_task.c
 *
 * Created: 21.08.2014 09:32:08
 *  Author: Hubert
 */ 

#include <avr/io.h>
#include "twi_slave.h"
#include "twi_task.h"
#include "timer.h"
#include "tfl.h"


extern volatile uint8_t i2cdata[i2c_buffer_size]; 
extern volatile struct_COUNTDOWN	Countdown;
extern volatile struct_TFL	Tfl;
extern volatile uint8_t buffer_adr;

uint8_t calculateID(char* name){
	//calculate an ID from the first 3 Letter of its name
	uint8_t ID;
	ID = (name[0]-48) * 3 + (name[1]-48) * 2 + (name[2]-48);
	ID >> 2;
	return ID;
}

void twi_task(void){
	#warning: TODO: "implement TWI task!"		
}
