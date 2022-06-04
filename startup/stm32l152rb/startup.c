/*******************************************************************************
@file     startup_stm32l1xx.c
@author   Rajmund Szymanski
@date     04.06.2022
@brief    STM32L1xx startup file.
          After reset the Cortex-M3 processor is in thread mode,
          priority is privileged, and the stack is set to main.
*******************************************************************************/

#define __PROGRAM_START
#include <stm32l1xx.h>

/*******************************************************************************
 Specific definitions for the chip
*******************************************************************************/

#define   RAM_start 0x20000000
#define   RAM_end   0x20004000

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
void Fault_Handler()
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
__ALIAS(Fault_Handler) void PVD_IRQHandler                (void);
__ALIAS(Fault_Handler) void TAMPER_STAMP_IRQHandler       (void);
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
__ALIAS(Fault_Handler) void USB_HP_IRQHandler             (void);
__ALIAS(Fault_Handler) void USB_LP_IRQHandler             (void);
__ALIAS(Fault_Handler) void DAC_IRQHandler                (void);
__ALIAS(Fault_Handler) void COMP_IRQHandler               (void);
__ALIAS(Fault_Handler) void EXTI9_5_IRQHandler            (void);
__ALIAS(Fault_Handler) void LCD_IRQHandler                (void);
__ALIAS(Fault_Handler) void TIM9_IRQHandler               (void);
__ALIAS(Fault_Handler) void TIM10_IRQHandler              (void);
__ALIAS(Fault_Handler) void TIM11_IRQHandler              (void);
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
__ALIAS(Fault_Handler) void USB_FS_WKUP_IRQHandler        (void);
__ALIAS(Fault_Handler) void TIM6_IRQHandler               (void);
__ALIAS(Fault_Handler) void TIM7_IRQHandler               (void);
__ALIAS(Fault_Handler) void SDIO_IRQHandler               (void);
__ALIAS(Fault_Handler) void TIM5_IRQHandler               (void);
__ALIAS(Fault_Handler) void SPI3_IRQHandler               (void);
__ALIAS(Fault_Handler) void UART4_IRQHandler              (void);
__ALIAS(Fault_Handler) void UART5_IRQHandler              (void);
__ALIAS(Fault_Handler) void DMA2_Channel1_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA2_Channel2_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA2_Channel3_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA2_Channel4_IRQHandler      (void);
__ALIAS(Fault_Handler) void DMA2_Channel5_IRQHandler      (void);
__ALIAS(Fault_Handler) void AES_IRQHandler                (void);
__ALIAS(Fault_Handler) void COMP_ACQ_IRQHandler           (void);

/*******************************************************************************
 Vector table for STM32L1xx (Cortex-M3)
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
	PVD_IRQHandler,
	TAMPER_STAMP_IRQHandler,
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
	USB_HP_IRQHandler,
	USB_LP_IRQHandler,
	DAC_IRQHandler,
	COMP_IRQHandler,
	EXTI9_5_IRQHandler,
	LCD_IRQHandler,
	TIM9_IRQHandler,
	TIM10_IRQHandler,
	TIM11_IRQHandler,
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
	USB_FS_WKUP_IRQHandler,
	TIM6_IRQHandler,
	TIM7_IRQHandler,
#if defined(COMP)
	SDIO_IRQHandler,
	TIM5_IRQHandler,
	SPI3_IRQHandler,
	UART4_IRQHandler,
	UART5_IRQHandler,
	DMA2_Channel1_IRQHandler,
	DMA2_Channel2_IRQHandler,
	DMA2_Channel3_IRQHandler,
	DMA2_Channel4_IRQHandler,
	DMA2_Channel5_IRQHandler,
	AES_IRQHandler,
	COMP_ACQ_IRQHandler,
#endif

#endif//__NO_EXTERNAL_INTERRUPTS
};

/******************************************************************************/
