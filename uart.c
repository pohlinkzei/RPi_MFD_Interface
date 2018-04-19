/*
 * uart.c
 *
 * Created: 20.08.2014 10:03:59
 *  Author: Hubert
 */ 
#include <avr/io.h>
#include "uart.h"
#include <util/delay.h>

void USART_Init( unsigned int ubrr){
	/* Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = /*(1<<URSEL0)|*/(1<<USBS0)|(3<<UCSZ00)|(1<<RXCIE0);
}

void USART_Transmit( unsigned char data ){
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
	_delay_us(10);
}

