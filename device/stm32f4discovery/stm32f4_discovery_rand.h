/******************************************************************************
 * @file    stm32f4_discovery_rand.h
 * @author  Rajmund Szymanski
 * @date    18.05.2021
 * @brief   This file contains definitions for STM32F4-Discovery Kit.
 ******************************************************************************/

#ifndef __STM32F4_DISCOVERY_RAND_H
#define __STM32F4_DISCOVERY_RAND_H

#include "stm32f4_io.h"

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

// start random number generator

static inline
void RND_Init( void )
{
	REG_SET_WAIT(RCC->AHB2ENR, RCC_AHB2ENR_RNGEN);

	RNG->CR = RNG_CR_RNGEN;
}

/* -------------------------------------------------------------------------- */

// get next random number

static inline
unsigned RND_Get( void )
{
	while ((RNG->SR & RNG_SR_DRDY) == 0);

	return RNG->DR;
}

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
}
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
namespace device {

/* -------------------------------------------------------------------------- */

struct Rnd
{
	Rnd( void ) { RND_Init(); }

	unsigned get( void ) { return RND_Get(); }

	unsigned operator ()( void ) { return RND_Get(); }
};

/* -------------------------------------------------------------------------- */

}     //  namespace
#endif//__cplusplus

#endif//__STM32F4_DISCOVERY_RAND_H
