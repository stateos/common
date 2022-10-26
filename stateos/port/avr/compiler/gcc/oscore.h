/******************************************************************************

    @file    StateOS: oscore.h
    @author  Rajmund Szymanski
    @date    26.10.2022
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

#define PUSH_CONTEXT()         \
	asm volatile               \
	(                          \
		"push r0           \n" \
		"push r1           \n" \
		"push r2           \n" \
		"push r3           \n" \
		"push r4           \n" \
		"push r5           \n" \
		"push r6           \n" \
		"push r7           \n" \
		"push r8           \n" \
		"push r9           \n" \
		"push r10          \n" \
		"push r11          \n" \
		"push r12          \n" \
		"push r13          \n" \
		"push r14          \n" \
		"push r15          \n" \
		"push r16          \n" \
		"push r17          \n" \
		"push r18          \n" \
		"push r19          \n" \
		"push r20          \n" \
		"push r21          \n" \
		"push r22          \n" \
		"push r23          \n" \
		"push r24          \n" \
		"push r25          \n" \
		"push r26          \n" \
		"push r27          \n" \
		"push r28          \n" \
		"push r29          \n" \
		"push r30          \n" \
		"push r31          \n" \
		"in   r0, __SREG__ \n" \
		"eor  r1, r1       \n" \
		"push r0           \n" \
	)

/* -------------------------------------------------------------------------- */

#define POP_CONTEXT()          \
	asm volatile               \
	(                          \
		"pop  r0           \n" \
		"out  __SREG__, r0 \n" \
		"pop  r31          \n" \
		"pop  r30          \n" \
		"pop  r29          \n" \
		"pop  r28          \n" \
		"pop  r27          \n" \
		"pop  r26          \n" \
		"pop  r25          \n" \
		"pop  r24          \n" \
		"pop  r23          \n" \
		"pop  r22          \n" \
		"pop  r21          \n" \
		"pop  r20          \n" \
		"pop  r19          \n" \
		"pop  r18          \n" \
		"pop  r17          \n" \
		"pop  r16          \n" \
		"pop  r15          \n" \
		"pop  r14          \n" \
		"pop  r13          \n" \
		"pop  r12          \n" \
		"pop  r11          \n" \
		"pop  r10          \n" \
		"pop  r9           \n" \
		"pop  r8           \n" \
		"pop  r7           \n" \
		"pop  r6           \n" \
		"pop  r5           \n" \
		"pop  r4           \n" \
		"pop  r3           \n" \
		"pop  r2           \n" \
		"pop  r1           \n" \
		"pop  r0           \n" \
	)

/* -------------------------------------------------------------------------- */
// task context

typedef struct __ctx ctx_t;

struct __ctx
{
	uint8_t  dummy;    // sp is a pointer to the first free byte on the stack
	lck_t    sreg;     // status register      (SREG)
	uint8_t  r[32];    //                      (r0-r31)
#if	defined(__AVR_3_BYTE_PC__) && __AVR_3_BYTE_PC__
	uint8_t  pcx;      // return address       (PC)
#endif
	uint8_t  pch;      // return address       (PC)
	uint8_t  pcl;      // return address       (PC)
};

#if	defined(__AVR_3_BYTE_PC__) && __AVR_3_BYTE_PC__
#define _CTX_INIT( pc ) { 0, 0, { 0 }, 0, 0, 0 }
#else
#define _CTX_INIT( pc ) { 0, 0, { 0 }, 0, 0 }
#endif

/* -------------------------------------------------------------------------- */
// init task context

__STATIC_INLINE
void port_ctx_init( ctx_t *ctx, fun_t *pc )
{
	ctx->sreg = 0;
	ctx->pch = (uint8_t)((uintptr_t)pc >> 8);
	ctx->pcl = (uint8_t)((uintptr_t)pc);
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

#ifdef __cplusplus
}
#endif

#endif//__STATEOSCORE_H
