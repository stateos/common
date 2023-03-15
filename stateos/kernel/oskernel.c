/******************************************************************************

    @file    StateOS: oskernel.c
    @author  Rajmund Szymanski
    @date    15.03.2023
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

#include "oskernel.h"
#include "inc/osmutex.h"
#include "inc/ostimer.h"
#include "inc/ostask.h"

/* -------------------------------------------------------------------------- */
// SYSTEM TIMER SERVICES
/* -------------------------------------------------------------------------- */

static
void priv_tmr_insert( tmr_t *tmr )
{
	tmr_t *prv;
	tmr_t *nxt = &WAIT;

	if (tmr->delay != INFINITE)
		do nxt = nxt->hdr.next;
		while (nxt->delay < tmr->start + tmr->delay - nxt->start);

	tmr->hdr.id = ID_TIMER;

	prv = nxt->hdr.prev;

	tmr->hdr.prev = prv;
	tmr->hdr.next = nxt;
	nxt->hdr.prev = tmr;
	prv->hdr.next = tmr;
}

/* -------------------------------------------------------------------------- */

static
void priv_tmr_remove( tmr_t *tmr )
{
	tmr_t *prv = tmr->hdr.prev;
	tmr_t *nxt = tmr->hdr.next;

	tmr->hdr.id = ID_STOPPED;

	nxt->hdr.prev = prv;
	prv->hdr.next = nxt;
}

/* -------------------------------------------------------------------------- */

void core_tmr_insert( tmr_t *tmr )
{
	priv_tmr_insert(tmr);
	port_tmr_force();
}

/* -------------------------------------------------------------------------- */

void core_tmr_remove( tmr_t *tmr )
{
	priv_tmr_remove(tmr);
}

/* -------------------------------------------------------------------------- */

#if HW_TIMER_SIZE

static
bool priv_tmr_expired( tmr_t *tmr )
{
	port_tmr_stop();

	if (tmr->delay == INFINITE)
	return false; // return if timer counting indefinitely

	if (tmr->delay < core_sys_time() - tmr->start + 1)
	return true;  // return if timer finished counting

	port_tmr_start((hwt_t)(tmr->start + tmr->delay));

	if (tmr->delay >= core_sys_time() - tmr->start + 1)
	return false; // return if timer still counts

	port_tmr_stop();

	return true;  // however timer finished counting
}

/* -------------------------------------------------------------------------- */

#else

static
bool priv_tmr_expired( tmr_t *tmr )
{
	if (tmr->delay < core_sys_time() - tmr->start + 1)
	return true;  // return if timer finished counting

	return false; // return if timer still counts or counting indefinitely
}

#endif

/* -------------------------------------------------------------------------- */

static
void priv_tmr_wakeup( tmr_t *tmr, int event )
{
	if (tmr->proc)
		((fun_a *)tmr->proc)(tmr->arg);

	priv_tmr_remove(tmr);
	if (tmr->delay >= core_sys_time() - tmr->start + 1)
		priv_tmr_insert(tmr);

	core_all_wakeup(tmr->obj.queue, event);
}

/* -------------------------------------------------------------------------- */

void core_tmr_handler( void )
{
	tmr_t *tmr;

	port_set_lock();
	{
		while (priv_tmr_expired(tmr = WAIT.hdr.next))
		{
			tmr->start += tmr->delay;

			if (tmr->hdr.id == ID_TIMER)
			{
				tmr->delay = tmr->period;
				priv_tmr_wakeup(tmr, E_SUCCESS);
			}
			else  /* hdr.id == ID_READY */
			{
				tmr->delay = 0;
				core_tsk_wakeup((tsk_t *)tmr, E_TIMEOUT);
			}
		}
	}
	port_clr_lock();
}

/* -------------------------------------------------------------------------- */
// SYSTEM TASK SERVICES
/* -------------------------------------------------------------------------- */

static
void priv_tsk_insert( tsk_t *tsk )
{
	tsk_t *prv;
	tsk_t *nxt = &IDLE;
	#if OS_ROBIN && HW_TIMER_SIZE == 0
	tsk->slice = 0;
	#endif
	if (tsk->prio)
		do nxt = nxt->hdr.next;
		while (tsk->prio <= nxt->prio);

	prv = nxt->hdr.prev;

	tsk->hdr.prev = prv;
	tsk->hdr.next = nxt;
	nxt->hdr.prev = tsk;
	prv->hdr.next = tsk;
}

