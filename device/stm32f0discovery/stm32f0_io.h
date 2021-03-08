/******************************************************************************
 * @file    stm32f0_io.h
 * @author  Rajmund Szymanski
 * @date    05.03.2021
 * @brief   This file contains macro definitions for the STM32F0XX GPIO ports.
 ******************************************************************************/

#ifndef __STM32F0_IO_H
#define __STM32F0_IO_H

#include "stm32f0xx.h"

/* -------------------------------------------------------------------------- */

#define __bitset(_,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,pA,pB,pC,pD,pE,pF,...) \
       (((1U<<(p0))|(1U<<(p1))|(1U<<(p2))|(1U<<(p3))|(1U<<(p4))|(1U<<(p5))|(1U<<(p6))|(1U<<(p7))|\
         (1U<<(p8))|(1U<<(p9))|(1U<<(pA))|(1U<<(pB))|(1U<<(pC))|(1U<<(pD))|(1U<<(pE))|(1U<<(pF)))&0xFFFFU)

#define __stretch(bits) \
      ((((bits)&0x0001UL)*0x0001UL)|(((bits)&0x0002UL)*0x0002UL)|(((bits)&0x0004UL)*0x0004UL)|(((bits)&0x0008UL)*0x0008UL)|\
       (((bits)&0x0010UL)*0x0010UL)|(((bits)&0x0020UL)*0x0020UL)|(((bits)&0x0040UL)*0x0040UL)|(((bits)&0x0080UL)*0x0080UL)|\
       (((bits)&0x0100UL)*0x0100UL)|(((bits)&0x0200UL)*0x0200UL)|(((bits)&0x0400UL)*0x0400UL)|(((bits)&0x0800UL)*0x0800UL)|\
       (((bits)&0x1000UL)*0x1000UL)|(((bits)&0x2000UL)*0x2000UL)|(((bits)&0x4000UL)*0x4000UL)|(((bits)&0x8000UL)*0x8000UL))

/* -------------------------------------------------------------------------- */

#define REG_SET_BITS(REG, VALUE) do (REG) = (REG) |  (VALUE); while (0)
#define REG_CLR_BITS(REG, VALUE) do (REG) = (REG) & ~(VALUE); while (0)
#define REG_REV_BITS(REG, VALUE) do (REG) = (REG) ^  (VALUE); while (0)

/* -------------------------------------------------------------------------- */
/*
struct __gpio_config
{
	unsigned af:     4; // 0
	unsigned pupd:   2; // 4
	unsigned ospeed: 2; // 6
	unsigned otype:  1; // 8
	unsigned mode:   2; // 9
	unsigned out:    1; // 11
};
*/
#define GPIO_OUT_Pos        (11U)
#define GPIO_OUT_Msk        (0x1UL << GPIO_OUT_Pos)
#define GPIO_OUT(cfg)       _FLD2VAL(GPIO_OUT, cfg)

#define GPIO_MODE_Pos       (9U)
#define GPIO_MODE_Msk       (0x3UL << GPIO_MODE_Pos)
#define GPIO_MODE(cfg)      _FLD2VAL(GPIO_MODE, cfg)

#define GPIO_OTYPE_Pos      (8U)
#define GPIO_OTYPE_Msk      (0x1UL << GPIO_OTYPE_Pos)
#define GPIO_OTYPE(cfg)     _FLD2VAL(GPIO_OTYPE, cfg)

#define GPIO_OSPEED_Pos     (6U)
#define GPIO_OSPEED_Msk     (0x3UL << GPIO_OSPEED_Pos)
#define GPIO_OSPEED(cfg)    _FLD2VAL(GPIO_OSPEED, cfg)

#define GPIO_PUPD_Pos       (4U)
#define GPIO_PUPD_Msk       (0x3UL << GPIO_PUPD_Pos)
#define GPIO_PUPD(cfg)      _FLD2VAL(GPIO_PUPD, cfg)

#define GPIO_AF_Pos         (0U)
#define GPIO_AF_Msk         (0xFUL << GPIO_AF_Pos)
#define GPIO_AF(cfg)        _FLD2VAL(GPIO_AF, cfg)

#define GPIO_PORT_Pos       (10U)
#define GPIO_PORT_Msk       (0x7UL << GPIO_PORT_Pos)
#define GPIO_PORT(gpio)    (_FLD2VAL(GPIO_PORT, gpio) + 17U)

