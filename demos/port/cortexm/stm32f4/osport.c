/******************************************************************************

    @file    DemOS: osport.c
    @author  Rajmund Szymanski
    @date    26.11.2025
    @brief   DemOS port file for STM32F4 uC.

 ******************************************************************************

   Copyright (c) 2018-2021 Rajmund Szymanski. All rights reserved.

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

#include "os.h"

/* --------------------------------------------------------------------------------------------- */
#ifndef USE_HAL_DRIVER

__attribute__((weak))
void sys_hook( void ); // user function - called from counter interrupt

#endif
/* --------------------------------------------------------------------------------------------- */
#ifndef USE_HAL_DRIVER

void SysTick_Handler( void )
{
	SysTick->CTRL;

	sys.counter++;
	sys_hook();
}

#endif
/* --------------------------------------------------------------------------------------------- */

void sys_init( void )
{
#ifndef USE_HAL_DRIVER
	SysTick_Config(SystemCoreClock/1000);
#endif
}

/* --------------------------------------------------------------------------------------------- */
#ifndef USE_HAL_DRIVER

cnt_t sys_time( void )
{
	cnt_t cnt;

	if (sizeof(cnt_t) > sizeof(uint32_t))
	{
		do cnt = sys.counter;
		while (cnt != sys.counter);
	}
	else
	{
		cnt = sys.counter;
	}

	return cnt;
}

#endif
/* --------------------------------------------------------------------------------------------- */
