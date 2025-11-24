/******************************************************************************

    @file    IntrOS: ostask.c
    @author  Rajmund Szymanski
    @date    19.11.2025
    @brief   This file provides set of functions for IntrOS.

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

#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_wrk_init( tsk_t *tsk, fun_t *proc, void *arg, stk_t *stack, size_t size )
/* -------------------------------------------------------------------------- */
{
	memset(tsk, 0, sizeof(tsk_t));

	core_hdr_init(&tsk->hdr);

	tsk->proc  = proc;
	tsk->arg   = arg;
	tsk->stack = stack;
	tsk->size  = size;
}

/* -------------------------------------------------------------------------- */
void wrk_init( tsk_t *tsk, fun_t *proc, stk_t *stack, size_t size )
/* -------------------------------------------------------------------------- */
{
	assert(tsk);
	assert(stack);
	assert(size);

	sys_lock();
	{
		priv_wrk_init(tsk, proc, NULL, stack, size);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void tsk_init( tsk_t *tsk, fun_t *proc, stk_t *stack, size_t size )
/* -------------------------------------------------------------------------- */
{
	assert(tsk);
	assert(stack);
	assert(size);

	sys_lock();
	{
		priv_wrk_init(tsk, proc, NULL, stack, size);
		if (proc)
		{
			tsk->start = core_sys_time();

			core_ctx_init(tsk);
			core_tsk_insert(tsk);
		}
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void tsk_start( tsk_t *tsk )
/* -------------------------------------------------------------------------- */
{
	assert(tsk);
	assert(tsk->proc);

	sys_lock();
	{
		if (tsk->hdr.id == ID_STOPPED)  // active tasks cannot be started
		{
			tsk->start = core_sys_time();

			core_ctx_init(tsk);
			core_tsk_insert(tsk);
		}
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void tsk_startFrom( tsk_t *tsk, fun_t *proc )
/* -------------------------------------------------------------------------- */
{
	assert(tsk);
	assert(proc);

	sys_lock();
	{
		if (tsk->hdr.id == ID_STOPPED)  // active tasks cannot be started
		{
			tsk->proc = proc;
			tsk->start = core_sys_time();

			core_ctx_init(tsk);
			core_tsk_insert(tsk);
		}
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void tsk_startWith( tsk_t *tsk, fun_a *proc, void *arg )
/* -------------------------------------------------------------------------- */
{
	assert(tsk);
	assert(proc);

	sys_lock();
	{
		if (tsk->hdr.id == ID_STOPPED)  // active tasks cannot be started
		{
			tsk->proc = (fun_t *)proc;
			tsk->arg  = arg;
			tsk->start = core_sys_time();

			core_ctx_init(tsk);
			core_tsk_insert(tsk);
		}
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
void priv_tsk_stop( tsk_t *tsk )
/* -------------------------------------------------------------------------- */
{
	if (tsk->hdr.id != ID_STOPPED)	// inactive task cannot be removed
	{
		core_tsk_wakeup(tsk, 0);	// remove task from blocked queue; ignored event value
		core_tsk_remove(tsk);		// remove task from ready queue
	}
}

/* -------------------------------------------------------------------------- */
void tsk_stop( void )
/* -------------------------------------------------------------------------- */
{
	port_set_lock();

	priv_tsk_stop(System.cur);                     // remove current task from ready queue

	assert(false);                                 // system cannot return here
	for (;;);                                      // disable unnecessary warning
}

/* -------------------------------------------------------------------------- */
void tsk_reset( tsk_t *tsk )
/* -------------------------------------------------------------------------- */
{
	assert(tsk);

	sys_lock();
	{
		priv_tsk_stop(tsk);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void tsk_flip( fun_t *proc )
/* -------------------------------------------------------------------------- */
{
	assert(proc);

	port_set_lock();

	System.cur->proc = proc;

	core_ctx_init(System.cur);
	core_tsk_switch();
}

/* -------------------------------------------------------------------------- */
void tsk_sleepFor( cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	sys_lock();
	{
		core_tsk_waitFor(&System.cur->obj.queue, delay);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void tsk_sleepNext( cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	sys_lock();
	{
		core_tsk_waitNext(&System.cur->obj.queue, delay);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void tsk_sleepUntil( cnt_t time )
/* -------------------------------------------------------------------------- */
{
	sys_lock();
	{
		core_tsk_waitUntil(&System.cur->obj.queue, time);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
int tsk_suspend( tsk_t *tsk )
/* -------------------------------------------------------------------------- */
{
	int result = E_FAILURE;

	assert(tsk);

	sys_lock();
	{
		if (tsk->hdr.id == ID_READY && tsk->guard == 0)
		{
			tsk->delay = INFINITE;
			core_tsk_wait(&tsk->obj.queue, tsk);
			result = E_SUCCESS;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int tsk_resume( tsk_t *tsk )
/* -------------------------------------------------------------------------- */
{
	int result = E_FAILURE;

	assert(tsk);

	sys_lock();
	{
		if (tsk->hdr.id == ID_READY && tsk->delay == INFINITE)
		{
			core_tsk_wakeup(tsk, 0); // ignored event value
			result = E_SUCCESS;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
