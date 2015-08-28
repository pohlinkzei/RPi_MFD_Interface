
#include <avr/io.h>
#define FOSC F_CPU// Clock Speed
#define BAUD 38400ul
#define MYUBRR FOSC/16/BAUD-1

//extern ISR(USART_RXC_vect);
void USART_Init(unsigned int ubrr);
void USART_Transmit(unsigned char data);