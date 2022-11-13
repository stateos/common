/******************************************************************************

   @file    startup.c
   @author  Rajmund Szymanski
   @date    13.11.2022
   @brief   Startup file for STM32L476VG uC

 ******************************************************************************

   Copyright (c) 2018-2022 Rajmund Szymanski. All rights reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 ******************************************************************************/

#define __PROGRAM_START
#include "stm32l4xx.h"

/*******************************************************************************
 Configuration of stacks
*******************************************************************************/

#ifndef   main_stack_size
#define   main_stack_size    0    // <- default size of main stack
#endif
#ifndef   proc_stack_size
#define   proc_stack_size    0    // <- default size of process stack
#endif

/*******************************************************************************
 Initial stacks' pointers
*******************************************************************************/

extern char __initial_msp[];
extern char __initial_sp [];

/*******************************************************************************
 Default fault handler
*******************************************************************************/

static
void Fault_Handler( void )
{
	/* Go into an infinite loop */
	for (;;);
}

/* Default _exit handler */
__WEAK
void _exit( int code )
{
	(void) code;

	/* Go into an infinite loop */
	for (;;);
}

/*******************************************************************************
 Specific definitions for the compiler
*******************************************************************************/

#if   defined( __CC_ARM )
	#include "armcc/startup.h"
#elif defined( __ARMCOMPILER_VERSION )
	#include "clang/startup.h"
#elif defined( __GNUC__ )
	#include "gnucc/startup.h"
#elif defined( __ICCARM__ )
	#include "iarcc/startup.h"
#elif defined( __CSMC__ )
	#include "csmcc/startup.h"
#else
	#error Unknown or unsupported compiler.
#endif

/*******************************************************************************
 Default reset handler
*******************************************************************************/

__WEAK __NO_RETURN
void Reset_Handler( void )
{
#if proc_stack_size > 0
	/* Initialize the process stack pointer */
	__set_PSP((uint32_t) __initial_sp);
	__set_CONTROL(CONTROL_SPSEL_Msk);
#endif
#if __FPU_USED && !defined(__ICCARM__)
	/* Set CP10 and CP11 Full Access */
	SCB->CPACR = 0x00F00000U;
#endif
#ifndef __NO_SYSTEM_INIT
	/* Call the system clock intitialization function */
	SystemInit();
#endif
	/* Call the application's entry point */
	__main();
}

/*******************************************************************************
 Declaration of exception handlers
*******************************************************************************/

/* Core exception handlers */
__ALIAS(Fault_Handler) void NMI_Handler                   (void);
__ALIAS(Fault_Handler) void HardFault_Handler             (void);
__ALIAS(Fault_Handler) void MemManage_Handler             (void);
__ALIAS(Fault_Handler) void BusFault_Handler              (void);
__ALIAS(Fault_Handler) void UsageFault_Handler            (void);
__ALIAS(Fault_Handler) void SVC_Handler                   (void);
__ALIAS(Fault_Handler) void DebugMon_Handler              (void);
__ALIAS(Fault_Handler) void PendSV_Handler                (void);
__ALIAS(Fault_Handler) void SysTick_Handler               (void);

