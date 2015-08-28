/*
 * spi.c
 *
 * Created: 31.07.2014 14:48:52
 *  Author: Hubert
 */ 

#include "spi.h"
#include <util/delay.h>
#include <stdbool.h>

#define DDR_SPI DDRB
#define DD_MISO PB6


void spi_init(bool master, uint8_t cpol, uint8_t phase){
	//SPSR |= (1<<CPOL);
	
	//DDR_SPI = (1<<MISO) | (1<<PB3);
	//PORTB |= (1<<SS);
	// Enable SPI 
	//SPCR |= (1<<CPOL) | (1<<CPHA) | (1<<SPIE) | (1<<SPE);
	SPCR = 0x00;
	_delay_us(5);
	
	
	/* Set MISO output, all others input */
	DDR_SPI |= (1<<DD_MISO);
	/* Enable SPI */
	SPCR |= (1<<CPOL) | (1<<CPHA) | (1<<SPE) | (1<<SPIE);
}

