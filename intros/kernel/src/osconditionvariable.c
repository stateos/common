/******************************************************************************

    @file    IntrOS: osconditionvariable.c
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

#include "inc/osconditionvariable.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_cnd_init( cnd_t *cnd )
/* -------------------------------------------------------------------------- */
{
	memset(cnd, 0, sizeof(cnd_t));
}

/* -------------------------------------------------------------------------- */
void cnd_init( cnd_t *cnd )
/* -------------------------------------------------------------------------- */
{
	assert(cnd);

	sys_lock();
	{
		priv_cnd_init(cnd);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
void priv_cnd_reset( cnd_t *cnd, int event )
/* -------------------------------------------------------------------------- */
{
	core_all_wakeup(&cnd->obj.queue, event);
}

/* -------------------------------------------------------------------------- */
void cnd_reset( cnd_t *cnd )
/* -------------------------------------------------------------------------- */
{
	assert(cnd);

	sys_lock();
	{
		priv_cnd_reset(cnd, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
int cnd_waitFor( cnd_t *cnd, mtx_t *mtx, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;
	int wait_result;

	assert(cnd);
	assert(mtx);

	sys_lock();
	{
		result = mtx_give(mtx);
		assert(result == E_SUCCESS);
		if (result == E_SUCCESS)
		{
			wait_result = core_tsk_waitFor(&cnd->obj.queue, delay);
			result = mtx_wait(mtx);
			assert(result == E_SUCCESS);
			if (result == E_SUCCESS)
				result = wait_result;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int cnd_waitUntil( cnd_t *cnd, mtx_t *mtx, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;
	int wait_result;

	assert(cnd);
	assert(mtx);

	sys_lock();
	{
		result = mtx_give(mtx);
		assert(result == E_SUCCESS);
		if (result == E_SUCCESS)
		{
			wait_result = core_tsk_waitUntil(&cnd->obj.queue, time);
			result = mtx_wait(mtx);
			assert(result == E_SUCCESS);
			if (result == E_SUCCESS)
				result = wait_result;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
void cnd_give( cnd_t *cnd, bool all )
/* -------------------------------------------------------------------------- */
{
	assert(cnd);

	sys_lock();
	{
		while (core_one_wakeup(&cnd->obj.queue, E_SUCCESS) && all);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
