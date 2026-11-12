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
// the virtual interrupts are masked (port_isr) until the switch is finished (see port_sys_poll, port_ctx_enter)

void *port_tsk_switch( void *sp )
{
	sp = core_tsk_switch(sp);

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
			__ASM volatile ("mov %%gs:0x08, %0; mov %%gs:0x10, %1" : "=r" (base), "=r" (limit));

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
		__ASM volatile ("mov %0, %%gs:0x08; mov %1, %%gs:0x10" :: "r" (tb), "r" (tl) : "memory");
	}
#endif

	return sp;
}

/* -------------------------------------------------------------------------- */
// the new task leaves the virtual interrupt handler: called by port_ctx_start
// (the pending interrupts are handled by port_clr_lock at the start of core_tsk_loop / core_tsk_exec)

void port_ctx_enter( void )
{
	port_isr = false;
}

/* -------------------------------------------------------------------------- */

#if defined(_WIN64)

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
"	sub   $168,  %rsp                \n"
"	stmxcsr      0(%rsp)             \n"
"	fnstcw       4(%rsp)             \n"
"	movups %xmm6,    8(%rsp)         \n"
"	movups %xmm7,   24(%rsp)         \n"
"	movups %xmm8,   40(%rsp)         \n"
"	movups %xmm9,   56(%rsp)         \n"
"	movups %xmm10,  72(%rsp)         \n"
"	movups %xmm11,  88(%rsp)         \n"
"	movups %xmm12, 104(%rsp)         \n"
"	movups %xmm13, 120(%rsp)         \n"
"	movups %xmm14, 136(%rsp)         \n"
"	movups %xmm15, 152(%rsp)         \n"
"	mov   %rsp,  %rcx                \n"
"	and   $-16,  %rsp                \n"
"	sub   $32,   %rsp                \n"
"	call " SYM(port_tsk_switch) "    \n"
"	mov   %rax,  %rsp                \n"
"	ldmxcsr      0(%rsp)             \n"
"	fldcw        4(%rsp)             \n"
"	movups    8(%rsp), %xmm6         \n"
"	movups   24(%rsp), %xmm7         \n"
"	movups   40(%rsp), %xmm8         \n"
"	movups   56(%rsp), %xmm9         \n"
"	movups   72(%rsp), %xmm10        \n"
"	movups   88(%rsp), %xmm11        \n"
"	movups  104(%rsp), %xmm12        \n"
"	movups  120(%rsp), %xmm13        \n"
"	movups  136(%rsp), %xmm14        \n"
"	movups  152(%rsp), %xmm15        \n"
"	add   $168,  %rsp                \n"
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
"	call " SYM(port_ctx_enter) "     \n"
"	call *%rbx                       \n"
"	ud2                              \n"
);

#else

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
"	sub   $8,    %rsp                \n"
"	stmxcsr      0(%rsp)             \n"
"	fnstcw       4(%rsp)             \n"
"	mov   %rsp,  %rdi                \n"
"	and   $-16,  %rsp                \n"
"	call " SYM(port_tsk_switch) "    \n"
"	mov   %rax,  %rsp                \n"
"	ldmxcsr      0(%rsp)             \n"
"	fldcw        4(%rsp)             \n"
"	add   $8,    %rsp                \n"
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
"	call " SYM(port_ctx_enter) "     \n"
"	call *%rbx                       \n"
"	ud2                              \n"
"	.cfi_endproc                     \n"
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

#if defined(_WIN64)
	__ASM volatile ("mov %0, %%rsp; and $-16, %%rsp; sub $32, %%rsp; call *%1" :: "r" (sp), "r" (pc) : "memory");
#else
	__ASM volatile ("mov %0, %%rsp; and $-16, %%rsp; call *%1" :: "r" (sp), "r" (pc) : "memory");
#endif
	__builtin_unreachable();
}

/* -------------------------------------------------------------------------- */
