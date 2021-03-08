/******************************************************************************
 * @file    stm32f0_discovery_btns.h
 * @author  Rajmund Szymanski
 * @date    05.03.2021
 * @brief   This file contains definitions for STM32F0-Discovery Kit.
 ******************************************************************************/

#ifndef __STM32F0_DISCOVERY_BTNS_H
#define __STM32F0_DISCOVERY_BTNS_H

#include <stdbool.h>
#include "stm32f0_io.h"

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 200000
struct  __BTN { uint16_t: 0; volatile uint16_t f: 1; uint16_t: 0; };
#else
struct  __BTN { uint16_t: 0; volatile uint16_t f: 1; uint16_t: 0; uint32_t _alignment_; };
#endif

#define   BTN (((struct __BTN *)&(GPIOA->IDR))->f) // user button

/* -------------------------------------------------------------------------- */

// init user button (PA0)

static inline
void BTN_Init( void )
{
	GPIO_Init(GPIOA, GPIO_Pins(0), GPIO_Input_PullDown);
}

/* -------------------------------------------------------------------------- */

// get user button state (PA0)

static inline
bool BTN_Get( void )
{
	return BTN;
}

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
}
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
namespace device {

/* -------------------------------------------------------------------------- */

struct Button
{
	Button( void ) { BTN_Init(); }
	
	bool operator ()( void ) { return BTN_Get(); }
};

/* -------------------------------------------------------------------------- */

}     //  namespace
#endif//__cplusplus

#endif//__STM32F0_DISCOVERY_BTNS_H
