#ifndef __LED_H__
#define __LED_H__

// led: { PORTC.0 }

#include <avr/io.h>
#include <stdint.h>

struct  __LEDs { uint8_t: PIN0_bp; volatile uint8_t f: 1; uint8_t: 0; };

#define   LEDs   (((struct __LEDs *)&(PORTC.OUT))->f)

static inline
void led_init( int nr )
{
	switch (nr)
	{
		case  0: PORTC.DIRSET = PIN0_bm; break;
		default: break;
	}
}

static inline
void led_set( int nr )
{
	switch (nr)
	{
		case  0: PORTC.OUTSET = PIN0_bm; break;
		default: break;
	}
}

static inline
void led_clear( int nr )
{
	switch (nr)
	{
		case  0: PORTC.OUTCLR = PIN0_bm; break;
		default: break;
	}
}

static inline
void led_toggle( int nr )
{
	switch (nr)
	{
		case  0: PORTC.OUTTGL = PIN0_bm; break;
		default: break;
	}
}

#endif//__LED_H__
