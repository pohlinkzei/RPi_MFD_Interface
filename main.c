/*
 * main.c
 *
 * Created: 18.08.2014 16:45:53
 *  Author: Hubert
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <string.h>
#include "main.h"
#include "adc.h"
#include "spi.h"
#include "uart.h"

#include "buttons.h"
#include "rotary.h"

#define WAIT 5

extern volatile buttons_t buttons, buttons_old, buttons_vold;
volatile bool mfd_active = false;
volatile uint16_t pi_shutdown_count = 0;
volatile uint16_t pi_start_count = 0;
volatile uint8_t data_3lb[20];
volatile bool status = false;
uint8_t count_3lb = 0;
volatile uint32_t spi_timeout;
volatile uint16_t mp_cnt = 0;
volatile bool buttons_active = false;
volatile uint8_t buttons_active_count = 0;
volatile int32_t ZV_count;
volatile bool ready_3lb = false;
volatile uint8_t count_3lb_max = 0xFF;
volatile bool shutdown_irq = false;

void timer1_init(void){
	TCCR1B |= (1<<WGM12) | (1<<CS11) | (1<<CS10);     // CTC, XTAL / 64
	OCR1A = (uint8_t)(XTAL / 64.0 * 1e-3 - 0.5);   // 1ms
	TIMSK |= 1<<OCIE1A;
}

void pi_shutdown_init(void){
	PISTARTPDDR |= (1<<PISTART);
	PIDDR |= (1<<PISHUTDOWN);
	pi_shutdown_count = 0;
}

void pi_shutdown_task(void){
	if(!(PISTARTPORT & (1<<PISTART))) return; // pi ausgeschaltet? dann zurück!
	//pi herunterfahren
	if(PIPORT & (1<<PISHUTDOWN)){
		// pi hat shutdown signal bekommen: ist er schon aus?
		if(PIPIN & (1<<PIACTIVE)){
			//noch an... da machen wir nix.
			return;
		}
		//hier isser schon aus.
		PIPORT &= ~(1<<PISHUTDOWN);
		_delay_ms(10000);//sicherheit
		PISTARTPORT &= ~(1<<PISTART);// und abschalten
		_delay_ms(10000);//sicherheit
	}else{
		// pi ist an - soll jetzt abgeschaltet werden
		if(!(PIPIN & (1<<PIREADY))){
			uint8_t x = 0;
			do{
				USART_Transmit(0xFF); // MFD ausgeschaltet. Der Pi m�ge seine Arbeit einstellen.
				_delay_ms(200);
				x++;
				if(x>50) break;
			}while(PIPIN & (1<<PIREADY));
		}
		PIPORT |= (1<<PISHUTDOWN);
	}
}

void pi_cooling_init(){
	//timer2 init
    ASSR  = 0x00;
    TCNT2 = 0x01;
    OCR2 = 0xFF;
    //TCCR2 = 0x00; 		// Einfacher Timer
    TCCR2 |= (1<<WGM20) | (1<<COM21);	// phase-correct, non-inverted PWM
	TCCR2 |= (1<<CS21);				
}

void pi_cooling_task(void){
	uint16_t temperature = read_adc(PI_TEMP);
	if(temperature > TEMP_HIGH){
		PWM = PWM_HIGH;
		return;
	}
	if(temperature > TEMP_MED){
		PWM = PWM_MED;
		return;
	}
	if(temperature >TEMP_LOW){
		PWM = PWM_LOW;
		return;
	}
	PWM = PWM_OFF;
}

void init_3lb(void){
	spi_init(false,1,0);
}

void start_pi(void){
	if(!(PISTARTPORT & (1<<PISTART))){ 
		cli();
		int i = 0;
		PIPORT &= ~(1<<PISHUTDOWN);
		_delay_ms(100);
		PISTARTPORT |= (1<<PISTART);
		for(;i<30;i++){
			_delay_ms(1000);
		}		
		sei();
	}	
}

void uart_task(){
//*
	if(buttons_old._1) USART_Transmit('1');
	if(buttons_old._2) USART_Transmit('2');
	if(buttons_old._3) USART_Transmit('3');
	if(buttons_old._4) USART_Transmit('4');
	if(buttons_old._5) USART_Transmit('5');
	if(buttons_old._6) USART_Transmit('6');
	if(buttons_old.next) USART_Transmit('n');
	if(buttons_old.prev) USART_Transmit('p');
	if(buttons_old.eject) USART_Transmit('e');
	if(buttons_old.tim) USART_Transmit('t');
	if(buttons_old.info) USART_Transmit('i');
	if(buttons_old.as) USART_Transmit('a');
	if(buttons_old.scan) USART_Transmit('s');
	if(buttons_old.flag) USART_Transmit('f');
	if(buttons_old.light) USART_Transmit('l');
	if(buttons_old.navi) USART_Transmit('N');
	if(buttons_old.back) USART_Transmit('b');
	if(buttons_old.audio) USART_Transmit('A');
	if(buttons_old.tone) USART_Transmit('T');
	if(buttons_old.left_right){
		int8_t c = buttons_old.left_right;
		if(c > 0){
			//while(c--){
				USART_Transmit('+');
				//_delay_ms(WAIT);
				USART_Transmit(c);
				//_delay_ms(WAIT);
			//}
		}else{
			//while(c++){
				c *= -1:
				USART_Transmit('-');
				//_delay_ms(WAIT);
				USART_Transmit(c);
				//_delay_ms(WAIT);
			//}							
		}
	}	
	if(buttons_old.enter) USART_Transmit('E');
	if(buttons_old.traffic) USART_Transmit('v');
	
	if(buttons.next == false && buttons.prev == false && buttons.eject == false && buttons.tim == false && buttons.info == false && buttons.as == false && buttons.scan == false && buttons._1 == false &&	buttons._2 == false && buttons._3 == false && buttons.flag == false && buttons.light == false && buttons.navi == false && buttons.back == false && buttons.audio == false && buttons.tone == false && buttons.left_right == 0 && buttons.enter == false && buttons._4 == false && buttons._5 == false && buttons._6 == false && buttons.traffic == false){
		USART_Transmit('0');
	}
	buttons_old = buttons;
//*/	
	buttons_init();
	
}

