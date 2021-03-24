/******************************************************************************
 * @file    system_stm32l4xx.c
 * @author  Rajmund Szymanski
 * @date    24.02.2021
 * @brief   This file provides set of configuration functions for STM32L4 uC.
 ******************************************************************************/

#include "stm32l4xx.h"

/* -------------------------------------------------------------------------- */

#ifdef USE_HAL_DRIVER
 const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
 const uint8_t APBPrescTable[8]  = {0, 0, 0, 0, 1, 2, 3, 4};
 const uint32_t MSIRangeTable[12] = {100000U,  200000U,   400000U,   800000U,  1000000U,  2000000U, \
                                    4000000U, 8000000U, 16000000U, 24000000U, 32000000U, 48000000U};
#endif

/* -------------------------------------------------------------------------- */

//#define PLL_SOURCE_HSI        // PLL source: HSI (16MHz)
  #define PLL_SOURCE_HSE        // PLL source: HSE ( 8MHz)
//#define PLL_SOURCE_HSE_BYPASS // PLL source: HSE ( 8MHz)

/* -------------------------------------------------------------------------- */

#define MHz    1000000
#define MSI_FREQ     4 /* MHz */
#define HSI_FREQ    16 /* MHz */
#define HSE_FREQ     8 /* MHz */
#define USB_FREQ    48 /* MHz */
#define CPU_FREQ    80 /* MHz */
#define VDD       3000 /* mV  */

#define LATENCY  ((100*CPU_FREQ+VDD/2)/VDD)

#if     defined (PLL_SOURCE_MSI)
#define PLL_M (MSI_FREQ/4)
#elif   defined (PLL_SOURCE_HSI)
#define PLL_M (HSI_FREQ/4)
#else
#define PLL_M (HSE_FREQ/4)
#endif
#define PLL_N (CPU_FREQ)
#define PLL_P (2)
#define PLL_Q (PLL_N/USB_FREQ)
#define PLL_R (4)

/* -------------------------------------------------------------------------- */

#ifndef __NO_SYSTEM_INIT
__WEAK
void SystemInit( void )
{
	FLASH->ACR = LATENCY | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
#ifdef  PLL_SOURCE_HSI
	RCC->CR |= RCC_CR_HSION;
	while ((RCC->CR & RCC_CR_HSIRDY) == 0);

	RCC->PLLCFGR = (PLL_M<<0) | (PLL_N<<6) | (((PLL_P>>1)-1)<<16) | RCC_PLLCFGR_PLLSRC_HSI | (PLL_Q<<24);
#else
#ifdef  PLL_SOURCE_HSE_BYPASS
	RCC->CR |= RCC_CR_HSEON | RCC_CR_HSEBYP;
#else
	RCC->CR |= RCC_CR_HSEON;
#endif//PLL_SOURCE_HSE_BYPASS
	while ((RCC->CR & RCC_CR_HSERDY) == 0);

	RCC->PLLCFGR = (PLL_M<<0) | (PLL_N<<6) | (((PLL_P>>1)-1)<<16) | RCC_PLLCFGR_PLLSRC_HSE | (PLL_Q<<24);
#endif//PLL_SOURCE_HSI
	RCC->CR |= RCC_CR_PLLON;
	while ((RCC->CR & RCC_CR_PLLRDY) == 0);

	RCC->CFGR = RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2 | RCC_CFGR_SW_PLL;
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
uint32_t SystemCoreClock = MSI_FREQ * MHz;
#endif//__NO_SYSTEM_INIT

/* -------------------------------------------------------------------------- */
