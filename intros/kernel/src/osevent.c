/******************************************************************************

    @file    IntrOS: osevent.c
    @author  Rajmund Szymanski
    @date    17.11.2025
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

#include "inc/osevent.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_evt_init( evt_t *evt )
/* -------------------------------------------------------------------------- */
{
	memset(evt, 0, sizeof(evt_t));
}

/* -------------------------------------------------------------------------- */
void evt_init( evt_t *evt )
/* -------------------------------------------------------------------------- */
{
	assert(evt);

	sys_lock();
	{
		priv_evt_init(evt);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
void priv_evt_reset( evt_t *evt, int event )
/* -------------------------------------------------------------------------- */
{
	core_all_wakeup(&evt->obj.queue, event);
}

/* -------------------------------------------------------------------------- */
void evt_reset( evt_t *evt )
/* -------------------------------------------------------------------------- */
{
	assert(evt);

	sys_lock();
	{
		priv_evt_reset(evt, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
int evt_waitFor( evt_t *evt, unsigned *event, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(evt);

	sys_lock();
	{
		result = core_tsk_waitFor(&evt->obj.queue, delay);
		if (result == E_SUCCESS && event != NULL)
			*event = System.cur->tmp.evt.event;
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int evt_waitUntil( evt_t *evt, unsigned *event, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(evt);

	sys_lock();
	{
		result = core_tsk_waitUntil(&evt->obj.queue, time);
		if (result == E_SUCCESS && event != NULL)
			*event = System.cur->tmp.evt.event;
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
void evt_give( evt_t *evt, unsigned event )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	assert(evt);

	sys_lock();
	{
		while (tsk = core_one_wakeup(&evt->obj.queue, E_SUCCESS), tsk != NULL)
			tsk->tmp.evt.event = event;
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
