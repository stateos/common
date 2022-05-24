/******************************************************************************
 * @file    stm32f0_discovery_leds.h
 * @author  Rajmund Szymanski
 * @date    05.03.2021
 * @brief   This file contains definitions for STM32F0-Discovery Kit.
 ******************************************************************************/

#ifndef __STM32F0_DISCOVERY_LEDS_H
#define __STM32F0_DISCOVERY_LEDS_H

#include <stdbool.h>
#include "stm32f051x8_io.h"

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 200000
struct  __LEDs { uint16_t: 8; volatile uint16_t f: 2; uint16_t: 0; };
struct  __LEDB { uint16_t: 8; volatile uint16_t f: 1; uint16_t: 0; };
struct  __LEDG { uint16_t: 9; volatile uint16_t f: 1; uint16_t: 0; };
#else
struct  __LEDs { uint16_t: 8; volatile uint16_t f: 2; uint16_t: 0; uint32_t _alignment_; };
struct  __LEDB { uint16_t: 8; volatile uint16_t f: 1; uint16_t: 0; uint32_t _alignment_; };
struct  __LEDG { uint16_t: 9; volatile uint16_t f: 1; uint16_t: 0; uint32_t _alignment_; };
#endif

#define   LEDs (((struct __LEDs *)&(GPIOC->ODR))->f) // leds array
#define   LEDB (((struct __LEDB *)&(GPIOC->ODR))->f) // blue led
#define   LEDG (((struct __LEDG *)&(GPIOC->ODR))->f) // green led

/* -------------------------------------------------------------------------- */

// init leds (PC8..PC9) as pushpull output

static inline
void LED_Init( void )
{
	GPIO_Init(GPIOC, GPIO_Pins(8, 9), GPIO_Output_PushPull);
}

/* -------------------------------------------------------------------------- */

// rotate leds

static inline
void LED_Tick( void )
{
	unsigned leds = (GPIOC->ODR << 1) & 0x300;
	GPIOC->BSRR = 0x3000000;
	GPIOC->BSRR = leds ? leds : 0x100;
}

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
}
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
namespace device {

/* -------------------------------------------------------------------------- */

struct Led
{
	Led( void ) { LED_Init(); }

	unsigned get( void )            { return GPIOC->ODR >> 8; }
	void     set( unsigned status ) { GPIOC->BSRR = (~status << 24) | (uint16_t)(status << 8); }

	unsigned   operator = ( const unsigned status ) {                              set(status); return status & 0x3; }
	unsigned   operator ++( void ) /* ++led */      { unsigned status = get() + 1; set(status); return status & 0x3; }
	unsigned   operator ++( int  ) /* led++ */      { unsigned status = get(); set(status + 1); return status; }

	void tick( void ) { LED_Tick(); }
};

/* -------------------------------------------------------------------------- */

}     //  namespace
#endif//__cplusplus

#endif//__STM32F0_DISCOVERY_LEDS_H
