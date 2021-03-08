/******************************************************************************
 * @file    stm32l_discovery_btn.h
 * @author  Rajmund Szymanski
 * @date    18.01.2019
 * @brief   This file contains definitions for STM32L-Discovery Kit.
 ******************************************************************************/

#ifndef __STM32L_DISCOVERY_BTN_H
#define __STM32L_DISCOVERY_BTN_H

#include <stdbool.h>
#include "stm32l1_io.h"

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
	GPIO_Init(GPIOA, GPIO_Pin_0, GPIO_Input_NoPull);

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	RCC->APB2ENR;

//	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA; // SYSCFG->EXTI0(PIN_0) = 0(PORT_A)
	EXTI->IMR  |= EXTI_IMR_MR0;  // interrupt
//	EXTI->EMR  &=~EXTI_EMR_MR0;  // event
	EXTI->RTSR |= EXTI_RTSR_TR0; // rising trigger
//	EXTI->FTSR &=~EXTI_FTSR_TR0; // falling trigger

	NVIC_EnableIRQ(EXTI0_IRQn);
}

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
}
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus

/* -------------------------------------------------------------------------- */

struct Button
{
	Button( void ) { BTN_Init(); }
	
	bool operator ()( void ) { return BTN_Get(); }
};

/* -------------------------------------------------------------------------- */

#endif//__cplusplus

#endif//__STM32L_DISCOVERY_BTN_H
