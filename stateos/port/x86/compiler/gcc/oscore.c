/******************************************************************************

    @file    StateOS: oscore.c
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

#include "oskernel.h"
#include "inc/ostask.h"

/* -------------------------------------------------------------------------- */

#define __STR(x) #x
#define _STR(x) __STR(x)
#define SYM(x) _STR(__USER_LABEL_PREFIX__) #x

/* -------------------------------------------------------------------------- */
// switch the context of the current task: called by port_ctx_swap

void *port_tsk_switch( void *sp );

void *port_tsk_switch( void *sp )
{
	port_isr = true;
	sp = core_tsk_switch(sp);
	port_isr = false;

#if defined(_WIN32)
	// update the stack limits of the current task in the thread information block
	// (required by the structured exception handling / stack unwinding)
	{
		static
		void *base = NULL;
		static
		void *limit = NULL;
		tsk_t *cur = System.cur;
		void *tb, *tl;

		if (base == NULL)
		#ifdef __x86_64__
			__ASM volatile ("mov %%gs:0x08, %0; mov %%gs:0x10, %1" : "=r" (base), "=r" (limit));
		#else
			__ASM volatile ("mov %%fs:0x04, %0; mov %%fs:0x08, %1" : "=r" (base), "=r" (limit));
		#endif

		if (cur == &MAIN)
		{
			tb = base;
			tl = limit;
		}
		else
		{
			tb = (uint8_t *)cur->stack + cur->size;
			tl = cur->stack;
		}
		#ifdef __x86_64__
		__ASM volatile ("mov %0, %%gs:0x08; mov %1, %%gs:0x10" :: "r" (tb), "r" (tl) : "memory");
		#else
		__ASM volatile ("mov %0, %%fs:0x04; mov %1, %%fs:0x08" :: "r" (tb), "r" (tl) : "memory");
		#endif
	}
#endif

	return sp;
}

/* -------------------------------------------------------------------------- */

#if   defined(__x86_64__) && defined(_WIN64)

__ASM
(
"	.text                            \n"
"	.p2align 4                       \n"
"	.globl " SYM(port_ctx_swap) "    \n"
SYM(port_ctx_swap) ":                \n"
"	push  %rbp                       \n"
"	push  %rbx                       \n"
"	push  %rdi                       \n"
"	push  %rsi                       \n"
"	push  %r12                       \n"
"	push  %r13                       \n"
"	push  %r14                       \n"
"	push  %r15                       \n"
"	sub   $160,  %rsp                \n"
"	movups %xmm6,    0(%rsp)         \n"
"	movups %xmm7,   16(%rsp)         \n"
"	movups %xmm8,   32(%rsp)         \n"
"	movups %xmm9,   48(%rsp)         \n"
"	movups %xmm10,  64(%rsp)         \n"
"	movups %xmm11,  80(%rsp)         \n"
"	movups %xmm12,  96(%rsp)         \n"
"	movups %xmm13, 112(%rsp)         \n"
"	movups %xmm14, 128(%rsp)         \n"
"	movups %xmm15, 144(%rsp)         \n"
"	mov   %rsp,  %rcx                \n"
"	and   $-16,  %rsp                \n"
"	sub   $32,   %rsp                \n"
"	call " SYM(port_tsk_switch) "    \n"
"	mov   %rax,  %rsp                \n"
"	movups    0(%rsp), %xmm6         \n"
"	movups   16(%rsp), %xmm7         \n"
"	movups   32(%rsp), %xmm8         \n"
"	movups   48(%rsp), %xmm9         \n"
"	movups   64(%rsp), %xmm10        \n"
"	movups   80(%rsp), %xmm11        \n"
"	movups   96(%rsp), %xmm12        \n"
"	movups  112(%rsp), %xmm13        \n"
"	movups  128(%rsp), %xmm14        \n"
"	movups  144(%rsp), %xmm15        \n"
"	add   $160,  %rsp                \n"
"	pop   %r15                       \n"
"	pop   %r14                       \n"
"	pop   %r13                       \n"
"	pop   %r12                       \n"
"	pop   %rsi                       \n"
"	pop   %rdi                       \n"
"	pop   %rbx                       \n"
"	pop   %rbp                       \n"
"	ret                              \n"

"	.p2align 4                       \n"
"	.globl " SYM(port_ctx_start) "   \n"
SYM(port_ctx_start) ":               \n"
"	and   $-16,  %rsp                \n"
"	sub   $32,   %rsp                \n"
"	call *%rbx                       \n"
"	ud2                              \n"
);

