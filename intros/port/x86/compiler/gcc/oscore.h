/******************************************************************************

    @file    IntrOS: oscore.h
    @author  Rajmund Szymanski
    @date    28.12.2020
    @brief   IntrOS port file for X86.

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

#ifndef __INTROSCORE_H
#define __INTROSCORE_H

#include "osbase.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_STACK_SIZE
#define OS_STACK_SIZE     16384 /* default task stack size in bytes           */
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_LOCK_LEVEL
#define OS_LOCK_LEVEL         0 /* critical section blocks all interrupts     */
#endif

#if     OS_LOCK_LEVEL > 0
#error  osconfig.h: Incorrect OS_LOCK_LEVEL value! Must be 0.
#endif

/* -------------------------------------------------------------------------- */

#ifdef __x86_64__
typedef uint64_t              lck_t;
typedef uint64_t              stk_t;
#else
typedef uint32_t              lck_t;
typedef uint32_t              stk_t;
#endif

/* -------------------------------------------------------------------------- */
// task context

typedef struct __ctx ctx_t;

struct __ctx
{
#ifdef __x86_64
#error Unsupported cpu architecture!
#else
#ifdef __linux__
	unsigned long Ebx;
	unsigned long Esi;
	unsigned long Edi;
	unsigned long Ebp;
	void        * sp;
	fun_t       * pc;
#else
	unsigned long Ebp;
	unsigned long Ebx;
	unsigned long Edi;
	unsigned long Esi;
	void        * sp;
	fun_t       * pc;
#endif
#define _CTX_INIT() { 0, 0, 0, 0, NULL, NULL }
#endif
};

/* -------------------------------------------------------------------------- */
// init task context

__STATIC_INLINE
void port_ctx_init( ctx_t *ctx, stk_t *sp, fun_t *pc )
{
	setjmp((_JBTYPE *)ctx);

	ctx->sp = sp;
	ctx->pc = pc;
}

/* -------------------------------------------------------------------------- */
// get current stack pointer

__STATIC_FORCEINLINE
void * port_get_sp( void )
{
	void *sp;
#ifdef __x86_64__
	__ASM volatile ("mov %%rsp, %0" : "=&r" (sp));
#else
	__ASM volatile ("mov %%esp, %0" : "=&r" (sp));
#endif
	return sp;
}

/* -------------------------------------------------------------------------- */

__STATIC_FORCEINLINE
lck_t __get_cc( void )
{
	lck_t cc;
	__ASM volatile ("pushf; pop %0" : "=&r" (cc));
	return cc;
}

__STATIC_FORCEINLINE
void __set_cc( lck_t cc )
{
	__ASM volatile ("push %0; popf" :: "r" (cc) : "cc");
}

__STATIC_FORCEINLINE
void __cli( void )
{
	__ASM volatile ("cli" ::: "cc");
}

__STATIC_FORCEINLINE
void __sti( void )
{
	__ASM volatile ("sti" ::: "cc");
}

__STATIC_FORCEINLINE
void __nop( void )
{
	__ASM volatile ("nop" ::: "memory");
}

/* -------------------------------------------------------------------------- */

__STATIC_INLINE
lck_t port_get_lock( void )
{
	return 0;
}

__STATIC_INLINE
void port_put_lock( lck_t lck )
{
	(void) lck;
}

__STATIC_INLINE
void port_set_lock( void )
{
}

__STATIC_INLINE
void port_clr_lock( void )
{
}

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif//__INTROSCORE_H
