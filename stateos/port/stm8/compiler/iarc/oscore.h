/******************************************************************************

    @file    StateOS: oscore.h
    @author  Rajmund Szymanski
    @date    25.06.2022
    @brief   StateOS port file for STM8 uC.

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
#define OS_STACK_SIZE       128 /* default task stack size in bytes           */
#endif

#ifndef OS_IDLE_STACK
#define OS_IDLE_STACK       128 /* idle task stack size in bytes              */
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

extern  stk_t                _stack[];
#define MAIN_TOP             _stack+1

/* -------------------------------------------------------------------------- */
// task context

typedef struct __ctx ctx_t;

struct __ctx
{
	char     dummy; // sp is a pointer to the first free byte on the stack
	// context saved by the software
	char     r[8];  // virtual registers: l0, l1
	// context saved by the hardware
	char     cc;
	char     a;
	unsigned x;
	unsigned y;
#if __CODE_MODEL__ == __SMALL_CODE_MODEL__
	char     far;   // hardware saves pc as a far pointer (3 bytes)
#endif
	fun_t  * pc;
};

#if __CODE_MODEL__ == __SMALL_CODE_MODEL__
#define _CTX_INIT( pc ) { 0, { 0 }, 0x20, 0, 0, 0, 0, (fun_t *) pc }
#else
#define _CTX_INIT( pc ) { 0, { 0 }, 0x20, 0, 0, 0,    (fun_t *) pc }
#endif

/* -------------------------------------------------------------------------- */
// init task context

__STATIC_INLINE
void port_ctx_init( ctx_t *ctx, fun_t *pc )
{
	ctx->cc = 0x20;
#if __CODE_MODEL__ == __SMALL_CODE_MODEL__
	ctx->far = 0;
#endif
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

void *_get_SP( void );

void  _set_SP( void *sp );

/* -------------------------------------------------------------------------- */
// get current stack pointer

__STATIC_INLINE
void * port_get_sp( void )
{
	return _get_SP();
}

/* -------------------------------------------------------------------------- */

__STATIC_INLINE
lck_t port_get_lock( void )
{
	return __get_interrupt_state();
}

__STATIC_INLINE
void port_put_lock( lck_t lck )
{
	__set_interrupt_state(lck);
}

__STATIC_INLINE
void port_set_lock( void )
{
	disableInterrupts();
}

__STATIC_INLINE
void port_clr_lock( void )
{
	enableInterrupts();
}

/* -------------------------------------------------------------------------- */

#define  PUSH_VREGS()   \
    __asm (              \
        "push  0x00  \n"  \
        "push  0x01  \n"  \
        "push  0x02  \n"  \
        "push  0x03  \n"  \
        "push  0x04  \n"  \
        "push  0x05  \n"  \
        "push  0x06  \n"  \
        "push  0x07  \n"  \
        "push  0x08  \n"  \
        "push  0x09  \n"  \
        "push  0x0A  \n"  \
        "push  0x0B  \n"  \
        "push  0x0C  \n"  \
        "push  0x0D  \n"  \
        "push  0x0E  \n"  \
        "push  0x0F  \n"  \
    )

#define  POP_VREGS()    \
    __asm (              \
        "pop   0x0F  \n"  \
        "pop   0x0E  \n"  \
        "pop   0x0D  \n"  \
        "pop   0x0C  \n"  \
        "pop   0x0B  \n"  \
        "pop   0x0A  \n"  \
        "pop   0x09  \n"  \
        "pop   0x08  \n"  \
        "pop   0x07  \n"  \
        "pop   0x06  \n"  \
        "pop   0x05  \n"  \
        "pop   0x04  \n"  \
        "pop   0x03  \n"  \
        "pop   0x02  \n"  \
        "pop   0x01  \n"  \
        "pop   0x00  \n"  \
    )

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif//__STATEOSCORE_H
