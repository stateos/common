/******************************************************************************

    @file    IntrOS: osport.h
    @author  Rajmund Szymanski
    @date    03.06.2020
    @brief   IntrOS port definitions for STM32F3 uC.

 ******************************************************************************

   Copyright (c) 2018-2022 Rajmund Szymanski. All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.

 ******************************************************************************/

#ifndef __INTROSPORT_H
#define __INTROSPORT_H

#include <stm32f3xx.h>
#ifndef   NOCONFIG
#include "osconfig.h"
#endif
#include "osdefs.h"
#include "osmpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef CPU_FREQUENCY
#define CPU_FREQUENCY  72000000 /* Hz */
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_FREQUENCY
#define OS_FREQUENCY       1000 /* Hz */
#endif

/* -------------------------------------------------------------------------- */
// !! WARNING! OS_TIMER_SIZE < HW_TIMER_SIZE may cause unexpected problems !!

#ifndef OS_TIMER_SIZE
#define OS_TIMER_SIZE        32 /* bit size of system timer counter           */
#endif

/* -------------------------------------------------------------------------- */
// !! WARNING! OS_TIMER_SIZE < HW_TIMER_SIZE may cause unexpected problems !!

#ifdef  HW_TIMER_SIZE
#error  HW_TIMER_SIZE is an internal os definition!
#elif   OS_FREQUENCY > 1000
#define HW_TIMER_SIZE        32 /* bit size of hardware timer                 */
#else
#define HW_TIMER_SIZE         0 /* os does not work in tick-less mode         */
#endif

/* -------------------------------------------------------------------------- */
// alternate clock source for SysTick

#ifdef  ST_FREQUENCY
#error  ST_FREQUENCY is an internal port definition!
#else
#define ST_FREQUENCY        ((CPU_FREQUENCY)/8)
#endif

/* -------------------------------------------------------------------------- */
// return current system time

#if HW_TIMER_SIZE >= OS_TIMER_SIZE

__STATIC_INLINE
uint32_t port_sys_time( void )
{
	return TIM2->CNT;
}

#endif

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#endif//__INTROSPORT_H