/* -------------------------------------------------------------------------- */

static
void priv_tsk_remove( tsk_t *tsk )
{
	tsk_t *prv = tsk->hdr.prev;
	tsk_t *nxt = tsk->hdr.next;

	nxt->hdr.prev = prv;
	prv->hdr.next = nxt;
}

/* -------------------------------------------------------------------------- */

void core_tsk_insert( tsk_t *tsk )
{
	tsk->hdr.id = ID_READY;
	priv_tsk_insert(tsk);
	#if OS_ROBIN
	if (tsk == IDLE.hdr.next)
		port_ctx_switch();
	#endif
}

/* -------------------------------------------------------------------------- */

void core_tsk_remove( tsk_t *tsk )
{
	tsk->hdr.id = ID_STOPPED;
	priv_tsk_remove(tsk);
	if (tsk == System.cur)
		port_ctx_switchNow();
}

/* -------------------------------------------------------------------------- */

void core_ctx_init( tsk_t *tsk )
{
	assert(tsk->size>STK_OVER(sizeof(ctx_t)));
	#ifdef DEBUG
	if (tsk != System.cur)
		memset(tsk->stack, 0xFF, tsk->size - sizeof(ctx_t));
	#endif
	tsk->sp = (ctx_t *)STK_CROP(tsk->stack, tsk->size) - 1;
	#if OS_TASK_EXIT
	port_ctx_init(tsk->sp, core_tsk_exec);
	#else
	port_ctx_init(tsk->sp, core_tsk_loop);
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
bool priv_stk_integrity( tsk_t *tsk, void *tp, void *sp)
{
	if (tsk == &MAIN) return true;
	if (sp < tp) return false;
	#if (__MPU_USED == 0) && ((OS_GUARD_SIZE) > 0)
	if (tsk == &IDLE) return true;
	if (core_stk_space(tsk) < STK_OVER(0)) return false;
	#endif
	return true;
}

bool core_ctx_integrity( tsk_t *tsk, void *sp )
{
	void *tp = tsk->stack + STK_SIZE(0);
	return priv_stk_integrity(tsk, tp, sp);
}

bool core_stk_integrity( void )
{
	tsk_t *tsk = System.cur;
	void *tp = tsk->stack + STK_SIZE(sizeof(ctx_t));
	void *sp = port_get_sp();
	return priv_stk_integrity(tsk, tp, sp);
}

#endif

void core_stk_move( tsk_t *tsk, size_t size )
{
	assert(size<tsk->size);

	tsk->stack += STK_SIZE(size);
	tsk->size  -= STK_OVER(size);
}

/* -------------------------------------------------------------------------- */

void core_ctx_switch( void )
{
	tsk_t *cur = IDLE.hdr.next;
	tsk_t *nxt = cur->hdr.next;
	if (nxt->prio == cur->prio)
		port_ctx_switch();
}

/* -------------------------------------------------------------------------- */

void core_ctx_switchNow( void )
{
	tsk_t *cur = IDLE.hdr.next;
	tsk_t *nxt = cur->hdr.next;
	if (nxt->prio == cur->prio)
		port_ctx_switchNow();
}

/* -------------------------------------------------------------------------- */

void core_tsk_loop( void )
{
	for (;;)
	{
		port_clr_lock();
		((fun_a *)System.cur->proc)(System.cur->arg);
		port_set_lock();
		core_ctx_switchNow();
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

void core_tsk_append( tsk_t *tsk, tsk_t **que )
{
	tsk_t *nxt = *que;
	tsk->guard  = que;
	tsk->hdr.id = ID_READY;

	while (nxt && tsk->prio <= nxt->prio)
	{
		que = &nxt->obj.queue;
		nxt = *que;
	}

	if (nxt)
		nxt->back = &tsk->obj.queue;
	tsk->back = que;
	tsk->obj.queue = nxt;
	*que = tsk;
}

/* -------------------------------------------------------------------------- */

void core_tsk_unlink( tsk_t *tsk, int event )
{
	tsk_t**que = tsk->back;
	tsk_t *nxt = tsk->obj.queue;
	tsk->event = event;
	tsk->guard = 0;

	if (nxt)
		nxt->back = que;
	*que = nxt;
}

/* -------------------------------------------------------------------------- */

void core_tsk_transfer( tsk_t *tsk, tsk_t **que )
{
	core_tsk_unlink(tsk, tsk->event);
	core_tsk_append(tsk, que);
}

/* -------------------------------------------------------------------------- */

int core_tsk_wait( tsk_t *tsk, tsk_t **que )
{
	assert_tsk_context();

	if (que)
	{
		priv_tsk_remove(tsk);
		core_tmr_insert((tmr_t *)tsk); // sets ID_TIMER for a while
		core_tsk_append(tsk, que);     // must be last; sets ID_READY back
	}

	if (tsk == System.cur)
		port_ctx_switchNow();

	return tsk->event;
}

/* -------------------------------------------------------------------------- */

int core_tsk_waitFor( tsk_t **que, cnt_t delay )
{
	tsk_t *cur = System.cur;

	cur->start = core_sys_time();
	cur->delay = delay;

	if (cur->delay == IMMEDIATE)
		return E_TIMEOUT;

	return core_tsk_wait(cur, que);
}

/* -------------------------------------------------------------------------- */

int core_tsk_waitNext( tsk_t **que, cnt_t delay )
{
	tsk_t *cur = System.cur;

	cur->delay = delay;

	if (cur->delay == IMMEDIATE)
		return E_TIMEOUT;

	return core_tsk_wait(cur, que);
}

/* -------------------------------------------------------------------------- */

int core_tsk_waitUntil( tsk_t **que, cnt_t time )
{
	tsk_t *cur = System.cur;

	cur->start = core_sys_time();
	cur->delay = time - cur->start;

	if (cur->delay - 1U > CNT_LIMIT)
		return E_TIMEOUT;

	return core_tsk_wait(cur, que);
}

/* -------------------------------------------------------------------------- */

void core_tsk_suspend( tsk_t *tsk )
{
	tsk->delay = INFINITE;

	core_tsk_wait(tsk, &WAIT.obj.queue);
}

/* -------------------------------------------------------------------------- */

tsk_t *core_tsk_wakeup( tsk_t *tsk, int event )
{
	if (tsk)
	{
		core_tsk_unlink(tsk, event);
		priv_tmr_remove((tmr_t *)tsk);
		core_tsk_insert(tsk);
	}

	return tsk;
}

/* -------------------------------------------------------------------------- */

unsigned core_num_wakeup( tsk_t *tsk, int event, unsigned num )
{
	unsigned cnt = 0;

	while (tsk && num > 0) { tsk = core_tsk_wakeup(tsk, event)->obj.queue; ++cnt; --num; }

	return cnt;
}

/* -------------------------------------------------------------------------- */

void core_all_wakeup( tsk_t *tsk, int event )
{
	while (tsk) { tsk = core_tsk_wakeup(tsk, event)->obj.queue; }
}

/* -------------------------------------------------------------------------- */

unsigned core_tsk_count( tsk_t *tsk )
{
	unsigned cnt = 0;

	while (tsk) { tsk = tsk->obj.queue; ++cnt; }

	return cnt;
}

/* -------------------------------------------------------------------------- */

void core_tsk_prio( tsk_t *tsk, unsigned prio )
{
	mtx_t *mtx;

	if (prio < tsk->basic)
		prio = tsk->basic;

	for (mtx = tsk->mtx.list; mtx; mtx = mtx->list)
		if ((mtx->mode & mtxPrioMASK) != mtxPrioNone && mtx->obj.queue)
			if (prio < mtx->obj.queue->prio)
				prio = mtx->obj.queue->prio;

	if (tsk->prio != prio)
	{
		tsk->prio = prio;

		if (tsk == System.cur)       // current task
		{
			tsk = tsk->hdr.next;
			#if OS_ROBIN
			if (tsk->prio > prio)
				port_ctx_switch();
			#endif
		}
		else
		if (tsk->guard != 0)         // blocked task
		{
			core_tsk_transfer(tsk, tsk->guard);
			if (tsk->mtx.tree)
				core_tsk_prio(tsk->mtx.tree->owner, prio);
		}
		else
		if (tsk->hdr.id == ID_READY) // ready task
		{
			priv_tsk_remove(tsk);
			core_tsk_insert(tsk);
		}
	}
}

/* -------------------------------------------------------------------------- */

void core_cur_prio( unsigned prio )
{
	mtx_t *mtx;
	tsk_t *tsk = System.cur;

	if (prio < tsk->basic)
		prio = tsk->basic;

	for (mtx = tsk->mtx.list; mtx; mtx = mtx->list)
		if ((mtx->mode & mtxPrioMASK) != mtxPrioNone && mtx->obj.queue)
			if (prio < mtx->obj.queue->prio)
				prio = mtx->obj.queue->prio;

	if (tsk->prio != prio)
	{
		tsk->prio = prio;
		tsk = tsk->hdr.next;
		#if OS_ROBIN
		if (tsk->prio > prio)
			port_ctx_switch();
		#endif
	}
}

/* -------------------------------------------------------------------------- */

void *core_tsk_switch( void *sp )
{
	tsk_t *cur, *nxt;

	assert_ctx_integrity(System.cur, sp);

	port_set_lock();
	{
		core_ctx_reset();

		cur = System.cur;
		if (cur->sp == 0)
			cur->sp = sp;

		nxt = IDLE.hdr.next;

		#if OS_ROBIN && HW_TIMER_SIZE == 0
		if (cur == nxt || (nxt->slice >= (OS_FREQUENCY)/(OS_ROBIN) && (nxt->slice = 0) == 0))
		#else
		if (cur == nxt)
		#endif
		{
			priv_tsk_remove(nxt);
			priv_tsk_insert(nxt);
			nxt = IDLE.hdr.next;
		}

		System.cur = nxt;
		sp = nxt->sp;
		nxt->sp = 0;

		#if __MPU_USED == 1
	//	port_mpu_disable();
		port_mpu_stackUpdate(nxt == &MAIN ? NULL : nxt->stack);
	//	port_mpu_enable();
		#endif
	}
	port_clr_lock();

	assert_ctx_integrity(System.cur, sp);

	return sp;
}

/* -------------------------------------------------------------------------- */
// SYSTEM MUTEX SERVICES
/* -------------------------------------------------------------------------- */

void core_mtx_link( mtx_t *mtx, tsk_t *tsk )
{
	assert(mtx);

	mtx->owner = tsk;

	if (tsk)
	{
		mtx->list = tsk->mtx.list;
		tsk->mtx.list = mtx;
	}
}

/* -------------------------------------------------------------------------- */

void core_mtx_unlink( mtx_t *mtx )
{
	tsk_t *tsk;
	mtx_t *lst;

	assert(mtx);

	tsk = mtx->owner;

	if (tsk)
	{
		if (tsk->mtx.list == mtx)
			tsk->mtx.list = mtx->list;

		for (lst = tsk->mtx.list; lst; lst = lst->list)
			if (lst->list == mtx)
				lst->list = mtx->list;

		mtx->list  = 0;
		mtx->owner = 0;
		mtx->count = 0;

		core_tsk_prio(tsk, tsk->basic);
	}
}

/* -------------------------------------------------------------------------- */

tsk_t *core_mtx_transferLock( mtx_t *mtx, int event )
{
	tsk_t *tsk;

	core_mtx_unlink(mtx);
	tsk = core_one_wakeup(mtx->obj.queue, event);
	core_mtx_link(mtx, tsk);

	return tsk;
}

/* -------------------------------------------------------------------------- */

void core_mtx_reset( mtx_t *mtx, int event )
{
	core_mtx_unlink(mtx);
	core_all_wakeup(mtx->obj.queue, event);
}

/* -------------------------------------------------------------------------- */

#if HW_TIMER_SIZE == 0

void core_sys_tick( void )
{
	System.cnt++;
	core_tmr_handler();
	#if OS_ROBIN
	if (++System.cur->slice >= (OS_FREQUENCY)/(OS_ROBIN))
		core_ctx_switch();
	#endif
}

#endif

/* -------------------------------------------------------------------------- */

void core_tsk_idle( void )
{
	__WFI();
}

/* -------------------------------------------------------------------------- */

void core_res_free( obj_t *obj )
{
	void *ptr = obj->res;
	if (ptr != NULL && ptr != RELEASED)
	{
		obj->res = RELEASED;
		free(ptr);
	}
}

/* -------------------------------------------------------------------------- */

void core_tsk_deleter( void )
{
	tsk_t *tsk;

	while (tsk = IDLE.obj.queue, tsk)   // garbage collection procedure
	{
		core_tsk_unlink(tsk, 0);        // remove task from DESTRUCTOR queue; ignored event value
		core_tmr_remove((tmr_t *)tsk);  // remove task from WAIT queue
		core_res_free(&tsk->obj);       // release resources
	}
}

/* -------------------------------------------------------------------------- */
