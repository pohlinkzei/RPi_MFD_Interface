

#ifndef MAIN_H
#define MAIN_H
#ifndef F_CPU
#define F_CPU 8000000UL
#endif
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

// 3LB
#define PORT_3LB PORTB	
#define DDR_3LB	DDRB
#define EN		PB2 //INT2
#define DATA	PB5 //MOSI
#define CLK		PB7 //SCK
#define AUX		1
#define NOT_AUX	0
#define AUX_INFO 2
#define AUX_NR 3

// Shutdown Pi
#define PIPORT PORTB
#define PIPIN PINB
#define PIDDR DDRB
#define PIACTIVE PB3
#define PISHUTDOWN PB1
#define PIREADY PB0

// Start Pi
#define PISTARTPDDR DDRD
#define PISTARTPORT PORTD
#define PISTART PD6 

#define ZV_PIN	PIND
#define ZV_ZU	PB2

#define AMP_PIN PIND
#define AMP_ON PB3

// cool Pi
#define PWM OCR2
#define PWM_HIGH 255
#define PWM_MED	128
#define PWM_LOW	64
#define PWM_OFF 0
#define TEMP_HIGH 350
#define TEMP_MED 300
#define TEMP_LOW 275
#define PI_TEMP PA5

uint8_t read_3lb(void);
//extern void reset (void) __attribute__((noreturn));

extern volatile uint16_t pi_shutdown_count;
extern volatile uint16_t pi_start_count;
extern volatile uint8_t data_3lb[20];
extern volatile bool status;
extern uint8_t count_3lb;
extern volatile uint32_t spi_timeout;

#endif