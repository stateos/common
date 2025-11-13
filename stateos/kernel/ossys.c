/******************************************************************************

    @file    StateOS: ossys.c
    @author  Rajmund Szymanski
    @date    28.07.2022
    @brief   This file provides set of variables and functions for StateOS.

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

#include "ossys.h"
#include "inc/ostimer.h"
#include "inc/ostask.h"
#include "inc/osonceflag.h"

/* -------------------------------------------------------------------------- */

#ifndef MAIN_TOP
static  stk_t     MAIN_STK[STK_SIZE(OS_STACK_SIZE)] __STKALIGN;
#define MAIN_TOP (MAIN_STK+STK_SIZE(OS_STACK_SIZE))
#endif

static  union  { stk_t STK[STK_SIZE(OS_IDLE_STACK)] __STKALIGN;
        struct { char  stk[STK_OVER(OS_IDLE_STACK)-sizeof(ctx_t)]; ctx_t ctx; } CTX; }
        IDLE_STACK = { .CTX = { .ctx = _CTX_INIT(core_tsk_loop) } };
#define IDLE_STK  IDLE_STACK.STK
#define IDLE_SP  &IDLE_STACK.CTX.ctx

/* -------------------------------------------------------------------------- */

tmr_t WAIT = { .hdr={ .prev=&WAIT, .next=&WAIT, .id=ID_TIMER }, .delay=INFINITE }; // timers queue
tsk_t MAIN = { .hdr={ .prev=&IDLE, .next=&IDLE, .id=ID_READY }, .stack=MAIN_TOP, .basic=OS_MAIN_PRIO, .prio=OS_MAIN_PRIO }; // main task
tsk_t IDLE = { .hdr={ .prev=&MAIN, .next=&MAIN, .id=ID_READY }, .proc=core_tsk_idle, .stack=IDLE_STK, .size=sizeof(IDLE_STK), .sp=IDLE_SP, .owner=&IDLE }; // idle task and tasks queue

sys_t System = { .cur=&MAIN };

/* -------------------------------------------------------------------------- */
void sys_init( void )
/* -------------------------------------------------------------------------- */
{
	static one_t init = ONE_INIT();

	one_call(&init, port_sys_init);
}

/* -------------------------------------------------------------------------- */
void sys_suspend( void )
/* -------------------------------------------------------------------------- */
{
	sys_lock();
	{
		System.tsk = System.cur;
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void sys_resume( void )
/* -------------------------------------------------------------------------- */
{
	sys_lock();
	{
		System.tsk = NULL;
		#if OS_ROBIN
		port_ctx_switchNow();
		#endif
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
