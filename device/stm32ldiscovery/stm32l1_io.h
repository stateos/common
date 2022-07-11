/******************************************************************************
 * @file    stm32l1_io.h
 * @author  Rajmund Szymanski
 * @date    11.07.2022
 * @brief   This file contains macro definitions for the STM32L1XX GPIO ports.
 ******************************************************************************/

#ifndef __STM32L1_IO_H
#define __STM32L1_IO_H

#include "stm32l1xx.h"
#include "bitband.h"

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

#define REG_SET_BITS(REG, VALUE) do { (REG) = (REG) |  (VALUE);              } while (0)
#define REG_SET_WAIT(REG, VALUE) do { (REG) = (REG) |  (VALUE); (void)(REG); } while (0)
#define REG_CLR_BITS(REG, VALUE) do { (REG) = (REG) & ~(VALUE);              } while (0)
#define REG_REV_BITS(REG, VALUE) do { (REG) = (REG) ^  (VALUE);              } while (0)

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
#define GPIO_OUT_Pos         (11U)
#define GPIO_OUT_Msk         (0x1UL << GPIO_OUT_Pos)
#define GPIO_OUT_Val(cfg)    _FLD2VAL(GPIO_OUT, cfg)

#define GPIO_MODE_Pos        (9U)
#define GPIO_MODE_Msk        (0x3UL << GPIO_MODE_Pos)
#define GPIO_MODE_Val(cfg)   _FLD2VAL(GPIO_MODE, cfg)

#define GPIO_OTYPE_Pos       (8U)
#define GPIO_OTYPE_Msk       (0x1UL << GPIO_OTYPE_Pos)
#define GPIO_OTYPE_Val(cfg)  _FLD2VAL(GPIO_OTYPE, cfg)

#define GPIO_OSPEED_Pos      (6U)
#define GPIO_OSPEED_Msk      (0x3UL << GPIO_OSPEED_Pos)
#define GPIO_OSPEED_Val(cfg) _FLD2VAL(GPIO_OSPEED, cfg)

#define GPIO_PUPD_Pos        (4U)
#define GPIO_PUPD_Msk        (0x3UL << GPIO_PUPD_Pos)
#define GPIO_PUPD_Val(cfg)   _FLD2VAL(GPIO_PUPD, cfg)

#define GPIO_AF_Pos          (0U)
#define GPIO_AF_Msk          (0xFUL << GPIO_AF_Pos)
#define GPIO_AF_Val(cfg)     _FLD2VAL(GPIO_AF, cfg)

#define GPIO_PORT_Pos        (10U)
#define GPIO_PORT_Msk        (0xFUL << GPIO_PORT_Pos)
#define GPIO_PORT_Val(gpio)  _FLD2VAL(GPIO_PORT, gpio)

#define GPIO_Reset           _VAL2FLD(GPIO_OUT, 0)  /* reset output pin */
#define GPIO_Set             _VAL2FLD(GPIO_OUT, 1)  /*   set output pin */

#define GPIO_Input           _VAL2FLD(GPIO_MODE, 0)
#define GPIO_Output          _VAL2FLD(GPIO_MODE, 1)
#define GPIO_Alternate       _VAL2FLD(GPIO_MODE, 2)
#define GPIO_Analog          _VAL2FLD(GPIO_MODE, 3)

#define GPIO_PushPull        _VAL2FLD(GPIO_OTYPE, 0)
#define GPIO_OpenDrain       _VAL2FLD(GPIO_OTYPE, 1)

#define GPIO_LowSpeed        _VAL2FLD(GPIO_OSPEED, 0)
#define GPIO_MediumSpeed     _VAL2FLD(GPIO_OSPEED, 1)
#define GPIO_HighSpeed       _VAL2FLD(GPIO_OSPEED, 2)
#define GPIO_VeryHighSpeed   _VAL2FLD(GPIO_OSPEED, 3)

#define GPIO_NoPull          _VAL2FLD(GPIO_PUPD, 0)
#define GPIO_PullUp          _VAL2FLD(GPIO_PUPD, 1)
#define GPIO_PullDown        _VAL2FLD(GPIO_PUPD, 2)

