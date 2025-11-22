/******************************************************************************

    @file    IntrOS: oskernel.c
    @author  Rajmund Szymanski
    @date    15.03.2023
    @brief   This file provides set of variables and functions for IntrOS.

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
#include "inc/ostimer.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
// SYSTEM TIMER SERVICES
/* -------------------------------------------------------------------------- */

void core_tmr_insert( tmr_t *tmr )
{
	tsk_t *nxt = System.cur;
	tsk_t *prv = nxt->hdr.prev;

	tmr->hdr.id = ID_TIMER;

	tmr->hdr.prev = prv;
	tmr->hdr.next = nxt;
	nxt->hdr.prev = tmr;
	prv->hdr.next = tmr;
}

/* -------------------------------------------------------------------------- */

void core_tmr_remove( tmr_t *tmr )
{
	tsk_t *prv = tmr->hdr.prev;
	tsk_t *nxt = tmr->hdr.next;

	tmr->hdr.id = ID_STOPPED;

	nxt->hdr.prev = prv;
	prv->hdr.next = nxt;
}

/* -------------------------------------------------------------------------- */
// SYSTEM TASK SERVICES
/* -------------------------------------------------------------------------- */

void core_tsk_insert( tsk_t *tsk )
{
	tsk_t *nxt = System.cur;
	tsk_t *prv = nxt->hdr.prev;

	tsk->hdr.id = ID_READY;

	tsk->hdr.prev = prv;
	tsk->hdr.next = nxt;
	nxt->hdr.prev = tsk;
	prv->hdr.next = tsk;
}

/* -------------------------------------------------------------------------- */

void core_tsk_remove( tsk_t *tsk )
{
	tsk_t *prv = tsk->hdr.prev;
	tsk_t *nxt = tsk->hdr.next;

	tsk->hdr.id = ID_STOPPED;

	nxt->hdr.prev = prv;
	prv->hdr.next = nxt;

	if (tsk == System.tsk)
		System.tsk = NULL;

	if (tsk == System.cur)
		core_ctx_switch();
}

/* -------------------------------------------------------------------------- */

void core_tsk_append( tsk_t **que, tsk_t *tsk )
{
	tsk->guard = que;

	while (*que)
		que = &(*que)->obj.queue;

	tsk->obj.queue = NULL;
	*que = tsk;
}

/* -------------------------------------------------------------------------- */

void core_tsk_wakeup( tsk_t *tsk, int event )
{
	tsk_t **que = tsk->guard;

	if (que)
	{
		tsk->delay = 0;
		tsk->guard = 0;
		tsk->event = event;
		while (*que != tsk)
			que = &(*que)->obj.queue;
		*que = tsk->obj.queue;
	}
}

/* -------------------------------------------------------------------------- */

int core_tsk_wait( tsk_t **que, tsk_t *tsk )
{
	if (que)
		core_tsk_append(que, tsk);

	if (tsk == System.cur)
		core_ctx_switch();

	return tsk->event;
}

/* -------------------------------------------------------------------------- */

int core_tsk_waitFor( tsk_t **que, cnt_t delay )
{
	tsk_t *cur = System.cur;

	cur->start = core_sys_time();
	cur->delay = delay;

	return core_tsk_wait(que, cur);
}

/* -------------------------------------------------------------------------- */

int core_tsk_waitNext( tsk_t **que, cnt_t delay )
{
	tsk_t *cur = System.cur;

	cur->delay = delay;

	return core_tsk_wait(que, cur);
}

/* -------------------------------------------------------------------------- */

int core_tsk_waitUntil( tsk_t **que, cnt_t time )
{
	tsk_t *cur = System.cur;

	cur->start = core_sys_time();
	cur->delay = time - cur->start;
	if (cur->delay - 1U > CNT_LIMIT)
		cur->delay = 0;

	return core_tsk_wait(que, cur);
}

/* -------------------------------------------------------------------------- */

tsk_t *core_one_wakeup( tsk_t **que, int event )
{
	tsk_t *tsk = *que;

	if (tsk)
	{
		tsk->guard = 0;
		tsk->delay = 0;
		tsk->event = event;
		*que = tsk->obj.queue;
	}

	return tsk;
}

/* -------------------------------------------------------------------------- */

unsigned core_num_wakeup( tsk_t **que, int event, unsigned num )
{
	unsigned cnt = 0;

	while (num > 0 && core_one_wakeup(que, event)) { cnt++; num--; }

	return cnt;
}

/* -------------------------------------------------------------------------- */

void core_all_wakeup( tsk_t **que, int event )
{
	while (core_one_wakeup(que, event));
}