#define GPIO_Reset          _VAL2FLD(GPIO_OUT, 0)  /* reset output pin */
#define GPIO_Set            _VAL2FLD(GPIO_OUT, 1)  /*   set output pin */

#define GPIO_Input          _VAL2FLD(GPIO_MODE, 0)
#define GPIO_Output         _VAL2FLD(GPIO_MODE, 1)
#define GPIO_Alternate      _VAL2FLD(GPIO_MODE, 2)
#define GPIO_Analog         _VAL2FLD(GPIO_MODE, 3)

#define GPIO_PushPull       _VAL2FLD(GPIO_OTYPE, 0)
#define GPIO_OpenDrain      _VAL2FLD(GPIO_OTYPE, 1)

#define GPIO_2MHz           _VAL2FLD(GPIO_OSPEED, 0)
#define GPIO_25MHz          _VAL2FLD(GPIO_OSPEED, 1)
#define GPIO_50MHz          _VAL2FLD(GPIO_OSPEED, 2)
#define GPIO_100MHz         _VAL2FLD(GPIO_OSPEED, 3)

#define GPIO_NoPull         _VAL2FLD(GPIO_PUPD, 0)
#define GPIO_PullUp         _VAL2FLD(GPIO_PUPD, 1)
#define GPIO_PullDown       _VAL2FLD(GPIO_PUPD, 2)

#define GPIO_AF0            _VAL2FLD(GPIO_AF, 0)
#define GPIO_AF1            _VAL2FLD(GPIO_AF, 1)
#define GPIO_AF2            _VAL2FLD(GPIO_AF, 2)
#define GPIO_AF3            _VAL2FLD(GPIO_AF, 3)
#define GPIO_AF4            _VAL2FLD(GPIO_AF, 4)
#define GPIO_AF5            _VAL2FLD(GPIO_AF, 5)
#define GPIO_AF6            _VAL2FLD(GPIO_AF, 6)
#define GPIO_AF7            _VAL2FLD(GPIO_AF, 7)
#define GPIO_AF8            _VAL2FLD(GPIO_AF, 8)
#define GPIO_AF9            _VAL2FLD(GPIO_AF, 9)
#define GPIO_AF10           _VAL2FLD(GPIO_AF,10)
#define GPIO_AF11           _VAL2FLD(GPIO_AF,11)
#define GPIO_AF12           _VAL2FLD(GPIO_AF,12)
#define GPIO_AF13           _VAL2FLD(GPIO_AF,13)
#define GPIO_AF14           _VAL2FLD(GPIO_AF,14)
#define GPIO_AF15           _VAL2FLD(GPIO_AF,15)

#ifndef USE_HAL_DRIVER