#elif defined(__x86_64__)

__ASM
(
"	.text                            \n"
"	.p2align 4                       \n"
"	.globl " SYM(port_ctx_swap) "    \n"
SYM(port_ctx_swap) ":                \n"
"	push  %rbp                       \n"
"	push  %rbx                       \n"
"	push  %r12                       \n"
"	push  %r13                       \n"
"	push  %r14                       \n"
"	push  %r15                       \n"
"	mov   %rsp,  %rdi                \n"
"	and   $-16,  %rsp                \n"
"	call " SYM(port_tsk_switch) "    \n"
"	mov   %rax,  %rsp                \n"
"	pop   %r15                       \n"
"	pop   %r14                       \n"
"	pop   %r13                       \n"
"	pop   %r12                       \n"
"	pop   %rbx                       \n"
"	pop   %rbp                       \n"
"	ret                              \n"

"	.p2align 4                       \n"
"	.globl " SYM(port_ctx_start) "   \n"
SYM(port_ctx_start) ":               \n"
"	.cfi_startproc                   \n"
"	.cfi_undefined %rip              \n"
"	and   $-16,  %rsp                \n"
"	call *%rbx                       \n"
"	ud2                              \n"
"	.cfi_endproc                     \n"
);

#else

__ASM
(
"	.text                            \n"
"	.p2align 4                       \n"
"	.globl " SYM(port_ctx_swap) "    \n"
SYM(port_ctx_swap) ":                \n"
"	push  %ebp                       \n"
"	push  %ebx                       \n"
"	push  %esi                       \n"
"	push  %edi                       \n"
"	mov   %esp,  %eax                \n"
"	and   $-16,  %esp                \n"
"	sub   $12,   %esp                \n"
"	push  %eax                       \n"
"	call " SYM(port_tsk_switch) "    \n"
"	mov   %eax,  %esp                \n"
"	pop   %edi                       \n"
"	pop   %esi                       \n"
"	pop   %ebx                       \n"
"	pop   %ebp                       \n"
"	ret                              \n"

"	.p2align 4                       \n"
"	.globl " SYM(port_ctx_start) "   \n"
SYM(port_ctx_start) ":               \n"
"	and   $-16,  %esp                \n"
"	call *%ebx                       \n"
"	ud2                              \n"
);

#endif

/* -------------------------------------------------------------------------- */

void core_tsk_flip( void *sp )
{
#if OS_TASK_EXIT == 0
	fun_t *pc = core_tsk_loop;
#else
	fun_t *pc = core_tsk_exec;
#endif

#if   defined(__x86_64__) && defined(_WIN64)
	__ASM volatile ("mov %0, %%rsp; and $-16, %%rsp; sub $32, %%rsp; call *%1" :: "r" (sp), "r" (pc) : "memory");
#elif defined(__x86_64__)
	__ASM volatile ("mov %0, %%rsp; and $-16, %%rsp; call *%1" :: "r" (sp), "r" (pc) : "memory");
#else
	__ASM volatile ("mov %0, %%esp; and $-16, %%esp; call *%1" :: "r" (sp), "r" (pc) : "memory");
#endif
	__builtin_unreachable();
}

/* -------------------------------------------------------------------------- */
