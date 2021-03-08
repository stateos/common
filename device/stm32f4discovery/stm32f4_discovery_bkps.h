/******************************************************************************
 * @file    stm32f4_discovery_bkps.h
 * @author  Rajmund Szymanski
 * @date    04.03.2021
 * @brief   This file contains definitions for STM32F4-Discovery Kit.
 ******************************************************************************/

#ifndef __STM32F4_DISCOVERY_BKPS_H
#define __STM32F4_DISCOVERY_BKPS_H

#include "stm32f4_io.h"

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#ifdef  __GNUC__
#define __bkp __attribute__ ((section(".bkp")))
#endif

#ifdef  __CC_ARM
#define __bkp __attribute__ ((section("BKP")))
#endif

/* -------------------------------------------------------------------------- */

// init backup sram

static inline
void BKP_Init( void )
{
	REG_SET_BITS(RCC->APB1ENR, RCC_APB1ENR_PWREN);
	RCC->APB1ENR;
	REG_SET_BITS(PWR->CR,      PWR_CR_DBP);
	REG_SET_BITS(RCC->AHB1ENR, RCC_AHB1ENR_BKPSRAMEN);
	RCC->AHB1ENR;
}

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__STM32F4_DISCOVERY_BKPS_H
