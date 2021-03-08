/******************************************************************************
 * @file    stm32f4_discovery_leds.h
 * @author  Rajmund Szymanski
 * @date    04.03.2021
 * @brief   This file contains definitions for STM32F4-Discovery Kit.
 ******************************************************************************/

#ifndef __STM32F4_DISCOVERY_LEDS_H
#define __STM32F4_DISCOVERY_LEDS_H

#include <stdbool.h>
#include "stm32f4_io.h"

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#define GRN      USB_LED
#define GRN_Init USB_LED_Init

/* -------------------------------------------------------------------------- */

#define     LED (BITBAND(GPIOD->ODR)+12) // leds array
#define     LEDG BITBAND(GPIOD->ODR)[12] // green led
#define     LEDO BITBAND(GPIOD->ODR)[13] // orange led
#define     LEDR BITBAND(GPIOD->ODR)[14] // red led
#define     LEDB BITBAND(GPIOD->ODR)[15] // blue led

#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 200000
struct    __LEDs { uint16_t: 12; volatile uint16_t f: 4; uint16_t: 0; };
#else
struct    __LEDs { uint16_t: 12; volatile uint16_t f: 4; uint16_t: 0; uint32_t _alignment_; };
#endif

#define     LEDs (((struct __LEDs *)&(GPIOD->ODR))->f)

#define USB_LED  BITBAND(GPIOA->ODR)[ 9] // usb green led

#define PWM_LED  (&(TIM4->CCR1)) // pwm leds power array (0..100%)
#define PWM_LEDG   (TIM4->CCR1)  // pwm green led power (0..100%)
#define PWM_LEDO   (TIM4->CCR2)  // pwm orange led power (0..100%)
#define PWM_LEDR   (TIM4->CCR3)  // pwm red led power (0..100%)
#define PWM_LEDB   (TIM4->CCR4)  // pwm blue led power (0..100%)

/* -------------------------------------------------------------------------- */

// init leds (PD12..PD15) as pushpull output

static inline
void LED_Init( void )
{
	GPIO_Init(GPIOD, GPIO_Pins(12,13,14,15), GPIO_Output_PushPull);
}

/* -------------------------------------------------------------------------- */

// get user led state

static inline
bool LED_Get( unsigned nr )
{
	bool result = false;
	switch (nr)
	{
	case 0: result = (GPIOD->ODR & GPIO_ODR_OD12) != 0; break;
	case 1: result = (GPIOD->ODR & GPIO_ODR_OD13) != 0; break;
	case 2: result = (GPIOD->ODR & GPIO_ODR_OD14) != 0; break;
	case 3: result = (GPIOD->ODR & GPIO_ODR_OD15) != 0; break;
	}
	return result;
}

/* -------------------------------------------------------------------------- */

// set user led state

static inline
void LED_Set( unsigned nr )
{
	switch (nr)
	{
	case 0: GPIOD->BSRR = GPIO_BSRR_BS12; break;
	case 1: GPIOD->BSRR = GPIO_BSRR_BS13; break;
	case 2: GPIOD->BSRR = GPIO_BSRR_BS14; break;
	case 3: GPIOD->BSRR = GPIO_BSRR_BS15; break;
	}
}

/* -------------------------------------------------------------------------- */

// reset user led state

static inline
void LED_Reset( unsigned nr )
{
	switch (nr)
	{
	case 0: GPIOD->BSRR = GPIO_BSRR_BR12; break;
	case 1: GPIOD->BSRR = GPIO_BSRR_BR13; break;
	case 2: GPIOD->BSRR = GPIO_BSRR_BR14; break;
	case 3: GPIOD->BSRR = GPIO_BSRR_BR15; break;
	}
}

/* -------------------------------------------------------------------------- */

// toggle user led state

static inline
void LED_Toggle( unsigned nr )
{
	switch (nr)
	{
	case 0: REG_REV_BITS(GPIOD->ODR, GPIO_ODR_OD12); break;
	case 1: REG_REV_BITS(GPIOD->ODR, GPIO_ODR_OD13); break;
	case 2: REG_REV_BITS(GPIOD->ODR, GPIO_ODR_OD14); break;
	case 3: REG_REV_BITS(GPIOD->ODR, GPIO_ODR_OD15); break;
	}
}

/* -------------------------------------------------------------------------- */

// rotate leds

static inline
void LED_Tick( void )
{
	unsigned leds = (GPIOD->ODR << 1) & 0xE000;
	GPIOD->BSRR = 0xF0000000;
	GPIOD->BSRR = leds ? leds : 0x1000;
}

/* -------------------------------------------------------------------------- */

// init usb green led (PA9)

static inline
void USB_LED_Init( void )
{
	GPIO_Init(GPIOA, GPIO_Pins(9), GPIO_Output_PushPull);
}

/* -------------------------------------------------------------------------- */

// get usb green led state

static inline
bool USB_LED_Get( void )
{
	return (GPIOA->ODR & GPIO_ODR_OD9) != 0;
}

/* -------------------------------------------------------------------------- */

// set usb green led state

static inline
void USB_LED_Set( void )
{
	GPIOA->BSRR = GPIO_BSRR_BS9;
}

/* -------------------------------------------------------------------------- */

// reset usb green led state

static inline
void USB_LED_Reset( void )
{
	GPIOA->BSRR = GPIO_BSRR_BR9;
}

/* -------------------------------------------------------------------------- */

// toggle usb green led state

