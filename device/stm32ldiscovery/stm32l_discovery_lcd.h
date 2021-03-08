/******************************************************************************
 * @file    stm32l_discovery_lcd.h
 * @author  Rajmund Szymanski
 * @date    05.01.2021
 * @brief   This file contains definitions for STM32L-Discovery Kit.
 ******************************************************************************/

#ifndef __STM32L_DISCOVERY_LCD_H
#define __STM32L_DISCOVERY_LCD_H

#include <stdbool.h>
#include "stm32l1_io.h"

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

void LCD_Init  ( void );
void LCD_Clear ( void );
void LCD_Put   ( int, int, bool, bool );
void LCD_Print ( char* );
void LCD_Bar   ( unsigned );
void LCD_Tick  ( void );

/* -------------------------------------------------------------------------- */

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__STM32L_DISCOVERY_LCD_H
