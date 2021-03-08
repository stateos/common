/******************************************************************************
 * @file    stm32l_discovery_led.h
 * @author  Rajmund Szymanski
 * @date    18.01.2019
 * @brief   This file contains definitions for STM32L-Discovery Kit.
 ******************************************************************************/

#ifndef __STM32L_DISCOVERY_LED_H
#define __STM32L_DISCOVERY_LED_H

#include <stdbool.h>
#include "stm32l1_io.h"

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#define GRN      USB_LED
#define GRN_Init USB_LED_Init

/* -------------------------------------------------------------------------- */

#define     LED (BITBAND(GPIOB->ODR)+6) // leds array
#define     LEDB BITBAND(GPIOB->ODR)[6] // blur led
#define     LEDG BITBAND(GPIOB->ODR)[7] // green led

struct    __LEDs { uint16_t: 6; volatile uint16_t f: 2; uint16_t: 0; uint32_t alignment_for_gcc8; };

#define     LEDs (((struct __LEDs *)&(GPIOB->ODR))->f)

/* -------------------------------------------------------------------------- */

// init leds (PB6..PB7) as pushpull output

static inline
void LED_Init( void )
{
	GPIO_Init(GPIOB, GPIO_Pins(6, 7), GPIO_Output_PushPull);
}

/* -------------------------------------------------------------------------- */

// get user led state

static inline
bool LED_Get( unsigned nr )
{
	switch (nr)
	{
	case  0: return (GPIOB->ODR & GPIO_ODR_ODR_6) != 0;
	case  1: return (GPIOB->ODR & GPIO_ODR_ODR_7) != 0;
	default: return false;
	}
}

/* -------------------------------------------------------------------------- */

// set user led state

static inline
void LED_Set( unsigned nr )
{
	switch (nr)
	{
	case  0: GPIOB->BSRR = GPIO_BSRR_BS_6; break;
	case  1: GPIOB->BSRR = GPIO_BSRR_BS_7; break;
	default: break;
	}
}

/* -------------------------------------------------------------------------- */

// reset user led state

static inline
void LED_Reset( unsigned nr )
{
	switch (nr)
	{
	case  0: GPIOB->BSRR = GPIO_BSRR_BR_6; break;
	case  1: GPIOB->BSRR = GPIO_BSRR_BR_7; break;
	default: break;
	}
}

/* -------------------------------------------------------------------------- */

// toggle user led state

static inline
void LED_Toggle( unsigned nr )
{
	switch (nr)
	{
	case  0: GPIOB->ODR ^= GPIO_ODR_ODR_6; break;
	case  1: GPIOB->ODR ^= GPIO_ODR_ODR_7; break;
	default: break;
	}
}

/* -------------------------------------------------------------------------- */

// rotate leds

static inline
void LED_Tick( void )
{
	unsigned leds = (GPIOB->ODR << 1) & 0xC0;
	GPIOB->BSRR = 0x00C00000;
	GPIOB->BSRR = leds ? leds : 0x40;
}

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
}
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus

/* -------------------------------------------------------------------------- */

struct Led
{
	Led( void ) { LED_Init(); }

	bool get   ( unsigned nr ) { return LED_Get(nr);    }
	void set   ( unsigned nr ) {        LED_Set(nr);    }
	void reset ( unsigned nr ) {        LED_Reset(nr);  }
	void toggle( unsigned nr ) {        LED_Toggle(nr); }
	void tick  ( void )        {        LED_Tick();     }

	unsigned   operator = ( const unsigned status ) { return LEDs = status; }
	unsigned & operator []( const unsigned number ) { return (unsigned &)LED[number]; }
};

/* -------------------------------------------------------------------------- */

#endif//__cplusplus

#endif//__STM32L_DISCOVERY_LED_H
