/******************************************************************************

    @file    IntrOS: ostimer.c
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

#include "inc/ostimer.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_tmr_init( tmr_t *tmr, fun_t *proc, void *arg )
/* -------------------------------------------------------------------------- */
{
	memset(tmr, 0, sizeof(tmr_t));

	core_hdr_init(&tmr->hdr);

	tmr->proc = proc;
	tmr->arg  = arg;
}

/* -------------------------------------------------------------------------- */
void tmr_init( tmr_t *tmr, fun_t *proc )
/* -------------------------------------------------------------------------- */
{
	assert(tmr);

	sys_lock();
	{
		priv_tmr_init(tmr, proc, NULL);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
void priv_tmr_reset( tmr_t *tmr, int event )
/* -------------------------------------------------------------------------- */
{
	if (tmr->hdr.id != ID_STOPPED)
	{
		core_all_wakeup(&tmr->obj.queue, event);
		core_tmr_remove(tmr);
	}
}

/* -------------------------------------------------------------------------- */
void tmr_reset( tmr_t *tmr )
/* -------------------------------------------------------------------------- */
{
	assert(tmr);

	sys_lock();
	{
		priv_tmr_reset(tmr, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
void priv_tmr_start( tmr_t *tmr )
/* -------------------------------------------------------------------------- */
{
	if (tmr->hdr.id == ID_STOPPED)
		core_tmr_insert(tmr);
}

/* -------------------------------------------------------------------------- */
void tmr_start( tmr_t *tmr, cnt_t delay, cnt_t period )
/* -------------------------------------------------------------------------- */
{
	assert(tmr);

	sys_lock();
	{
		tmr->start  = core_sys_time();
		tmr->delay  = delay;
		tmr->period = period;

		priv_tmr_start(tmr);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void tmr_startFrom( tmr_t *tmr, cnt_t delay, cnt_t period, fun_t *proc )
/* -------------------------------------------------------------------------- */
{
	assert(tmr);

	sys_lock();
	{
		tmr->proc   = proc;
		tmr->start  = core_sys_time();
		tmr->delay  = delay;
		tmr->period = period;

		priv_tmr_start(tmr);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void tmr_startNext( tmr_t *tmr, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	assert(tmr);

	sys_lock();
	{
		tmr->delay = delay;

		priv_tmr_start(tmr);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void tmr_startUntil( tmr_t *tmr, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	assert(tmr);

	sys_lock();
	{
		tmr->start = core_sys_time();
		tmr->delay = time - tmr->start;
		if (tmr->delay > CNT_LIMIT)
			tmr->delay = 0;
		tmr->period = 0;

		priv_tmr_start(tmr);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
int priv_tmr_take( tmr_t *tmr )
/* -------------------------------------------------------------------------- */
{
	if (tmr->hdr.next == 0)
		return E_FAILURE; // timer has not yet been started

	if (tmr->hdr.id == ID_STOPPED)
		return E_SUCCESS;

	return E_TIMEOUT;
}

/* -------------------------------------------------------------------------- */
int tmr_take( tmr_t *tmr )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(tmr);

	sys_lock();
	{
		result = priv_tmr_take(tmr);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int tmr_waitFor( tmr_t *tmr, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(tmr);

	sys_lock();
	{
		result = priv_tmr_take(tmr);
		if (result == E_TIMEOUT)
			result = core_tsk_waitFor(&tmr->obj.queue, delay);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int tmr_waitNext( tmr_t *tmr, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(tmr);

	sys_lock();
	{
		result = priv_tmr_take(tmr);
		if (result == E_TIMEOUT)
			result = core_tsk_waitNext(&tmr->obj.queue, delay);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int tmr_waitUntil( tmr_t *tmr, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(tmr);

	sys_lock();
	{
		result = priv_tmr_take(tmr);
		if (result == E_TIMEOUT)
			result = core_tsk_waitUntil(&tmr->obj.queue, time);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
