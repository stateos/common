/******************************************************************************

    @file    StateOS: oscore.h
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

#ifndef __STATEOSCORE_H
#define __STATEOSCORE_H

#include "osbase.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_HEAP_SIZE
#define OS_HEAP_SIZE          0 /* default system heap: host memory allocator */
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_STACK_SIZE
#define OS_STACK_SIZE     16384 /* default task stack size in bytes           */
#endif

#ifndef OS_IDLE_STACK
#define OS_IDLE_STACK     16384 /* idle task stack size in bytes              */
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

typedef unsigned              lck_t;
#ifdef __x86_64__
typedef uint64_t              stk_t;
#else
typedef uint32_t              stk_t;
#endif

/* -------------------------------------------------------------------------- */
// task context: callee-saved registers pushed onto the task stack by port_ctx_swap
// the task procedure is passed to the startup code (port_ctx_start) in rbx / ebx

typedef struct __ctx ctx_t;

#if   defined(__x86_64__) && defined(_WIN64)

struct __ctx
{
	uint64_t xmm[20]; // xmm6 - xmm15
	void   * r15;
	void   * r14;
	void   * r13;
	void   * r12;
	void   * rsi;
	void   * rdi;
	fun_t  * rbx;
	void   * rbp;
	fun_t  * pc;
};

#define _CTX_INIT( pc ) { { 0 }, 0, 0, 0, 0, 0, 0, pc, 0, port_ctx_start }

#elif defined(__x86_64__)

struct __ctx
{
	void   * r15;
	void   * r14;
	void   * r13;
	void   * r12;
	fun_t  * rbx;
	void   * rbp;
	fun_t  * pc;
};

#define _CTX_INIT( pc ) { 0, 0, 0, 0, pc, 0, port_ctx_start }

#elif defined(__i386__)

struct __ctx
{
	void   * edi;
	void   * esi;
	fun_t  * ebx;
	void   * ebp;
	fun_t  * pc;
};

#define _CTX_INIT( pc ) { 0, 0, pc, 0, port_ctx_start }

#else
#error  Unsupported architecture!
#endif

/* -------------------------------------------------------------------------- */
// startup code of the task (see oscore.c)

void port_ctx_start( void );

/* -------------------------------------------------------------------------- */
// save the context of the current task, switch to the next task and restore its context (see oscore.c)

void port_ctx_swap( void );

/* -------------------------------------------------------------------------- */
// init task context

__STATIC_INLINE
void port_ctx_init( ctx_t *ctx, fun_t *pc )
{
#ifdef __x86_64__
	ctx->rbx = pc;
#else
	ctx->ebx = pc;
#endif
	ctx->pc = port_ctx_start;
}

/* -------------------------------------------------------------------------- */
// state of the virtual interrupt controller (see osport.c)

extern lck_t port_lck; // interrupts are masked
extern bool  port_isr; // virtual interrupt handler (or context switch) is running

// handle pending virtual interrupts (system timer, context switch)
// called whenever the virtual interrupts are being unmasked
void port_sys_poll( void );

/* -------------------------------------------------------------------------- */
// is procedure inside ISR?

__STATIC_INLINE
bool port_isr_context( void )
{
	return port_isr;
}

/* -------------------------------------------------------------------------- */
// are interrupts masked?

__STATIC_INLINE
bool port_isr_masked( void )
{
	return port_lck != 0;
}

/* -------------------------------------------------------------------------- */
// get current stack pointer

__STATIC_FORCEINLINE
void * port_get_sp( void )
{
	void *sp;
#ifdef __x86_64__
	__ASM volatile ("mov %%rsp, %0" : "=r" (sp));
#else
	__ASM volatile ("mov %%esp, %0" : "=r" (sp));
#endif
	return sp;
}

/* -------------------------------------------------------------------------- */

__STATIC_INLINE
lck_t port_get_lock( void )
{
	return port_lck;
}

__STATIC_INLINE
void port_put_lock( lck_t lck )
{
	port_lck = lck;
	if (lck == 0 && !port_isr)
		port_sys_poll();
}

__STATIC_INLINE
void port_set_lock( void )
{
	port_lck = 1;
}

__STATIC_INLINE
void port_clr_lock( void )
{
	port_lck = 0;
	if (!port_isr)
		port_sys_poll();
}

/* -------------------------------------------------------------------------- */
// force yield system control to the next process now

__STATIC_INLINE
void port_ctx_switchNow( void )
{
	lck_t lck = port_get_lock();
	port_ctx_switch();
	port_clr_lock();
	port_put_lock(lck);
}

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif//__STATEOSCORE_H