/* External interrupt handlers */
__ALIAS(Fault_Handler) void WWDG_IRQHandler               (void);
__ALIAS(Fault_Handler) void PVD_PVM_IRQHandler            (void);
__ALIAS(Fault_Handler) void TAMP_STAMP_IRQHandler         (void);
__ALIAS(Fault_Handler) void RTC_WKUP_IRQHandler           (void);
__ALIAS(Fault_Handler) void FLASH_IRQHandler              (void);
__ALIAS(Fault_Handler) void RCC_IRQHandler                (void);
__ALIAS(Fault_Handler) void EXTI0_IRQHandler              (void);
__ALIAS(Fault_Handler) void EXTI1_IRQHandler              (void);
__ALIAS(Fault_Handler) void EXTI2_IRQHandler              (void);
__ALIAS(Fault_Handler) void EXTI3_IRQHandler              (void);
__ALIAS(Fault_Handler) void EXTI4_IRQHandler              (void);
__ALIAS(Fault_Handler) void DMA1_Channel1_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA1_Channel2_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA1_Channel3_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA1_Channel4_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA1_Channel5_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA1_Channel6_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA1_Channel7_IRQHandler      (void);
__ALIAS(Fault_Handler) void ADC1_IRQHandler               (void);
__ALIAS(Fault_Handler) void CAN1_TX_IRQHandler            (void);
__ALIAS(Fault_Handler) void CAN1_RX0_IRQHandler           (void);
__ALIAS(Fault_Handler) void CAN1_RX1_IRQHandler           (void);
__ALIAS(Fault_Handler) void CAN1_SCE_IRQHandler           (void);
__ALIAS(Fault_Handler) void EXTI9_5_IRQHandler            (void);
__ALIAS(Fault_Handler) void TIM1_BRK_TIM15_IRQHandler     (void);
__ALIAS(Fault_Handler) void TIM1_UP_TIM16_IRQHandler      (void);
__ALIAS(Fault_Handler) void TIM1_TRG_COM_IRQHandler       (void);
__ALIAS(Fault_Handler) void TIM1_CC_IRQHandler            (void);
__ALIAS(Fault_Handler) void TIM2_IRQHandler               (void);
__ALIAS(Fault_Handler) void TIM3_IRQHandler               (void);
__ALIAS(Fault_Handler) void TIM4_IRQHandler               (void);
__ALIAS(Fault_Handler) void I2C1_EV_IRQHandler            (void);
__ALIAS(Fault_Handler) void I2C1_ER_IRQHandler            (void);
__ALIAS(Fault_Handler) void I2C2_EV_IRQHandler            (void);
__ALIAS(Fault_Handler) void I2C2_ER_IRQHandler            (void);
__ALIAS(Fault_Handler) void SPI1_IRQHandler               (void);
__ALIAS(Fault_Handler) void SPI2_IRQHandler               (void);
__ALIAS(Fault_Handler) void USART1_IRQHandler             (void);
__ALIAS(Fault_Handler) void USART2_IRQHandler             (void);
__ALIAS(Fault_Handler) void USART3_IRQHandler             (void);
__ALIAS(Fault_Handler) void EXTI15_10_IRQHandler          (void);
__ALIAS(Fault_Handler) void RTC_Alarm_IRQHandler          (void);
__ALIAS(Fault_Handler) void DFSDM1_FLT3_IRQHandler        (void);
__ALIAS(Fault_Handler) void TIM8_BRK_IRQHandler           (void);
__ALIAS(Fault_Handler) void TIM8_UP_IRQHandler            (void);
__ALIAS(Fault_Handler) void TIM8_TRG_COM_IRQHandler       (void);
__ALIAS(Fault_Handler) void TIM8_CC_IRQHandler            (void);
__ALIAS(Fault_Handler) void ADC3_IRQHandler               (void);
__ALIAS(Fault_Handler) void FMC_IRQHandler                (void);
__ALIAS(Fault_Handler) void SDMMC1_IRQHandler             (void);
__ALIAS(Fault_Handler) void TIM5_IRQHandler               (void);
__ALIAS(Fault_Handler) void SPI3_IRQHandler               (void);
__ALIAS(Fault_Handler) void UART4_IRQHandler              (void);
__ALIAS(Fault_Handler) void UART5_IRQHandler              (void);
__ALIAS(Fault_Handler) void TIM6_DAC_IRQHandler           (void);
__ALIAS(Fault_Handler) void TIM7_IRQHandler               (void);
__ALIAS(Fault_Handler) void DMA2_Channel1_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA2_Channel2_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA2_Channel3_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA2_Channel4_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA2_Channel5_IRQHandler      (void);
__ALIAS(Fault_Handler) void DFSDM1_FLT0_IRQHandler        (void);
__ALIAS(Fault_Handler) void DFSDM1_FLT1_IRQHandler        (void);
__ALIAS(Fault_Handler) void DFSDM1_FLT2_IRQHandler        (void);
__ALIAS(Fault_Handler) void COMP_IRQHandler               (void);
__ALIAS(Fault_Handler) void LPTIM1_IRQHandler             (void);
__ALIAS(Fault_Handler) void LPTIM2_IRQHandler             (void);
__ALIAS(Fault_Handler) void OTG_FS_IRQHandler             (void);
__ALIAS(Fault_Handler) void DMA2_Channel6_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA2_Channel7_IRQHandler      (void);
__ALIAS(Fault_Handler) void LPUART1_IRQHandler            (void);
__ALIAS(Fault_Handler) void QUADSPI_IRQHandler            (void);
__ALIAS(Fault_Handler) void I2C3_EV_IRQHandler            (void);
__ALIAS(Fault_Handler) void I2C3_ER_IRQHandler            (void);
__ALIAS(Fault_Handler) void SAI1_IRQHandler               (void);
__ALIAS(Fault_Handler) void SAI2_IRQHandler               (void);
__ALIAS(Fault_Handler) void SWPMI1_IRQHandler             (void);
__ALIAS(Fault_Handler) void TSC_IRQHandler                (void);
__ALIAS(Fault_Handler) void LCD_IRQHandler                (void);
__ALIAS(Fault_Handler) void RNG_IRQHandler                (void);
__ALIAS(Fault_Handler) void FPU_IRQHandler                (void);