#define GPIO_AF0_CAN         GPIO_AF0   /* CAN Alternate Function mapping */
#define GPIO_AF0_CEC         GPIO_AF0   /* CEC Alternate Function mapping */
#define GPIO_AF0_CRS         GPIO_AF0   /* CRS Alternate Function mapping */
#define GPIO_AF0_EVENTOUT    GPIO_AF0   /* EVENTOUT Alternate Function mapping */
#define GPIO_AF0_IR          GPIO_AF0   /* IR Alternate Function mapping */
#define GPIO_AF0_MCO         GPIO_AF0   /* MCO Alternate Function mapping */
#define GPIO_AF0_SPI1        GPIO_AF0   /* SPI1/I2S1 Alternate Function mapping */
#define GPIO_AF0_SPI2        GPIO_AF0   /* SPI2/I2S2 Alternate Function mapping */
#define GPIO_AF0_SWCLK       GPIO_AF0   /* SWCLK Alternate Function mapping */
#define GPIO_AF0_SWDAT       GPIO_AF0   /* SWDAT Alternate Function mapping */
#define GPIO_AF0_SWDIO       GPIO_AF0   /* SWDIO Alternate Function mapping */
#define GPIO_AF0_TIM1        GPIO_AF0   /* TIM1 Alternate Function mapping */
#define GPIO_AF0_TIM3        GPIO_AF0   /* TIM3 Alternate Function mapping */
#define GPIO_AF0_TIM14       GPIO_AF0   /* TIM14 Alternate Function mapping */
#define GPIO_AF0_TIM15       GPIO_AF0   /* TIM15 Alternate Function mapping */
#define GPIO_AF0_TIM16       GPIO_AF0   /* TIM16 Alternate Function mapping */
#define GPIO_AF0_TIM17       GPIO_AF0   /* TIM17 Alternate Function mapping */
#define GPIO_AF0_TSC         GPIO_AF0   /* TSC Alternate Function mapping */
#define GPIO_AF0_USART1      GPIO_AF0   /* USART1 Alternate Function mapping */
#define GPIO_AF0_USART2      GPIO_AF0   /* USART2 Alternate Function mapping */
#define GPIO_AF0_USART3      GPIO_AF0   /* USART3 Alternate Function mapping */
#define GPIO_AF0_USART4      GPIO_AF0   /* USART4 Alternate Function mapping */
#define GPIO_AF0_USART8      GPIO_AF0   /* USART8 Alternate Function mapping */
#define GPIO_AF1_CEC         GPIO_AF1   /* CEC Alternate Function mapping */
#define GPIO_AF1_EVENTOUT    GPIO_AF1   /* EVENTOUT Alternate Function mapping */
#define GPIO_AF1_I2C1        GPIO_AF1   /* I2C1 Alternate Function mapping */
#define GPIO_AF1_I2C2        GPIO_AF1   /* I2C2 Alternate Function mapping */
#define GPIO_AF1_IR          GPIO_AF1   /* IR Alternate Function mapping */
#define GPIO_AF1_SPI1        GPIO_AF1   /* SPI1 Alternate Function mapping */
#define GPIO_AF1_SPI2        GPIO_AF1   /* SPI2 Alternate Function mapping */
#define GPIO_AF1_TIM3        GPIO_AF1   /* TIM3 Alternate Function mapping */
#define GPIO_AF1_TIM15       GPIO_AF1   /* TIM15 Alternate Function mapping */
#define GPIO_AF1_TSC         GPIO_AF1   /* TSC Alternate Function mapping */
#define GPIO_AF1_USART1      GPIO_AF1   /* USART1 Alternate Function mapping */
#define GPIO_AF1_USART2      GPIO_AF1   /* USART2 Alternate Function mapping */
#define GPIO_AF1_USART3      GPIO_AF1   /* USART4 Alternate Function mapping */
#define GPIO_AF1_USART4      GPIO_AF1   /* USART4 Alternate Function mapping */
#define GPIO_AF1_USART5      GPIO_AF1   /* USART5 Alternate Function mapping */
#define GPIO_AF1_USART6      GPIO_AF1   /* USART6 Alternate Function mapping */
#define GPIO_AF1_USART7      GPIO_AF1   /* USART7 Alternate Function mapping */
#define GPIO_AF1_USART8      GPIO_AF1   /* USART8 Alternate Function mapping */
#define GPIO_AF2_EVENTOUT    GPIO_AF2   /* EVENTOUT Alternate Function mapping */
#define GPIO_AF2_TIM1        GPIO_AF2   /* TIM1 Alternate Function mapping */
#define GPIO_AF2_TIM2        GPIO_AF2   /* TIM2 Alternate Function mapping */
#define GPIO_AF2_TIM16       GPIO_AF2   /* TIM16 Alternate Function mapping */
#define GPIO_AF2_TIM17       GPIO_AF2   /* TIM17 Alternate Function mapping */
#define GPIO_AF2_USART5      GPIO_AF2   /* USART5 Alternate Function mapping */
#define GPIO_AF2_USART6      GPIO_AF2   /* USART6 Alternate Function mapping */
#define GPIO_AF2_USART7      GPIO_AF2   /* USART7 Alternate Function mapping */
#define GPIO_AF2_USART8      GPIO_AF2   /* USART8 Alternate Function mapping */
#define GPIO_AF2_USB         GPIO_AF2   /* USB Alternate Function mapping */
#define GPIO_AF3_EVENTOUT    GPIO_AF3   /* EVENTOUT Alternate Function mapping */
#define GPIO_AF3_I2C1        GPIO_AF3   /* I2C1 Alternate Function mapping */
#define GPIO_AF3_TIM15       GPIO_AF3   /* TIM15 Alternate Function mapping */
#define GPIO_AF3_TSC         GPIO_AF3   /* TSC Alternate Function mapping */
#define GPIO_AF4_CAN         GPIO_AF4   /* CAN Alternate Function mapping */
#define GPIO_AF4_CRS         GPIO_AF4   /* CRS Alternate Function mapping */
#define GPIO_AF4_I2C1        GPIO_AF4   /* I2C1 Alternate Function mapping */
#define GPIO_AF4_TIM14       GPIO_AF4   /* TIM14 Alternate Function mapping */
#define GPIO_AF4_USART3      GPIO_AF4   /* USART3 Alternate Function mapping */
#define GPIO_AF4_USART4      GPIO_AF4   /* USART4 Alternate Function mapping */
#define GPIO_AF4_USART5      GPIO_AF4   /* USART5 Alternate Function mapping */
#define GPIO_AF5_I2C2        GPIO_AF5   /* I2C2 Alternate Function mapping */
#define GPIO_AF5_MCO         GPIO_AF5   /* MCO Alternate Function mapping */
#define GPIO_AF5_SPI2        GPIO_AF5   /* SPI2 Alternate Function mapping */
#define GPIO_AF5_TIM15       GPIO_AF5   /* TIM15 Alternate Function mapping */
#define GPIO_AF5_TIM16       GPIO_AF5   /* TIM16 Alternate Function mapping */
#define GPIO_AF5_TIM17       GPIO_AF5   /* TIM17 Alternate Function mapping */
#define GPIO_AF5_USART6      GPIO_AF5   /* USART6 Alternate Function mapping */
#define GPIO_AF5_USB         GPIO_AF5   /* USB Alternate Function mapping */
#define GPIO_AF6_EVENTOUT    GPIO_AF6   /* EVENTOUT Alternate Function mapping */
#define GPIO_AF7_COMP1       GPIO_AF7   /* COMP1 Alternate Function mapping */
#define GPIO_AF7_COMP2       GPIO_AF7   /* COMP2 Alternate Function mapping */