#define GPIO_AF0             _VAL2FLD(GPIO_AF, 0)
#define GPIO_AF1             _VAL2FLD(GPIO_AF, 1)
#define GPIO_AF2             _VAL2FLD(GPIO_AF, 2)
#define GPIO_AF3             _VAL2FLD(GPIO_AF, 3)
#define GPIO_AF4             _VAL2FLD(GPIO_AF, 4)
#define GPIO_AF5             _VAL2FLD(GPIO_AF, 5)
#define GPIO_AF6             _VAL2FLD(GPIO_AF, 6)
#define GPIO_AF7             _VAL2FLD(GPIO_AF, 7)
#define GPIO_AF8             _VAL2FLD(GPIO_AF, 8)
#define GPIO_AF9             _VAL2FLD(GPIO_AF, 9)
#define GPIO_AF10            _VAL2FLD(GPIO_AF,10)
#define GPIO_AF11            _VAL2FLD(GPIO_AF,11)
#define GPIO_AF12            _VAL2FLD(GPIO_AF,12)
#define GPIO_AF13            _VAL2FLD(GPIO_AF,13)
#define GPIO_AF14            _VAL2FLD(GPIO_AF,14)
#define GPIO_AF15            _VAL2FLD(GPIO_AF,15)

#ifndef USE_HAL_DRIVER

#define GPIO_AF0_MCO         GPIO_AF0   /* MCO Alternate Function mapping */
#define GPIO_AF0_TAMPER      GPIO_AF0   /* TAMPER Alternate Function mapping */
#define GPIO_AF0_SWJ         GPIO_AF0   /* SWJ (SWD and JTAG) Alternate Function mapping */
#define GPIO_AF0_TRACE       GPIO_AF0   /* TRACE Alternate Function mapping */
#define GPIO_AF0_RTC_50Hz    GPIO_AF0   /* RTC_OUT Alternate Function mapping */
#define GPIO_AF1_TIM2        GPIO_AF1   /* TIM2 Alternate Function mapping */
#define GPIO_AF2_TIM3        GPIO_AF2   /* TIM3 Alternate Function mapping */
#define GPIO_AF2_TIM4        GPIO_AF2   /* TIM4 Alternate Function mapping */
#define GPIO_AF2_TIM5        GPIO_AF2   /* TIM5 Alternate Function mapping */
#define GPIO_AF3_TIM9        GPIO_AF3   /* TIM9 Alternate Function mapping */
#define GPIO_AF3_TIM10       GPIO_AF3   /* TIM10 Alternate Function mapping */
#define GPIO_AF3_TIM11       GPIO_AF3   /* TIM11 Alternate Function mapping */
#define GPIO_AF4_I2C1        GPIO_AF4   /* I2C1 Alternate Function mapping */
#define GPIO_AF4_I2C2        GPIO_AF4   /* I2C2 Alternate Function mapping */
#define GPIO_AF5_SPI1        GPIO_AF5   /* SPI1/I2S1 Alternate Function mapping */
#define GPIO_AF5_SPI2        GPIO_AF5   /* SPI2/I2S2 Alternate Function mapping */
#define GPIO_AF6_SPI3        GPIO_AF6   /* SPI3/I2S3 Alternate Function mapping */
#define GPIO_AF7_USART1      GPIO_AF7   /* USART1 Alternate Function mapping */
#define GPIO_AF7_USART2      GPIO_AF7   /* USART2 Alternate Function mapping */
#define GPIO_AF7_USART3      GPIO_AF7   /* USART3 Alternate Function mapping */
#define GPIO_AF8_UART4       GPIO_AF8   /* UART4 Alternate Function mapping */
#define GPIO_AF8_UART5       GPIO_AF8   /* UART5 Alternate Function mapping */
#define GPIO_AF11_LCD        GPIO_AF11  /* LCD Alternate Function mapping */
#define GPIO_AF12_FSMC       GPIO_AF12  /* FSMC Alternate Function mapping */
#define GPIO_AF12_SDIO       GPIO_AF12  /* SDIO Alternate Function mapping */
#define GPIO_AF14_TIM_IC1    GPIO_AF14  /* TIMER INPUT CAPTURE Alternate Function mapping */
#define GPIO_AF14_TIM_IC2    GPIO_AF14  /* TIMER INPUT CAPTURE Alternate Function mapping */
#define GPIO_AF14_TIM_IC3    GPIO_AF14  /* TIMER INPUT CAPTURE Alternate Function mapping */
#define GPIO_AF14_TIM_IC4    GPIO_AF14  /* TIMER INPUT CAPTURE Alternate Function mapping */
#define GPIO_AF15_EVENTOUT   GPIO_AF15  /* EVENTOUT Alternate Function mapping */

#endif//USE_HAL_DRIVER

#define GPIO_Pin_0           (1UL<< 0)
#define GPIO_Pin_1           (1UL<< 1)
#define GPIO_Pin_2           (1UL<< 2)
#define GPIO_Pin_3           (1UL<< 3)
#define GPIO_Pin_4           (1UL<< 4)
#define GPIO_Pin_5           (1UL<< 5)
#define GPIO_Pin_6           (1UL<< 6)
#define GPIO_Pin_7           (1UL<< 7)
#define GPIO_Pin_8           (1UL<< 8)
#define GPIO_Pin_9           (1UL<< 9)
#define GPIO_Pin_10          (1UL<<10)
#define GPIO_Pin_11          (1UL<<11)
#define GPIO_Pin_12          (1UL<<12)
#define GPIO_Pin_13          (1UL<<13)
#define GPIO_Pin_14          (1UL<<14)
#define GPIO_Pin_15          (1UL<<15)
#define GPIO_Pin_All        ((1UL<<16)-1UL)

