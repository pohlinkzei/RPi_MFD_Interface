/************************************************************************/
/*                                                                      */
/*                      Reading rotary encoder                      */
/*                      one, two and four step encoders supported   */
/*                                                                      */
/*              Author: Peter Dannegger                                 */
/*                                                                      */
/************************************************************************/
#ifndef ROTARY_H
#define ROTARY_H

#include <avr/io.h>
#include <avr/interrupt.h>
 
                // target: ATmega16
//------------------------------------------------------------------------
 
#define XTAL        8e6         // 8MHz
 
#define PHASE_B     (PINA & 1<<PA6)
#define PHASE_A     (PINA & 1<<PA7)
 
extern volatile int8_t enc_delta;          // -128 ... 127
 
void encode_init( void );
int8_t encode_read1( void );        // read single step encoders
int8_t encode_read2( void );         // read two step encoders
int8_t encode_read4( void );        // read four step encoders

 #endif
 