#endif//USE_HAL_DRIVER

#define GPIO_Pin_0          (1UL<< 0)
#define GPIO_Pin_1          (1UL<< 1)
#define GPIO_Pin_2          (1UL<< 2)
#define GPIO_Pin_3          (1UL<< 3)
#define GPIO_Pin_4          (1UL<< 4)
#define GPIO_Pin_5          (1UL<< 5)
#define GPIO_Pin_6          (1UL<< 6)
#define GPIO_Pin_7          (1UL<< 7)
#define GPIO_Pin_8          (1UL<< 8)
#define GPIO_Pin_9          (1UL<< 9)
#define GPIO_Pin_10         (1UL<<10)
#define GPIO_Pin_11         (1UL<<11)
#define GPIO_Pin_12         (1UL<<12)
#define GPIO_Pin_13         (1UL<<13)
#define GPIO_Pin_14         (1UL<<14)
#define GPIO_Pin_15         (1UL<<15)
#define GPIO_Pin_All       ((1UL<<16)-1UL)

#define GPIO_Pins(...)     __bitset(_,##__VA_ARGS__,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16)

/* -------------------------------------------------------------------------- */

#define GPIO_Input_NoPull                       (GPIO_Input | GPIO_NoPull)
#define GPIO_Input_PullUp                       (GPIO_Input | GPIO_PullUp)
#define GPIO_Input_PullDown                     (GPIO_Input | GPIO_PullDown)

#define GPIO_Output_2MHz                        (GPIO_Output | GPIO_2MHz)
#define GPIO_Output_25MHz                       (GPIO_Output | GPIO_25MHz)
#define GPIO_Output_50MHz                       (GPIO_Output | GPIO_50MHz)
#define GPIO_Output_100MHz                      (GPIO_Output | GPIO_100MHz)

#define GPIO_Output_PushPull                    (GPIO_Output | GPIO_PushPull)
#define GPIO_Output_PushPull_2MHz               (GPIO_Output | GPIO_PushPull | GPIO_2MHz)
#define GPIO_Output_PushPull_25MHz              (GPIO_Output | GPIO_PushPull | GPIO_25MHz)
#define GPIO_Output_PushPull_50MHz              (GPIO_Output | GPIO_PushPull | GPIO_50MHz)
#define GPIO_Output_PushPull_100MHz             (GPIO_Output | GPIO_PushPull | GPIO_100MHz)