static inline
void USB_LED_Toggle( void )
{
	REG_REV_BITS(GPIOA->ODR, GPIO_ODR_OD9);
}

/* -------------------------------------------------------------------------- */

// init pwm leds (PD12..PD15) as alternate output

static inline
void PWM_LED_Init( void )
{
	GPIO_Init(GPIOD, GPIO_Pins(12,13,14,15), GPIO_Alternate_100MHz | GPIO_AF2_TIM4);

	REG_SET_BITS(RCC->APB1ENR, RCC_APB1ENR_TIM4EN);
	RCC->APB1ENR;

	TIM4->PSC   = 0;
	TIM4->ARR   = 99;
	TIM4->CCMR1 = TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC2PE | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;
	TIM4->CCMR2 = TIM_CCMR2_OC3PE | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC4PE | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2;
//	TIM4->CCER  = TIM_CCER_CC1E   | TIM_CCER_CC2E    | TIM_CCER_CC3E    | TIM_CCER_CC4E;
	TIM4->CCER  = 0;
//	TIM4->CCR1  = 0;
//	TIM4->CCR2  = 0;
//	TIM4->CCR3  = 0;
//	TIM4->CCR4  = 0;
	TIM4->CR1   = TIM_CR1_CEN;
}

/* -------------------------------------------------------------------------- */

// get pwm led state

static inline
unsigned PWM_LED_Get( unsigned nr )
{
	unsigned result = 0;
	switch (nr)
	{
	case 0: result = (TIM4->CCER & TIM_CCER_CC1E) ? TIM4->CCR1 : 0; break;
	case 1: result = (TIM4->CCER & TIM_CCER_CC2E) ? TIM4->CCR2 : 0; break;
	case 2: result = (TIM4->CCER & TIM_CCER_CC3E) ? TIM4->CCR3 : 0; break;
	case 3: result = (TIM4->CCER & TIM_CCER_CC4E) ? TIM4->CCR4 : 0; break;
	}
	return result;
}

/* -------------------------------------------------------------------------- */

// set pwm led state
// state must be in the range of 0..100%

static inline
void PWM_LED_Set( unsigned nr, unsigned state )
{
	switch (nr)
	{
	case 0: if (state) { TIM4->CCR1 = state; REG_SET_BITS(TIM4->CCER, TIM_CCER_CC1E); } else REG_CLR_BITS(TIM4->CCER, TIM_CCER_CC1E); break;
	case 1: if (state) { TIM4->CCR2 = state; REG_SET_BITS(TIM4->CCER, TIM_CCER_CC2E); } else REG_CLR_BITS(TIM4->CCER, TIM_CCER_CC2E); break;
	case 2: if (state) { TIM4->CCR3 = state; REG_SET_BITS(TIM4->CCER, TIM_CCER_CC3E); } else REG_CLR_BITS(TIM4->CCER, TIM_CCER_CC3E); break;
	case 3: if (state) { TIM4->CCR4 = state; REG_SET_BITS(TIM4->CCER, TIM_CCER_CC4E); } else REG_CLR_BITS(TIM4->CCER, TIM_CCER_CC4E); break;
	}
}

/* -------------------------------------------------------------------------- */

// reset pwm led state

static inline
void PWM_LED_Reset( unsigned nr )
{
	switch (nr)
	{
	case 0: REG_CLR_BITS(TIM4->CCER, TIM_CCER_CC1E); break;
	case 1: REG_CLR_BITS(TIM4->CCER, TIM_CCER_CC2E); break;
	case 2: REG_CLR_BITS(TIM4->CCER, TIM_CCER_CC3E); break;
	case 3: REG_CLR_BITS(TIM4->CCER, TIM_CCER_CC4E); break;
	}
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

	bool get   ( unsigned nr ) { return LED_Get(nr);    }
	void set   ( unsigned nr ) {        LED_Set(nr);    }
	void reset ( unsigned nr ) {        LED_Reset(nr);  }
	void toggle( unsigned nr ) {        LED_Toggle(nr); }
	void tick  ( void )        {        LED_Tick();     }

	unsigned   operator = ( const unsigned status ) { LEDs = status & 0xF; return status & 0xF; }
	unsigned & operator []( const unsigned number ) { return (unsigned &)LED[number]; }
};

/* -------------------------------------------------------------------------- */

struct USB_Led
{
	USB_Led( void ) { USB_LED_Init(); }

	bool get   ( void ) { return USB_LED_Get   (); }
	void set   ( void ) {        USB_LED_Set   (); }
	void reset ( void ) {        USB_LED_Reset (); }
	void toggle( void ) {        USB_LED_Toggle(); }

	operator   unsigned & ( void )                  { return (unsigned &) GRN; }
	unsigned   operator = ( const unsigned status ) { USB_LED = status; return status; }
};

using GreenLed = USB_Led;

/* -------------------------------------------------------------------------- */

struct PWM_Led
{
	PWM_Led( void ) { PWM_LED_Init(); }

	unsigned get  ( unsigned nr )                 { return PWM_LED_Get  (nr);        }
	void     set  ( unsigned nr, unsigned state ) {        PWM_LED_Set  (nr, state); }
	void     reset( unsigned nr )                 {        PWM_LED_Reset(nr);        }

	unsigned & operator []( const unsigned number ) { return (unsigned &)PWM_LED[number]; }
};

/* -------------------------------------------------------------------------- */

}     //  namespace
#endif//__cplusplus

#endif//__STM32F4_DISCOVERY_LEDS_H
