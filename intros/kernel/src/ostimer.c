/******************************************************************************

    @file    IntrOS: ostimer.c
    @author  Rajmund Szymanski
    @date    27.03.2023
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
void tmr_init( tmr_t *tmr, fun_t *proc )
/* -------------------------------------------------------------------------- */
{
	assert(tmr);

	sys_lock();
	{
		memset(tmr, 0, sizeof(tmr_t));

		core_hdr_init(&tmr->hdr);

		tmr->proc = proc;
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
		tmr->start  = core_sys_time();
		tmr->delay  = time - tmr->start;
		if (tmr->delay > CNT_LIMIT)
			tmr->delay = 0;
		tmr->period = 0;

		priv_tmr_start(tmr);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void tmr_reset( tmr_t *tmr )
/* -------------------------------------------------------------------------- */
{
	assert(tmr);

	sys_lock();
	{
		if (tmr->hdr.id != ID_STOPPED)  // only active timers can be reseted
			core_tmr_remove(tmr);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
unsigned tmr_take( tmr_t *tmr )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	assert(tmr);

	sys_lock();
	{
		result = tmr->hdr.id == ID_STOPPED ? SUCCESS : FAILURE;
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
void tmr_wait( tmr_t *tmr )
/* -------------------------------------------------------------------------- */
{
	unsigned signal;

	assert(tmr);

	sys_lock();
	{
		signal = tmr->signal;
		while (tmr->hdr.id != ID_STOPPED && tmr->signal == signal)
			core_ctx_switch();
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