#define GPIO_Output_OpenDrain                   (GPIO_Output | GPIO_OpenDrain)
#define GPIO_Output_OpenDrain_2MHz              (GPIO_Output | GPIO_OpenDrain | GPIO_2MHz)
#define GPIO_Output_OpenDrain_25MHz             (GPIO_Output | GPIO_OpenDrain | GPIO_25MHz)
#define GPIO_Output_OpenDrain_50MHz             (GPIO_Output | GPIO_OpenDrain | GPIO_50MHz)
#define GPIO_Output_OpenDrain_100MHz            (GPIO_Output | GPIO_OpenDrain | GPIO_100MHz)

#define GPIO_Alternate_2MHz                     (GPIO_Alternate | GPIO_2MHz)
#define GPIO_Alternate_25MHz                    (GPIO_Alternate | GPIO_25MHz)
#define GPIO_Alternate_50MHz                    (GPIO_Alternate | GPIO_50MHz)
#define GPIO_Alternate_100MHz                   (GPIO_Alternate | GPIO_100MHz)

#define GPIO_Alternate_PushPull                 (GPIO_Alternate | GPIO_PushPull)
#define GPIO_Alternate_PushPull_2MHz            (GPIO_Alternate | GPIO_PushPull | GPIO_2MHz)
#define GPIO_Alternate_PushPull_25MHz           (GPIO_Alternate | GPIO_PushPull | GPIO_25MHz)
#define GPIO_Alternate_PushPull_50MHz           (GPIO_Alternate | GPIO_PushPull | GPIO_50MHz)
#define GPIO_Alternate_PushPull_100MHz          (GPIO_Alternate | GPIO_PushPull | GPIO_100MHz)

#define GPIO_Alternate_OpenDrain                (GPIO_Alternate | GPIO_OpenDrain)
#define GPIO_Alternate_OpenDrain_2MHz           (GPIO_Alternate | GPIO_OpenDrain | GPIO_2MHz)
#define GPIO_Alternate_OpenDrain_25MHz          (GPIO_Alternate | GPIO_OpenDrain | GPIO_25MHz)
#define GPIO_Alternate_OpenDrain_50MHz          (GPIO_Alternate | GPIO_OpenDrain | GPIO_50MHz)
#define GPIO_Alternate_OpenDrain_100MHz         (GPIO_Alternate | GPIO_OpenDrain | GPIO_100MHz)

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

__STATIC_FORCEINLINE
void __pinini( GPIO_TypeDef *gpio, uint32_t pins, uint32_t cfg )
{
	uint32_t high;

	if (0UL != pins*GPIO_OUT(cfg))    gpio->BSRR = (pins & GPIO_Pin_All) * GPIO_OUT(cfg);
	if (0UL != pins*GPIO_OTYPE(cfg))  REG_SET_BITS(gpio->OTYPER,  pins * GPIO_OTYPE(cfg));

	pins = __stretch(pins);

	if (0UL != pins*GPIO_MODE(cfg))   REG_SET_BITS(gpio->MODER,   pins * GPIO_MODE(cfg));
	if (0UL != pins*GPIO_OSPEED(cfg)) REG_SET_BITS(gpio->OSPEEDR, pins * GPIO_OSPEED(cfg));
	if (0UL != pins*GPIO_PUPD(cfg))   REG_SET_BITS(gpio->PUPDR,   pins * GPIO_PUPD(cfg));

	high = __stretch(pins >> 16);
	pins = __stretch(pins);

	if (0UL != pins*GPIO_AF(cfg))     REG_SET_BITS(gpio->AFR[0],  pins * GPIO_AF(cfg));
	if (0UL != high*GPIO_AF(cfg))     REG_SET_BITS(gpio->AFR[1],  high * GPIO_AF(cfg));
}

/* -------------------------------------------------------------------------- */

__STATIC_FORCEINLINE
void __pincfg( GPIO_TypeDef *gpio, uint32_t pins, uint32_t cfg )
{
	uint32_t high;

	gpio->BSRR = (pins & GPIO_Pin_All) << (16 * GPIO_OUT(~cfg));

	MODIFY_REG(gpio->OTYPER,  pins * 0x1, pins * GPIO_OTYPE(cfg));

	pins = __stretch(pins);

	MODIFY_REG(gpio->MODER,   pins * 0x3, pins * GPIO_MODE(cfg));
	MODIFY_REG(gpio->OSPEEDR, pins * 0x3, pins * GPIO_OSPEED(cfg));
	MODIFY_REG(gpio->PUPDR,   pins * 0x3, pins * GPIO_PUPD(cfg));

	high = __stretch(pins>>16);
	pins = __stretch(pins);

	if (pins)
	MODIFY_REG(gpio->AFR[0],  pins * 0xF, pins * GPIO_AF(cfg));
	if (high)
	MODIFY_REG(gpio->AFR[1],  high * 0xF, high * GPIO_AF(cfg));
}

