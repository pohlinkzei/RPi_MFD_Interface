/*
 * buttons.c
 *
 * Created: 18.08.2014 16:43:01
 *  Author: Hubert
 */ 
#include "buttons.h"
#include "main.h"
#include "rotary.h"


volatile buttons_t buttons, buttons_old, buttons_vold;
extern volatile bool mfd_active;
extern volatile uint8_t buttons_active_count;

void buttons_init(void){
	encode_init();
	BUTTON_DDR &= ~(1<<BUTTON_NAVI) & ~(1<<BUTTON_INFO);
	DDRD &=  ~(1<<BUTTON_VERKEHR) & ~(1<<BUTTON_FAHNE);
	buttons.next = false;
	buttons.prev = false;
	buttons.eject = false;
	buttons.tim = false;
	buttons.info = false;
	buttons.as = false;
	buttons.scan = false;
	buttons._1 = false;
	buttons._2 = false;
	buttons._3 = false;
	buttons.flag = false;
	buttons.light = false;
	buttons.navi = false;
	buttons.back = false;
	buttons.audio = false;
	buttons.tone = false;
	buttons.left_right = 0;
	buttons.enter = false;
	buttons._4 = false;
	buttons._5 = false;
	buttons._6 = false;
	buttons.traffic = false;
}

void buttons_task(void){
	uint8_t i;
	BUTTON_PORT |= (1<<BUTTON_NAVI) | (1<<BUTTON_INFO);
	PORTD |= (1<<BUTTON_VERKEHR) | (1<<BUTTON_FAHNE);
	uint8_t buttons_c = read_adc(BUTTONS_C) >> 2;
	
	if(buttons_c > UPPER3V3){
		buttons.eject = false;
		//buttons. = false; INFO
		buttons.scan = false;
		buttons._3 = false;
		//buttons. = false; FAHNE
		mfd_active = true;
	}else if(buttons_c < UPPER3V3 && buttons_c > LOWER3V3){
		buttons.eject = true;
		mfd_active = true;
	}else if(buttons_c < UPPER2V6 && buttons_c > LOWER2V6){
		//_delay_ms(5000);
		//buttons_init();
		//mfd_active = false;
		//return;
	}else if(buttons_c < UPPER1V8 && buttons_c > LOWER1V8){
		buttons.scan = true;
		mfd_active = true;
	}else if(buttons_c < UPPER0V9 && buttons_c > LOWER0V9){
		buttons._3 = true;
		mfd_active = true;
	}else if(buttons_c < UPPER0V0){
		//_delay_ms(5000);
		//buttons_init();
//		mfd_active = false;
		//return;
	}
	
	
	uint8_t buttons_a = read_adc(BUTTONS_A) >> 2;
	
	if(buttons_a > UPPER3V3){
		buttons.tim = false;
		mfd_active = true;
	}else if(buttons_a < UPPER3V3 && buttons_a > LOWER1V8){//radio wurde abgeschaltet. die spannung an den tasten führt zu unmöglichen messwerten.
		buttons.tim = false;
		//_delay_ms(5000);
		//buttons_init();
		//mfd_active = false;
		//return;
	}else if(buttons_a < UPPER0V9 && buttons_a > LOWER0V9){
		buttons.tim = true;
		mfd_active = true;
	}
	
	uint8_t buttons_b = read_adc(BUTTONS_B) >> 2;
	
	if(buttons_b > UPPER3V3){
		buttons.next = false;
		buttons.prev = false;
		buttons.as = false;
		buttons._1 = false;
		buttons._2 = false;
	}else if(buttons_b < UPPER3V3 && buttons_b > LOWER3V3){
		buttons.next = true;
	}else if(buttons_b < UPPER2V6 && buttons_b > LOWER2V6){
		buttons.prev = true;
	}else if(buttons_b < UPPER1V8 && buttons_b > LOWER1V8){
		buttons.as = true;
	}else if(buttons_b < UPPER0V9 && buttons_b > LOWER0V9){
		buttons._1 = true;
	}else if(buttons_b < UPPER0V0){
		buttons._2 = true;
	}
	
	
	uint8_t buttons_d = read_adc(BUTTONS_D) >> 2;
	
	if(buttons_d > UPPER3V3){
		buttons.enter = false;
		buttons.back = false;
		buttons.audio = false;
		buttons._6 = false;
		mfd_active = true;
	}else if(buttons_d < UPPER3V3 && buttons_d > LOWER3V3){
		buttons.enter = true;
		mfd_active = true;
	}else if(buttons_d < UPPER2V6 && buttons_d > LOWER2V6){
		buttons.back = true;
		mfd_active = true;
		//_delay_ms(5000);
		//buttons_init();
		//mfd_active = false;
		//return;
	}else if(buttons_d < UPPER1V8 && buttons_d > LOWER1V8){
		buttons.audio = true;
		mfd_active = true;
	}else if(buttons_d < UPPER0V9 && buttons_d > LOWER0V9){
		buttons._6 = true;
		mfd_active = true;
	}else if(buttons_d < LOWER0V9){
		
		//_delay_ms(5000);
		//buttons_init();
		//mfd_active = false;
		//return;
	}
	
	uint8_t buttons_e = read_adc(BUTTONS_E) >> 2;
	
	if(buttons_e > UPPER3V3){
		buttons.light = false;
		buttons.back = false;// NAVI
		buttons.tone = false;
		buttons._4 = false;
		buttons._5 = false;
		//buttons. = false; VERKEHR
	}else if(buttons_e < UPPER3V3 && buttons_e > LOWER3V3){
		buttons.light = true;
	}else if(buttons_e < UPPER2V6 && buttons_e > LOWER2V6){
		buttons.back = true;
	}else if(buttons_e < UPPER1V8 && buttons_e > LOWER1V8){
		buttons.tone = true;
	}else if(buttons_e < UPPER0V9 && buttons_e > LOWER0V9){
		buttons._4 = true;
	}else if(buttons_e < UPPER0V0){
		buttons._5 = true;
		
	}
	
	buttons.left_right += encode_read1();//enc_delta;
	buttons.navi = !(BUTTON_PIN & (1<<BUTTON_NAVI));
	buttons.traffic = !(PIND & (1<<BUTTON_VERKEHR));
	buttons.flag = !(PIND & (1<<BUTTON_FAHNE));
	buttons.info = !(BUTTON_PIN & (1<<BUTTON_INFO));
	BUTTON_PORT &= ~(1<<BUTTON_NAVI) & ~(1<<BUTTON_INFO);
	PORTD &= ~(1<<BUTTON_VERKEHR) & ~(1<<BUTTON_FAHNE);
	
	buttons_vold = buttons_old;
	buttons_old = buttons;
}

