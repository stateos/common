/******************************************************************************
 * @file    stm32f4_discovery_mems.h
 * @author  Rajmund Szymanski
 * @date    18.05.2021
 * @brief   This file contains definitions for STM32F4-Discovery Kit.
 ******************************************************************************/

#ifndef __STM32F4_DISCOVERY_MEMS_H
#define __STM32F4_DISCOVERY_MEMS_H

#include "stm32f4_io.h"

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#define MEMS_DUMMY  ((uint8_t)0x00)
#define MEMS_MULTI  ((uint8_t)0x40)
#define MEMS_RWCMD  ((uint8_t)0x80)

/* -------------------------------------------------------------------------- */

static inline
void MEMS_Init( void )
{
	GPIO_Init(GPIOA, GPIO_Pins(5,6,7), GPIO_Alternate_50MHz | GPIO_PullDown | GPIO_AF5_SPI1); // SCK, MISO, MOSI
	GPIO_Init(GPIOE, GPIO_Pins(0,1),   GPIO_Input);                                           // INT1, INT2
	GPIO_Init(GPIOE, GPIO_Pins(3),     GPIO_Output_PushPull | GPIO_Set);                      // CS

	REG_SET_WAIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN);

	SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | SPI_CR1_BR_0;
	SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | SPI_CR1_BR_0 | SPI_CR1_SPE;
}

/* -------------------------------------------------------------------------- */

static inline
uint8_t MEMS_Send( uint8_t val )
{
	while ((SPI1->SR & SPI_SR_TXE)  == 0);
	SPI1->DR = val;
	while ((SPI1->SR & SPI_SR_RXNE) == 0);
	return (uint8_t) SPI1->DR;
}

/* -------------------------------------------------------------------------- */

static inline
uint8_t MEMS_Read( uint8_t reg )
{
	uint8_t val;
	GPIOE->BSRR = GPIO_BSRR_BR3;
	      MEMS_Send(MEMS_RWCMD | reg);
	val = MEMS_Send(MEMS_DUMMY);
	GPIOE->BSRR = GPIO_BSRR_BS3;
	return val;
}

/* -------------------------------------------------------------------------- */

static inline
void MEMS_Write( uint8_t reg, uint8_t val )
{
	GPIOE->BSRR = GPIO_BSRR_BR3;
	MEMS_Send(reg);
	MEMS_Send(val);
	GPIOE->BSRR = GPIO_BSRR_BS3;
}

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__STM32F4_DISCOVERY_MEMS_H