/* -------------------------------------------------------------------------- */

__STATIC_FORCEINLINE
void __pinlck( GPIO_TypeDef *gpio, uint32_t pins )
{
	gpio->LCKR    = pins | 0x00010000;
	gpio->LCKR    = pins;
	gpio->LCKR    = pins | 0x00010000;
	(void)
	gpio->LCKR;
}

/* -------------------------------------------------------------------------- */

__STATIC_FORCEINLINE
void GPIO_Init( GPIO_TypeDef *gpio, uint32_t pins, uint32_t cfg )
{
	REG_SET_BITS(RCC->AHBENR, 1UL << GPIO_PORT(gpio)); RCC->AHBENR;

	__pinini(gpio, pins, cfg);
}

/* -------------------------------------------------------------------------- */

static inline
void GPIO_Config( GPIO_TypeDef *gpio, uint32_t pins, uint32_t cfg )
{
	REG_SET_BITS(RCC->AHBENR, 1UL << GPIO_PORT(gpio)); RCC->AHBENR;

	__pincfg(gpio, pins, cfg);
}

/* -------------------------------------------------------------------------- */

__STATIC_FORCEINLINE
void GPIO_Lock( GPIO_TypeDef *gpio, uint32_t pins )
{
	__pinlck(gpio, pins);
}

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
}
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
namespace device {

/* -------------------------------------------------------------------------- */

template<const unsigned gpio>
class PortT
{
public:
	PortT( void ) { REG_SET_BITS(RCC->AHBENR, 1UL << GPIO_PORT(gpio)); RCC->AHBENR; }
	
	void init  ( const unsigned pins, const unsigned cfg ) { __pinini((GPIO_TypeDef *)gpio, pins, cfg); }
	void config( const unsigned pins, const unsigned cfg ) { __pincfg((GPIO_TypeDef *)gpio, pins, cfg); }
	void lock  ( const unsigned pins )                     { __pinlck((GPIO_TypeDef *)gpio, pins);      }
};

#ifdef  GPIOA_BASE
typedef PortT<GPIOA_BASE> PORTA;
#endif
#ifdef  GPIOB_BASE
typedef PortT<GPIOB_BASE> PORTB;
#endif
#ifdef  GPIOC_BASE
typedef PortT<GPIOC_BASE> PORTC;
#endif
#ifdef  GPIOD_BASE
typedef PortT<GPIOD_BASE> PORTD;
#endif
#ifdef  GPIOE_BASE
typedef PortT<GPIOE_BASE> PORTE;
#endif
#ifdef  GPIOF_BASE
typedef PortT<GPIOF_BASE> PORTF;
#endif
#ifdef  GPIOG_BASE
typedef PortT<GPIOG_BASE> PORTG;
#endif
#ifdef  GPIOH_BASE
typedef PortT<GPIOH_BASE> PORTH;
#endif
#ifdef  GPIOI_BASE
typedef PortT<GPIOI_BASE> PORTI;
#endif
#ifdef  GPIOK_BASE
typedef PortT<GPIOK_BASE> PORTK;
#endif

/* -------------------------------------------------------------------------- */

template<const unsigned gpio, const unsigned pin>
class PinT
{
public:
	PinT( void ) { REG_SET_BITS(RCC->AHBENR, 1UL << GPIO_PORT(gpio)); RCC->AHBENR; }

	void init  ( const unsigned cfg ) { __pinini((GPIO_TypeDef *)gpio, 1 << pin, cfg); }
	void config( const unsigned cfg ) { __pincfg((GPIO_TypeDef *)gpio, 1 << pin, cfg); }
	void lock  ( void )               { __pinlck((GPIO_TypeDef *)gpio, 1 << pin);      }
};

/* -------------------------------------------------------------------------- */

}     //  namespace
#endif//__cplusplus

#endif//__STM32F0_IO_H