/*******************************************************************************
 Vector table
*******************************************************************************/

void (* const __VECTOR_TABLE[])(void) __VECTOR_TABLE_ATTRIBUTE =
{
	/* Initial stack pointer */
	__CAST(__initial_msp),

	/* Core exception handlers */
	Reset_Handler,      /* Reset                                   */
	NMI_Handler,        /* Non-maskable interrupt                  */
	HardFault_Handler,  /* All classes of faults                   */
	MemManage_Handler,  /* Memory management                       */
	BusFault_Handler,   /* Pre-fetch fault, memory access fault    */
	UsageFault_Handler, /* Undefined instruction or illegal state  */
	0, 0, 0, 0,         /* Reserved                                */
	SVC_Handler,        /* System service call via SWI instruction */
	DebugMon_Handler,   /* Debug Monitor                           */
	0,                  /* Reserved                                */
	PendSV_Handler,     /* Pendable request for system service     */
	SysTick_Handler,    /* System tick timer                       */

#ifndef __NO_EXTERNAL_INTERRUPTS

	/* External interrupt handlers */
	WWDG_IRQHandler,
	PVD_PVM_IRQHandler,
	TAMP_STAMP_IRQHandler,
	RTC_WKUP_IRQHandler,
	FLASH_IRQHandler,
	RCC_IRQHandler,
	EXTI0_IRQHandler,
	EXTI1_IRQHandler,
	EXTI2_IRQHandler,
	EXTI3_IRQHandler,
	EXTI4_IRQHandler,
	DMA1_Channel1_IRQHandler,
	DMA1_Channel2_IRQHandler,
	DMA1_Channel3_IRQHandler,
	DMA1_Channel4_IRQHandler,
	DMA1_Channel5_IRQHandler,
	DMA1_Channel6_IRQHandler,
	DMA1_Channel7_IRQHandler,
	ADC1_IRQHandler,
	CAN1_TX_IRQHandler,
	CAN1_RX0_IRQHandler,
	CAN1_RX1_IRQHandler,
	CAN1_SCE_IRQHandler,
	EXTI9_5_IRQHandler,
	TIM1_BRK_TIM15_IRQHandler,
	TIM1_UP_TIM16_IRQHandler,
	TIM1_TRG_COM_IRQHandler,
	TIM1_CC_IRQHandler,
	TIM2_IRQHandler,
	TIM3_IRQHandler,
	TIM4_IRQHandler,
	I2C1_EV_IRQHandler,
	I2C1_ER_IRQHandler,
	I2C2_EV_IRQHandler,
	I2C2_ER_IRQHandler,
	SPI1_IRQHandler,
	SPI2_IRQHandler,
	USART1_IRQHandler,
	USART2_IRQHandler,
	USART3_IRQHandler,
	EXTI15_10_IRQHandler,
	RTC_Alarm_IRQHandler,
	DFSDM1_FLT3_IRQHandler,
	TIM8_BRK_IRQHandler,
	TIM8_UP_IRQHandler,
	TIM8_TRG_COM_IRQHandler,
	TIM8_CC_IRQHandler,
	ADC3_IRQHandler,
	FMC_IRQHandler,
	SDMMC1_IRQHandler,
	TIM5_IRQHandler,
	SPI3_IRQHandler,
	UART4_IRQHandler,
	UART5_IRQHandler,
	TIM6_DAC_IRQHandler,
	TIM7_IRQHandler,
	DMA2_Channel1_IRQHandler,
	DMA2_Channel2_IRQHandler,
	DMA2_Channel3_IRQHandler,
	DMA2_Channel4_IRQHandler,
	DMA2_Channel5_IRQHandler,
	DFSDM1_FLT0_IRQHandler,
	DFSDM1_FLT1_IRQHandler,
	DFSDM1_FLT2_IRQHandler,
	COMP_IRQHandler,
	LPTIM1_IRQHandler,
	LPTIM2_IRQHandler,
	OTG_FS_IRQHandler,
	DMA2_Channel6_IRQHandler,
	DMA2_Channel7_IRQHandler,
	LPUART1_IRQHandler,
	QUADSPI_IRQHandler,
	I2C3_EV_IRQHandler,
	I2C3_ER_IRQHandler,
	SAI1_IRQHandler,
	SAI2_IRQHandler,
	SWPMI1_IRQHandler,
	TSC_IRQHandler,
	LCD_IRQHandler,
	0,
	RNG_IRQHandler,
	FPU_IRQHandler,

#endif//__NO_EXTERNAL_INTERRUPTS
};

/******************************************************************************/