uint8_t aux_check(void){
	//  01234567 89012345
	// [  AUX            ] 
	// [  AUX          TP]
	// [  AUX    INFO  TP]
	//return AUX;
	//if(ready_3lb){
		if(strncmp( (char*) &data_3lb[1], "  AUX   ", 8) == 0){
			if(strncmp( (char*) &data_3lb[9], "INFO  TP", 8) == 0){
				return AUX_INFO;
			}else{
				return AUX;
			}	
		}else{
			spi_init(false, 0,0);
			return NOT_AUX;
		}
	//}
	return AUX_NR; // not ready
}	
	

int main(void){
	adc_init();
	init_3lb();
	timer1_init();
    buttons_init();
	pi_shutdown_init();
	set_sleep_mode(SLEEP_MODE_IDLE);
	ZV_count = 10;
	USART_Init(MYUBRR);
	sei();
	while(1){
		//*
		if(buttons_active){
			if((AMP_PIN & (1<<AMP_ON))){
				start_pi();
				switch(aux_check()){
					case AUX:{
						buttons_task();
						uart_task();
						break;
					}
					case NOT_AUX:{
						USART_Transmit(0x01);
						_delay_ms(250);
						uint8_t i;
						for(i=1; i<17; i++){
							USART_Transmit(data_3lb[i]);
							_delay_ms(25);
						}
						USART_Transmit(0x00);
						_delay_ms(250);
						break;
					}
					case AUX_INFO:{
						USART_Transmit(0x01);
						_delay_ms(250);
						break;
					}
				}
				mfd_active = true;
			}else{
				pi_shutdown_task();
/*
				if(ZV_PIN & (1<<ZV_ZU)){
					pi_shutdown_task();
				}else{
					start_pi();
					USART_Transmit(0x02);
					_delay_ms(250);
				}
*/
			}
			buttons_active = false;
		}
		if(!(PISTARTPORT & (1<<PISTART))){
			sleep_mode();
			buttons_active = true;
		}	
		//*/		
    }
	return 0;
}

ISR(TIMER1_COMPA_vect){// 1ms Timer
	pi_start_count++;
	pi_shutdown_count++;
	mp_cnt++;
	if(mp_cnt == 100){
		mp_cnt = 0;
		buttons_active = true;
	}
}

ISR(SPI_STC_vect){
	//*
	uint8_t data = 0xFF-SPDR;
	SPSR &= ~(1<<SPIF);
	if(data == (0x81) || data == 0xC1){// data == 0x56 || data == 0x53 für navi frames
		uint8_t i = 0;
		_delay_us(150);
		PORT_3LB |= (1<<EN);
		DDR_3LB |= (1<<EN);
		SPSR &= ~(1<<SPIF);
		uint16_t timeout_us = 500;
		while(!(SPSR & (1<<SPIF)) && timeout_us > 0){
			_delay_us(10);
			timeout_us -= 20;	
		}
		data =  0xFF-SPDR;
		_delay_us(20);
		DDR_3LB &= ~(1<<EN);
		PORT_3LB &= ~(1<<EN);
		while(data--){
			_delay_us(150);
			PORT_3LB |= (1<<EN);
			DDR_3LB |= (1<<EN);		
			SPSR &= ~(1<<SPIF);
			timeout_us = 500;
			while(!(SPSR & (1<<SPIF)) && timeout_us > 0){
				_delay_us(10);
				timeout_us -= 20;	
			}
			data_3lb[i++] =  0xFF-SPDR;
			_delay_us(20);
			DDR_3LB &= ~(1<<EN);
			PORT_3LB &= ~(1<<EN);
		}
	}else if(data == 0xC3){
		if(data_3lb[1] == 0){
			PORT_3LB |= (1<<EN);
			DDR_3LB |= (1<<EN);
			_delay_us(3500);
			DDR_3LB &= ~(1<<EN);
			PORT_3LB &= ~(1<<EN);
			_delay_ms(2);
			PORT_3LB |= (1<<EN);
			DDR_3LB |= (1<<EN);
			_delay_us(3500);
			DDR_3LB &= ~(1<<EN);
			PORT_3LB &= ~(1<<EN);
		}
		ready_3lb = true;		
	}else{
		ready_3lb = false;
	}
	//*/
	DDR_3LB &= ~(1<<EN);
	PORT_3LB &= ~(1<<EN);
}
//ISR( TIMER0_COMP_vect )// 1ms for manual movement

ISR(USART_RXC_vect){/*
	uint8_t data = UDR;
	if(data == 0xFF){
		shutdown_irq = true;
		//mfd_active = true;
	}*/
}


ISR(INT0_vect){
	start_pi();
}

