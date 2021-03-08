/******************************************************************************
 * @file    stm32f4_discovery_btns.h
 * @author  Rajmund Szymanski
 * @date    04.03.2021
 * @brief   This file contains definitions for STM32F4-Discovery Kit.
 ******************************************************************************/

#ifndef __STM32F4_DISCOVERY_BTNS_H
#define __STM32F4_DISCOVERY_BTNS_H

#include <stdbool.h>
#include "stm32f4_io.h"

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#define BTN BITBAND(GPIOA->IDR)[0] // user button

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

// init user button (PA0) as a source of exti interrupt

static inline
void EXTI_BTN_Init( void )
{
	GPIO_Init(GPIOA, GPIO_Pins(0), GPIO_Input_NoPull);

	REG_SET_BITS(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
	RCC->APB2ENR;

//	REG_SET_BITS(SYSCFG->EXTICR[0], SYSCFG_EXTICR1_EXTI0_PA); // SYSCFG->EXTI0(PIN_0) = 0(PORT_A)
	REG_SET_BITS(EXTI->IMR,  EXTI_IMR_MR0);  // interrupt
//	REG_CLR_BITS(EXTI->EMR,  EXTI_EMR_MR0);  // event
	REG_SET_BITS(EXTI->RTSR, EXTI_RTSR_TR0); // rising trigger
//	REF_CLR_BITS(EXTI->FTSR, EXTI_FTSR_TR0); // falling trigger

	NVIC_EnableIRQ(EXTI0_IRQn);
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

#endif//__STM32F4_DISCOVERY_BTNS_H