/* -------------------------------------------------------------------------- */

unsigned core_tsk_count( tsk_t **que )
{
	unsigned cnt = 0;

	while (*que) { que = &(*que)->obj.queue; cnt++; }

	return cnt;
}

/* -------------------------------------------------------------------------- */

void core_ctx_init( tsk_t *tsk )
{
	assert(tsk->size>STK_OVER(0));
#ifdef DEBUG
	if (tsk != System.cur)
		memset(tsk->stack, 0xFF, tsk->size);
#endif
#if OS_TASK_EXIT == 0
	port_ctx_init(&tsk->ctx.reg, (stk_t *)STK_CROP(tsk->stack, tsk->size), core_tsk_loop);
#else
	port_ctx_init(&tsk->ctx.reg, (stk_t *)STK_CROP(tsk->stack, tsk->size), core_tsk_exec);
#endif
}

/* -------------------------------------------------------------------------- */
#ifdef DEBUG

size_t core_stk_space( tsk_t *tsk )
{
	void *stk = tsk->stack;
	uint8_t *ptr = stk;
	while (*ptr == 0xFF) ptr++;
	return (uintptr_t)ptr - (uintptr_t)stk;
}

static
bool priv_stk_integrity( tsk_t *tsk, void *sp )
{
	void *tp = tsk->stack + STK_SIZE(0);
	if (tsk == &MAIN) return true;
	if (sp < tp) return false;
#if (__MPU_USED == 0) && ((OS_GUARD_SIZE) > 0)
	if (core_stk_space(tsk) < STK_OVER(0)) return false;
#endif
	return true;
}

bool core_ctx_integrity( tsk_t *tsk )
{
	void *sp = tsk->ctx.reg.sp;
	return priv_stk_integrity(tsk, sp);
}

bool core_stk_integrity( void )
{
	tsk_t *tsk = System.cur;
	void *sp = port_get_sp();
	return priv_stk_integrity(tsk, sp);
}

#endif
/* -------------------------------------------------------------------------- */

void core_ctx_switch( void )
{
	sys_lock();
	{
		if (setjmp(System.cur->ctx.buf) == 0)
			core_tsk_switch();
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */

void core_tsk_loop( void )
{
	for (;;)
	{
		port_clr_lock();
		((fun_a *)System.cur->proc)(System.cur->arg);
		core_ctx_switch();
	}
}

/* -------------------------------------------------------------------------- */
#if OS_TASK_EXIT

void core_tsk_exec( void )
{
	port_clr_lock();
	((fun_a *)System.cur->proc)(System.cur->arg);
	tsk_stop();
}

#endif
/* -------------------------------------------------------------------------- */

static
bool priv_tmr_countdown( tmr_t *tmr )
{
	bool countdown = true;

	port_set_lock();
	{
		if (core_sys_time() - tmr->start + 1 > tmr->delay)
		{
			tmr->start += tmr->delay;
			tmr->delay = tmr->period;
			countdown = false;
		}
	}
	port_clr_lock();

	return countdown;
}

/* -------------------------------------------------------------------------- */

void core_tsk_switch( void )
{
	tsk_t *cur;
	tmr_t *tmr;

	assert_ctx_integrity(System.cur);

	for (;;)
	{
		port_set_lock();
		{
			cur = System.cur = System.cur->hdr.next;
		}
		port_clr_lock();

		if (cur->hdr.id == ID_STOPPED)
			continue;
		else
		if (cur->delay && priv_tmr_countdown((tmr_t *)cur))
			continue;
		else
		if (cur->hdr.id == ID_READY)
		{
			port_set_lock();
			{
				core_tsk_wakeup(cur, E_FAILURE);

				if (System.tsk != NULL && System.tsk != cur)
					continue;
			}
			port_clr_lock();

			break;
		}
		else
		if (cur->hdr.id == ID_TIMER)
		{
			port_set_lock();
			{
				tmr = (tmr_t *)cur;

				if (tmr->proc)
					((fun_a *)tmr->proc)(tmr->arg);

				if (tmr->delay == 0)
					core_tmr_remove(tmr);

				core_all_wakeup(&tmr->obj.queue, E_SUCCESS);
			}
			port_clr_lock();

			break;
		}
	}

#if __MPU_USED == 1
//	port_mpu_disable();
	port_mpu_stackUpdate(cur == &MAIN ? NULL : cur->stack);
//	port_mpu_enable();
#endif

	assert_ctx_integrity(System.cur);

	longjmp(cur->ctx.buf, 1);
}

/* -------------------------------------------------------------------------- */