#define GPIO_Pins(...)      __bitset(_,##__VA_ARGS__,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16)

/* -------------------------------------------------------------------------- */

#define GPIO_Input_NoPull                       (GPIO_Input | GPIO_NoPull)
#define GPIO_Input_PullUp                       (GPIO_Input | GPIO_PullUp)
#define GPIO_Input_PullDown                     (GPIO_Input | GPIO_PullDown)

#define GPIO_Output_LowSpeed                    (GPIO_Output | GPIO_LowSpeed)
#define GPIO_Output_MediumSpeed                 (GPIO_Output | GPIO_MediumSpeed)
#define GPIO_Output_HighSpeed                   (GPIO_Output | GPIO_HighSpeed)
#define GPIO_Output_VeryHighSpeed               (GPIO_Output | GPIO_VeryHighSpeed)

#define GPIO_Output_PushPull                    (GPIO_Output | GPIO_PushPull)
#define GPIO_Output_PushPull_LowSpeed           (GPIO_Output | GPIO_PushPull | GPIO_LowSpeed)
#define GPIO_Output_PushPull_MediumSpeed        (GPIO_Output | GPIO_PushPull | GPIO_MediumSpeed)
#define GPIO_Output_PushPull_HighSpeed          (GPIO_Output | GPIO_PushPull | GPIO_HighSpeed)
#define GPIO_Output_PushPull_VeryHighSpeed      (GPIO_Output | GPIO_PushPull | GPIO_VeryHighSpeed)

#define GPIO_Output_OpenDrain                   (GPIO_Output | GPIO_OpenDrain)
#define GPIO_Output_OpenDrain_LowSpeed          (GPIO_Output | GPIO_OpenDrain | GPIO_LowSpeed)
#define GPIO_Output_OpenDrain_MediumSpeed       (GPIO_Output | GPIO_OpenDrain | GPIO_MediumSpeed)
#define GPIO_Output_OpenDrain_HighSpeed         (GPIO_Output | GPIO_OpenDrain | GPIO_HighSpeed)
#define GPIO_Output_OpenDrain_VeryHighSpeed     (GPIO_Output | GPIO_OpenDrain | GPIO_VeryHighSpeed)

#define GPIO_Alternate_LowSpeed                 (GPIO_Alternate | GPIO_LowSpeed)
#define GPIO_Alternate_MediumSpeed              (GPIO_Alternate | GPIO_MediumSpeed)
#define GPIO_Alternate_HighSpeed                (GPIO_Alternate | GPIO_HighSpeed)
#define GPIO_Alternate_VeryHighSpeed            (GPIO_Alternate | GPIO_VeryHighSpeed)

#define GPIO_Alternate_PushPull                 (GPIO_Alternate | GPIO_PushPull)
#define GPIO_Alternate_PushPull_LowSpeed        (GPIO_Alternate | GPIO_PushPull | GPIO_LowSpeed)
#define GPIO_Alternate_PushPull_MediumSpeed     (GPIO_Alternate | GPIO_PushPull | GPIO_MediumSpeed)
#define GPIO_Alternate_PushPull_HighSpeed       (GPIO_Alternate | GPIO_PushPull | GPIO_HighSpeed)
#define GPIO_Alternate_PushPull_VeryHighSpeed   (GPIO_Alternate | GPIO_PushPull | GPIO_VeryHighSpeed)

#define GPIO_Alternate_OpenDrain                (GPIO_Alternate | GPIO_OpenDrain)
#define GPIO_Alternate_OpenDrain_LowSpeed       (GPIO_Alternate | GPIO_OpenDrain | GPIO_LowSpeed)
#define GPIO_Alternate_OpenDrain_MediumSpeed    (GPIO_Alternate | GPIO_OpenDrain | GPIO_MediumSpeed)
#define GPIO_Alternate_OpenDrain_HighSpeed      (GPIO_Alternate | GPIO_OpenDrain | GPIO_HighSpeed)
#define GPIO_Alternate_OpenDrain_VeryHighSpeed  (GPIO_Alternate | GPIO_OpenDrain | GPIO_VeryHighSpeed)

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

