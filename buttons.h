
#ifndef BUTTONS_H
#define BUTTONS_H


#include <avr/io.h>
#include <stdbool.h>
#include <string.h>
#include "adc.h"

// MFD Buttons
#define BUTTON_DDR DDRC
#define BUTTON_PORT PORTC
#define BUTTON_PIN PINC
#define BUTTON_NAVI PC7
#define BUTTON_FAHNE PC1
#define BUTTON_INFO PC6
#define BUTTON_VERKEHR PC0
#define BUTTONS_A PA0
#define BUTTONS_B PA1
#define BUTTONS_C PA2
#define BUTTONS_D PA3
#define BUTTONS_E PA4

	// 3,3V 168 153 - 183
	// 2,6V 133 118 - 148
	// 1,8V 92 107 - 137
	// 0,9V 46 31 - 61

#define UPPER3V3 183
#define LOWER3V3 153
#define UPPER2V6 148
#define LOWER2V6 118
#define UPPER1V8 107
#define LOWER1V8 77
#define UPPER0V9 61
#define LOWER0V9 31
#define UPPER0V0 25

typedef struct but{
	bool next;
	bool prev;
	bool eject;
	bool tim;
	bool info;
	bool as;
	bool scan;
	bool _1;
	bool _2;
	bool _3;
	bool flag;
	bool light;
	bool navi;
	bool back;
	bool audio;
	bool tone;
	int8_t left_right;
	bool enter;
	bool _4;
	bool _5;
	bool _6;
	bool traffic;
}buttons_t;


extern volatile buttons_t buttons, buttons_old;

void buttons_init(void);
void buttons_task(void);

#endif