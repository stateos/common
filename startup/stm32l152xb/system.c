/******************************************************************************
 * @file    system_stm32l1xx.c
 * @author  Rajmund Szymanski
 * @date    18.01.2019
 * @brief   This file provides set of configuration functions for STM32L1 uC.
 ******************************************************************************/

#include <stm32l1xx.h>

/* -------------------------------------------------------------------------- */

#ifdef USE_HAL_DRIVER
 const uint8_t PLLMulTable[9]    = {3, 4, 6, 8, 12, 16, 24, 32, 48};
 const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
 const uint8_t APBPrescTable[8]  = {0, 0, 0, 0, 1, 2, 3, 4};
#endif

/* -------------------------------------------------------------------------- */

  #define PLL_SOURCE_HSI        // PLL source: HSI (16MHz)
//#define PLL_SOURCE_HSE        // PLL source: HSE ( 8MHz)
//#define PLL_SOURCE_HSE_BYPASS // PLL source: HSE ( 8MHz)

/* -------------------------------------------------------------------------- */

#define MiHz   1048576
#define MSI_FREQ     2 /* MiHz */
#define MHz    1000000
#define HSI_FREQ    16 /* MHz */
#define HSE_FREQ     8 /* MHz */
#define CPU_FREQ    32 /* MHz */

#ifdef  PLL_SOURCE_HSI
#define PLLMUL       RCC_CFGR_PLLMUL6
#define PLLDIV       RCC_CFGR_PLLDIV3
#else //PLL_SOURCE_HSE (default)
#define PLLMUL       RCC_CFGR_PLLMUL12
#define PLLDIV       RCC_CFGR_PLLDIV3
#endif

/* -------------------------------------------------------------------------- */

#ifndef __NO_SYSTEM_INIT
__WEAK
void SystemInit( void )
{
	FLASH->ACR |= FLASH_ACR_ACC64;
	FLASH->ACR |= FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY;

	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CR = PWR_CR_VOS_0;  // Voltage scaling range 1 (1.8V)
	while ((PWR->CSR & PWR_CSR_VOSF) != 0);

	RCC->CFGR = RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV1 | RCC_CFGR_PPRE2_DIV1;
#ifdef  PLL_SOURCE_HSI
	RCC->CR |= RCC_CR_HSION;
	while ((RCC->CR & RCC_CR_HSIRDY) == 0);

	RCC->CFGR |= RCC_CFGR_PLLSRC_HSI | PLLMUL | PLLDIV;
#else //PLL_SOURCE_HSE (default)
#ifdef  PLL_SOURCE_HSE_BYPASS
	RCC->CR |= RCC_CR_HSEON | RCC_CR_HSEBYP;
#else
	RCC->CR |= RCC_CR_HSEON;
#endif//PLL_SOURCE_HSE_BYPASS
	while ((RCC->CR & RCC_CR_HSERDY) == 0);

	RCC->CFGR |= RCC_CFGR_PLLSRC_HSE | PLLMUL | PLLDIV;
#endif//PLL_SOURCE_HSI
	RCC->CR |= RCC_CR_PLLON;
	while ((RCC->CR & RCC_CR_PLLRDY) == 0);

	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}
#endif//__NO_SYSTEM_INIT

/* -------------------------------------------------------------------------- */

#ifndef __NO_SYSTEM_INIT
__WEAK
void SystemCoreClockUpdate( void )
{
}
#endif//__NO_SYSTEM_INIT

/* -------------------------------------------------------------------------- */

#ifndef __NO_SYSTEM_INIT
__WEAK
uint32_t SystemCoreClock = CPU_FREQ * MHz;
#else
__WEAK
uint32_t SystemCoreClock = MSI_FREQ * MiHz;
#endif//__NO_SYSTEM_INIT

/* -------------------------------------------------------------------------- */
