/******************************************************************************

    @file    IntrOS: osport.c
    @author  Rajmund Szymanski
    @date    18.06.2020
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

#undef  _FORTIFY_SOURCE // __longjmp_chk does not allow to jump to another stack
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE // clock_gettime is not declared with -std=c11
#endif

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "oskernel.h"
#include "inc/ostask.h"

/* -------------------------------------------------------------------------- */

void port_sys_init( void )
{
}

/* -------------------------------------------------------------------------- */

#if defined(__linux__)

// clock() counts the processor time of the process on linux, the system time must not stop
uint64_t port_hw_time( void )
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000000U + (uint64_t)ts.tv_nsec / 1000U;
}

#elif defined(_WIN32)

// clock_t has 32 bits on windows, clock() stops after 2^31 milliseconds
uint64_t port_hw_time( void )
{
	static
	LARGE_INTEGER frq = { .QuadPart = 0 };
	LARGE_INTEGER cnt;

	if (frq.QuadPart == 0)
		QueryPerformanceFrequency(&frq);

	QueryPerformanceCounter(&cnt);

	return (uint64_t)(cnt.QuadPart / frq.QuadPart) * 1000U
	     + (uint64_t)(cnt.QuadPart % frq.QuadPart) * 1000U / (uint64_t)frq.QuadPart;
}

#endif

/* -------------------------------------------------------------------------- */

#if defined(__linux__) || defined(__x86_64__)

#undef  longjmp

/******************************************************************************
 Task context: the context of every new task is a copy of the trampoline
 context; the trampoline runs on its own small stack, switches to the stack
 of the current task and starts the task function
*******************************************************************************/

static jmp_buf priv_ctx_ret;
static jmp_buf priv_ctx_tmp;
static stk_t   priv_ctx_stk[1024];

#ifdef __x86_64__
#define priv_ctx_call(sp, fun) /* 16-byte stack alignment and 32-byte shadow space for win64 */ \
	__ASM volatile ("mov %0, %%rsp; and $-16, %%rsp; sub $32, %%rsp; call *%1" :: "r" (sp), "r" (fun) : "memory")
#else
#define priv_ctx_call(sp, fun) /* 16-byte stack alignment */ \
	__ASM volatile ("mov %0, %%esp; and $-16, %%esp; call *%1" :: "r" (sp), "r" (fun) : "memory")
#endif

static __NO_RETURN __attribute__((noinline, used))
void priv_ctx_start( void )
{
	if (setjmp(priv_ctx_tmp) == 0)
		longjmp(priv_ctx_ret, 1);

	// executed after longjmp to the context of the new task
	priv_ctx_call(System.cur->ctx.reg.sp, System.cur->ctx.reg.pc);
	__builtin_unreachable();
}

void port_ctx_init( ctx_t *ctx, stk_t *sp, fun_t *pc )
{
	static
	bool init = false;

	if (!init)
	{
		init = true;

		if (setjmp(priv_ctx_ret) == 0)
		{
			priv_ctx_call(priv_ctx_stk + sizeof(priv_ctx_stk) / sizeof(*priv_ctx_stk), priv_ctx_start);
			__builtin_unreachable();
		}
	}

	memcpy(ctx->buf, priv_ctx_tmp, sizeof(jmp_buf));
	ctx->sp = sp;
	ctx->pc = pc;
}

/* -------------------------------------------------------------------------- */

void port_longjmp( jmp_buf buf, int val )
{
#if defined(_WIN64)
	// update StackBase (gs:0x08) and StackLimit (gs:0x10) in the thread information block
	// the first longjmp is always called from the main thread stack
	static
	void * base = NULL;
	static
	void * limit = NULL;
	tsk_t *tsk = System.cur;

	if (base == NULL)
		__ASM volatile ("mov %%gs:0x08, %0; mov %%gs:0x10, %1" : "=r" (base), "=r" (limit));

	if (tsk == &MAIN)
		__ASM volatile ("mov %0, %%gs:0x08; mov %1, %%gs:0x10" :: "r" (base), "r" (limit) : "memory");
	else
		__ASM volatile ("mov %0, %%gs:0x08; mov %1, %%gs:0x10" :: "r" ((uint8_t *)tsk->stack + tsk->size), "r" (tsk->stack) : "memory");
#endif
	longjmp(buf, val);
}

#endif

/* -------------------------------------------------------------------------- */
