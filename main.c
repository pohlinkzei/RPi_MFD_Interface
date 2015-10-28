/*
 * main.c
 *
 * Created: 18.08.2014 16:45:53
 *  Author: Hubert
 */ 

/*
 * RPi_MFD_Interface.c
 *
 * Created: 30.07.2014 18:32:48
 *  Author: Hubert
 */ 
//#define F_CPU 8000000UL

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
	//TCCR1A |= ;
	TCCR1B |= (1<<WGM12) | (1<<CS11) | (1<<CS10);     // CTC, XTAL / 64
	OCR1A = (uint8_t)(XTAL / 64.0 * 1e-3 - 0.5);   // 1ms
	TIMSK |= 1<<OCIE1A;
}

void pi_shutdown_init(void){
	PISTARTPDDR |= (1<<PISTART);
	PIDDR |= (1<<PISHUTDOWN);
	MCUCR |= /*(1<<ISC00) | */(1<<ISC01);// | (1<<ISC10) | (1<<ISC11);
	GICR |= (1<<INT0);// | (1<<INT1);
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
	}else{
		// pi ist an - soll jetzt abgeschaltet werden
		PIPORT |= (1<<PISHUTDOWN);
	}
	
	// ist der pi 30s inaktiv und fertig, kann er herunter gefahren werden.
	/*
	if(PIPIN & (1<<PIACTIVE)) return; // wenn der pi noch aktiv ist, wird nicht heruntergefahren
	//ab hier ist der pi inaktiv - mp3player aus (idle)
	if(!(PIPIN & (1<<PIREADY))){ //solange der pi noch nicht fertig ist, passiert nichts
 		pi_shutdown_count = 0;
 		return;
	}
		if(pi_shutdown_count > 10000){
			PIPORT |= (1<<PISHUTDOWN);
			_delay_ms(10000);
			PIPORT &= ~(1<<PISHUTDOWN);
			while(PIPIN & (1<<PIREADY)){ // warten bis er wirklich aus ist
				if(shutdown_irq) return;
			}				
			_delay_ms(10000);//sicherheit
			PISTARTPORT &= ~(1<<PISTART);// und abschalten
		}
		//*/
	//}
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
	//init int2
//	GICR |= (1<<INT2);
//	MCUCSR |= (1<<ISC2); //rising edge
}

uint8_t read_3lb(void){
	
	/*
	spi_timeout = 0;
	
	while(SPI_NOT_READY){
		spi_timeout++;
		if(spi_timeout==100) return 0;
	}
	//SPDR = 0;
	//*/
	PORT_3LB |= (1<<EN);
	DDR_3LB |= (1<<EN);
	_delay_us(110);
	PORT_3LB &= ~(1<<EN);
	DDR_3LB &= ~(1<<EN);
	GIFR &= ~(1<<INT2);
	return SPDR;
}


void start_pi(void){
	// starte den pi und warte (!) bis er hochgefahren ist. Blockierend.
	if(!(PISTARTPORT & (1<<PISTART))){ 
		cli();
		PIPORT &= ~(1<<PISHUTDOWN);
		_delay_ms(100);
		PISTARTPORT |= (1<<PISTART);
		_delay_ms(10000);
		//while(!(PIPIN & (1<<PIACTIVE))){
		//	if(shutdown_irq) break;
		//}
		//_delay_ms(200);
		sei();
	}	
}

void mfd_active_check(void){
	if(!mfd_active){
		buttons_active_count++;
		if(buttons_active_count == 10){
			buttons_active_count = 0;
			if(!(PISTARTPORT & (1<<PISTART))) return;
			/*if(ZV_count > 0){//zv hat den pi aktiviert aber das radio ist noch aus. sag dem pi, er soll warten...
				USART_Transmit(0x02);
				return;
			}*/
			uint8_t x = 0;
			do{
				USART_Transmit(0xFF); // MFD ausgeschaltet. Der Pi möge seine Arbeit einstellen.
				_delay_ms(200);
				x++;
				if(x==0) return;
			}while(PIPIN & (1<<PIACTIVE));						
			return;
		}else{
			
			return;
		}
	}else{
		buttons_active_count = 0;					
		start_pi();
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
			while(c--){
				USART_Transmit('+');
				USART_Transmit('0');
			}
		}else{
			while(c++){
				USART_Transmit('-');
				USART_Transmit('0');
			}							
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
	//set_sleep_mode(SLEEP_MODE_IDLE);
	//pi_cooling_init();
	ZV_count = 10;
	USART_Init(MYUBRR);
	//start_pi();
	sei();
	while(1){
		
		//*
		if(buttons_active){
			if(AMP_PIN & (1<<AMP_ON)){
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
						USART_Transmit(0);
						_delay_ms(250);
						break;
					}
					case AUX_INFO:{
						USART_Transmit(0x02);
						_delay_ms(250);
						break;
					}
				}
				mfd_active = true;
				//ZV_count = 0;
			}else{
				pi_shutdown_task();
// 				if(ZV_PIN & (1<<ZV_ZU)){//zv ist zu. pi kann runter gefahren werden
// 					mfd_active = false;
// 				}else{
// 					start_pi();
// 					mfd_active = true;
// 					USART_Transmit(0x02);
// 					_delay_ms(250);
// 				}
			}
			mfd_active_check();				
			//pi_shutdown_task();
			
			buttons_active = false;
		}
		//USART_Transmit(0xFF-SPDR);
		if(!(PISTARTPORT & (1<<PISTART))){
			//sleep_mode();
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

uint8_t wait_for_SPI(uint16_t timeout_us){
	//*
	SPSR &= ~(1<<SPIF);
	while(!(SPSR & (1<<SPIF)) && timeout_us > 0){
		_delay_us(1);
		timeout_us -= 2;	
	}
	//*/
	//_delay_us(timeout_us);
	return 0xFF - SPDR;
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
		//_delay_us(250);
		
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
			//_delay_us(250);
		
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
		//uint8_t myAux = aux_check();
		if(data_3lb[1] == 0){
		//if(myAux != AUX || myAux != AUX_INFO){
			//#warning: "TODO: fix 3lb request timing!"
			//_delay_ms(2);
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
		/*
		uint8_t i = 0;
		for(i=0;i<20;i++){
			USART_Transmit(data_3lb[i]);
		}
		//*/
		
	}else{
		ready_3lb = false;
		//USART_Transmit('X');
	}
	//*/
	
	//USART_Transmit(0xFF - SPDR);
	/*
	if(data == 0xF0){//data > 0x7F || data < 0x20){ 
		count_3lb = 0;
		ready_3lb = false;
	}else if (data == 0xC3){
		ready_3lb = true;
		spi_init(false, 0, 0 );
	}else{
		data_3lb[count_3lb++] = data;
		
	}
	if(count_3lb == 16) ready_3lb = true;
	//*/
	
	DDR_3LB &= ~(1<<EN);
	PORT_3LB &= ~(1<<EN);
}
//ISR( TIMER0_COMP_vect )// 1ms for manual movement

ISR(USART_RXC_vect){
	uint8_t data = UDR;
	if(data == 0xFF){
		shutdown_irq = true;
		mfd_active = true;
	}
}