__STATIC_FORCEINLINE
void __pinini( GPIO_TypeDef *gpio, uint32_t pins, uint32_t cfg )
{
	uint32_t high;

	if (0UL != pins*GPIO_OUT_Val(cfg))    gpio->BSRR = (pins & GPIO_Pin_All) * GPIO_OUT_Val(cfg);
	if (0UL != pins*GPIO_OTYPE_Val(cfg))  REG_SET_BITS(gpio->OTYPER,  pins * GPIO_OTYPE_Val(cfg));

	pins = __stretch(pins);

	if (0UL != pins*GPIO_MODE_Val(cfg))   REG_SET_BITS(gpio->MODER,   pins * GPIO_MODE_Val(cfg));
	if (0UL != pins*GPIO_OSPEED_Val(cfg)) REG_SET_BITS(gpio->OSPEEDR, pins * GPIO_OSPEED_Val(cfg));
	if (0UL != pins*GPIO_PUPD_Val(cfg))   REG_SET_BITS(gpio->PUPDR,   pins * GPIO_PUPD_Val(cfg));

	high = __stretch(pins >> 16);
	pins = __stretch(pins);

	if (0UL != pins*GPIO_AF_Val(cfg))     REG_SET_BITS(gpio->AFR[0],  pins * GPIO_AF_Val(cfg));
	if (0UL != high*GPIO_AF_Val(cfg))     REG_SET_BITS(gpio->AFR[1],  high * GPIO_AF_Val(cfg));
}

/* -------------------------------------------------------------------------- */

__STATIC_FORCEINLINE
void __pincfg( GPIO_TypeDef *gpio, uint32_t pins, uint32_t cfg )
{
	uint32_t high;

	gpio->BSRR = (pins & GPIO_Pin_All) << (16 * GPIO_OUT_Val(~cfg));

	MODIFY_REG(gpio->OTYPER,  pins * 0x1, pins * GPIO_OTYPE_Val(cfg));

	pins = __stretch(pins);

	MODIFY_REG(gpio->MODER,   pins * 0x3, pins * GPIO_MODE_Val(cfg));
	MODIFY_REG(gpio->OSPEEDR, pins * 0x3, pins * GPIO_OSPEED_Val(cfg));
	MODIFY_REG(gpio->PUPDR,   pins * 0x3, pins * GPIO_PUPD_Val(cfg));

	high = __stretch(pins>>16);
	pins = __stretch(pins);

	if (pins)
	MODIFY_REG(gpio->AFR[0],  pins * 0xF, pins * GPIO_AF_Val(cfg));
	if (high)
	MODIFY_REG(gpio->AFR[1],  high * 0xF, high * GPIO_AF_Val(cfg));
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
	REG_SET_WAIT(RCC->AHBENR, 1UL << GPIO_PORT_Val(gpio));

	__pinini(gpio, pins, cfg);
}

/* -------------------------------------------------------------------------- */

static inline
void GPIO_Config( GPIO_TypeDef *gpio, uint32_t pins, uint32_t cfg )
{
	REG_SET_WAIT(RCC->AHBENR, 1UL << GPIO_PORT_Val(gpio));

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
	PortT( void ) { REG_SET_WAIT(RCC->AHBENR, 1UL << GPIO_PORT_Val(gpio)); }
	
	void init  ( const unsigned pins, const unsigned cfg ) { __pinini((GPIO_TypeDef *)gpio, pins, cfg); }
	void config( const unsigned pins, const unsigned cfg ) { __pincfg((GPIO_TypeDef *)gpio, pins, cfg); }
	void lock  ( const unsigned pins )                     { __pinlck((GPIO_TypeDef *)gpio, pins);      }

	unsigned   operator ()( const unsigned number ) { return BITBAND(((GPIO_TypeDef *)gpio)->IDR)[number]; }
	volatile
	unsigned & operator []( const unsigned number ) { return BITBAND(((GPIO_TypeDef *)gpio)->ODR)[number]; }
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
	PinT( void ) { REG_SET_WAIT(RCC->AHBENR, 1UL << GPIO_PORT_Val(gpio)); }

	void init  ( const unsigned cfg ) { __pinini((GPIO_TypeDef *)gpio, 1 << pin, cfg); }
	void config( const unsigned cfg ) { __pincfg((GPIO_TypeDef *)gpio, 1 << pin, cfg); }
	void lock  ( void )               { __pinlck((GPIO_TypeDef *)gpio, 1 << pin);      }

	unsigned operator ()( void )                 { return              BITBAND(((GPIO_TypeDef *)gpio)->IDR)[pin]; }
	operator unsigned & ( void )                 { return (unsigned &)(BITBAND(((GPIO_TypeDef *)gpio)->ODR)[pin]); }
	unsigned operator = ( const unsigned value ) { return             (BITBAND(((GPIO_TypeDef *)gpio)->ODR)[pin] = value), value; }
};

/* -------------------------------------------------------------------------- */

}     //  namespace
#endif//__cplusplus

#endif//__STM32L1_IO_H
