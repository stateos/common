#ifndef __BTN_H__
#define __BTN_H__

// btn: { PORTC.5 }

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

struct  __BTNs { uint8_t: PIN5_bp; volatile uint8_t f: 1; uint8_t: 0; };

#define   BTNs   (((struct __BTNs *)&(PORTC.IN))->f)

static inline
void btn_init( int nr )
{
	switch (nr)
	{
		case  0: PORTC.DIRCLR = PIN5_bm; PORTC.PIN5CTRL = PORT_INVEN_bm; break;
		default: break;
	}
}

static inline
bool btn_get( int nr )
{
	switch (nr)
	{
		case  0: return (PORTC.IN & PIN5_bm) != 0;
		default: return false;
	}
}

#endif//__BTN_H__
