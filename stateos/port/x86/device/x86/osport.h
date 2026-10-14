/******************************************************************************

    @file    StateOS: osport.h
    @author  Rajmund Szymanski
    @date    14.10.2026
    @brief   StateOS port file for X86.

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

#ifndef __STATEOSPORT_H
#define __STATEOSPORT_H

#include <stdint.h>
#include <stdbool.h>
#ifndef   NOCONFIG
#include "osconfig.h"
#endif
#include "osdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 Port for testing purposes: the system runs in a single thread of the host
 process (linux / windows, x86 / x64); the virtual interrupts (system timer,
 context switch) are handled whenever the interrupts are being unmasked
 (port_clr_lock, port_put_lock) and in the idle task; the time is taken from
 the monotonic clock of the host
 The interrupt handlers (timer callbacks) are executed on the stack of the
 current task, so the task stacks must be large enough for them and for the
 host library functions (see OS_STACK_SIZE)
 A task that does not call any system function is never interrupted
 The c++ wrapper is not available if the host libstdc++ supports threads
*******************************************************************************/

/* -------------------------------------------------------------------------- */

#ifdef  CPU_FREQUENCY
#error  osconfig.h: CPU_FREQUENCY value is independent of this software.
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_FREQUENCY
#define OS_FREQUENCY       1000 /* Hz */
#endif

#if     OS_FREQUENCY > 1000000000
#error  osconfig.h: Incorrect OS_FREQUENCY value!
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
#define HW_TIMER_SIZE OS_TIMER_SIZE /* bit size of hardware timer             */
#else
#define HW_TIMER_SIZE         0 /* os does not work in tick-less mode         */
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_ROBIN
#define OS_ROBIN              0 /* system works in cooperative mode           */
#endif

#if     OS_ROBIN > 0
#error  osconfig.h: Incorrect OS_ROBIN value! This port works only in cooperative mode.
#endif

/* -------------------------------------------------------------------------- */
// return current value of the host monotonic clock (in OS_FREQUENCY units)

uint64_t port_sys_clock( void );

/* -------------------------------------------------------------------------- */
// return current system time

#if HW_TIMER_SIZE >= OS_TIMER_SIZE

__STATIC_INLINE
uint64_t port_sys_time( void )
{
	return port_sys_clock();
}

#endif

/* -------------------------------------------------------------------------- */
// wait for the next virtual interrupt (idle task)

void port_sys_wait( void );

#ifndef __WFI
#define __WFI()               port_sys_wait()
#endif

/* -------------------------------------------------------------------------- */
// context switch request (virtual PendSV)

extern bool port_ctx_pending;

/* -------------------------------------------------------------------------- */
// force yield system control to the next process

__STATIC_INLINE
void port_ctx_switch( void )
{
	port_ctx_pending = true;
}

/* -------------------------------------------------------------------------- */
// reset context switch indicator

__STATIC_INLINE
void port_ctx_reset( void )
{
}

/* -------------------------------------------------------------------------- */
// clear time breakpoint

__STATIC_INLINE
void port_tmr_stop( void )
{
}

/* -------------------------------------------------------------------------- */
// set time breakpoint

__STATIC_INLINE
void port_tmr_start( uint64_t timeout )
{
	(void) timeout;
}

/* -------------------------------------------------------------------------- */
// force timer interrupt

__STATIC_INLINE
void port_tmr_force( void )
{
}

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#endif//__STATEOSPORT_H
