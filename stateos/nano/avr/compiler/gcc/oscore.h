/******************************************************************************

    @file    StateOS: oscore.h
    @author  Rajmund Szymanski
    @date    01.11.2022
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

#ifndef __STATEOSCORE_H
#define __STATEOSCORE_H

#include "osbase.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_HEAP_SIZE
#define OS_HEAP_SIZE          0 /* default system heap: all free memory       */
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_STACK_SIZE
#define OS_STACK_SIZE        64 /* default task stack size in bytes           */
#endif

#ifndef OS_IDLE_STACK
#define OS_IDLE_STACK        64 /* idle task stack size in bytes              */
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_LOCK_LEVEL
#define OS_LOCK_LEVEL         0 /* critical section blocks all interrupts     */
#endif

#if     OS_LOCK_LEVEL > 0
#error  osconfig.h: Incorrect OS_LOCK_LEVEL value! Must be 0.
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_MAIN_PRIO
#define OS_MAIN_PRIO          0 /* priority of main process                   */
#endif

/* -------------------------------------------------------------------------- */

typedef uint8_t               lck_t;
typedef uint8_t               stk_t;

/* -------------------------------------------------------------------------- */

extern  stk_t               __stack[];
#define MAIN_TOP            __stack+1

/* -------------------------------------------------------------------------- */
// task context

typedef struct __ctx ctx_t;

struct __ctx
{
	uint8_t  dummy;    // sp is a pointer to the first free byte on the stack
#if defined(__AVR_TINY__)
	uint8_t  regs[2];  // call-saved registers (r18-r19)
#else
	uint8_t  regs[16]; // call-saved registers (r2-r17)
#endif
	uint16_t fp;       // frame pointer        (r29:r28)
	lck_t    sreg;     // status register      (SREG)
	fun_t  * pc;       // return address       (PC)
#if	defined(__AVR_3_BYTE_PC__) && __AVR_3_BYTE_PC__
	uint8_t  pcx;      // high part of return address
#endif
};

#if	defined(__AVR_3_BYTE_PC__) && __AVR_3_BYTE_PC__
#define _CTX_INIT( pc ) { 0, { 0 }, 0, 0, pc, 0 }
#else
#define _CTX_INIT( pc ) { 0, { 0 }, 0, 0, pc }
#endif

/* -------------------------------------------------------------------------- */
// init task context

__STATIC_INLINE
void port_ctx_init( ctx_t *ctx, fun_t *pc )
{
	ctx->sreg = 0;
	ctx->pc = pc;
}

/* -------------------------------------------------------------------------- */
// is procedure inside ISR?

__STATIC_INLINE
bool port_isr_context( void )
{
	return false;
}

/* -------------------------------------------------------------------------- */
// get current stack pointer

__STATIC_INLINE
void * port_get_sp( void )
{
	return (void *) SP;
}

/* -------------------------------------------------------------------------- */
// set current stack pointer

__STATIC_INLINE
void port_set_sp( void *sp )
{
	SP = (uint16_t) sp;
}

/* -------------------------------------------------------------------------- */

__STATIC_INLINE
lck_t port_get_lock( void )
{
	return SREG;
}

__STATIC_INLINE
void port_put_lock( lck_t lck )
{
	SREG = lck;
}

__STATIC_INLINE
void port_set_lock( void )
{
	cli();
}

__STATIC_INLINE
void port_clr_lock( void )
{
	sei();
}

/* -------------------------------------------------------------------------- */
// force yield system control to the next process now

void port_ctx_switchNow( void );

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif//__STATEOSCORE_H
