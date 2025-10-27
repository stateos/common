/******************************************************************************

    @file    StateOS: oscore.c
    @author  Rajmund Szymanski
    @date    24.11.2022
    @brief   StateOS port file for AVR8 uC.

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

#include "oskernel.h"

/* -------------------------------------------------------------------------- */
// force yield system control to the next process now

__attribute__((naked))
void port_ctx_switchNow( void )
{
	asm volatile
	(
#if defined(__AVR_3_BYTE_PC__) && __AVR_3_BYTE_PC__
		"pop  r0           \n"
#endif
		"pop  r31          \n"
		"pop  r30          \n"
#if defined(__AVR_3_BYTE_PC__) && __AVR_3_BYTE_PC__
		"push r0           \n"
#endif
		"push r31          \n"
		"push r30          \n"
		"in   r0, __SREG__ \n"
		"cli               \n"
		"push r0           \n"
		"push r29          \n"
		"push r28          \n"
#if defined(__AVR_TINY__)
		"push r19          \n"
		"push r18          \n"
#else
		"push r17          \n"
		"push r16          \n"
		"push r15          \n"
		"push r14          \n"
		"push r13          \n"
		"push r12          \n"
		"push r11          \n"
		"push r10          \n"
		"push r9           \n"
		"push r8           \n"
		"push r7           \n"
		"push r6           \n"
		"push r5           \n"
		"push r4           \n"
		"push r3           \n"
		"push r2           \n"
#endif
	);

#if HW_TIMER_SIZE
	core_tmr_handler();
#endif
	port_set_sp(core_tsk_switch(port_get_sp()));

	asm volatile
	(
#if !defined(__AVR_TINY__)
		"pop  r2           \n"
		"pop  r3           \n"
		"pop  r4           \n"
		"pop  r5           \n"
		"pop  r6           \n"
		"pop  r7           \n"
		"pop  r8           \n"
		"pop  r9           \n"
		"pop  r10          \n"
		"pop  r11          \n"
		"pop  r12          \n"
		"pop  r13          \n"
		"pop  r14          \n"
		"pop  r15          \n"
		"pop  r16          \n"
		"pop  r17          \n"
#else
		"pop  r18          \n"
		"pop  r19          \n"
#endif
		"pop  r28          \n"
		"pop  r29          \n"
		"pop  r0           \n"
		"out  __SREG__, r0 \n"
		"pop  r30          \n"
		"pop  r31          \n"
#if defined(__AVR_3_BYTE_PC__) && __AVR_3_BYTE_PC__
		"pop  r0           \n"
#endif
		"push r30          \n"
		"push r31          \n"
#if defined(__AVR_3_BYTE_PC__) && __AVR_3_BYTE_PC__
		"push r0           \n"
#endif
		"ret               \n"
	);
}

/* -------------------------------------------------------------------------- */

void core_tsk_flip( void *sp )
{
	port_set_lock();
	SP = (uint16_t)sp - 1;
#if OS_TASK_EXIT == 0
	core_tsk_loop();
#else
	core_tsk_exec();
#endif
}

/* -------------------------------------------------------------------------- */
