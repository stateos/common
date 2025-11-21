/******************************************************************************

    @file    IntrOS: osmutex.c
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

#include "inc/osmutex.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_mtx_init( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	memset(mtx, 0, sizeof(mtx_t));
}

/* -------------------------------------------------------------------------- */
void mtx_init( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	assert(mtx);

	sys_lock();
	{
		priv_mtx_init(mtx);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
void priv_mtx_reset( mtx_t *mtx, int event )
/* -------------------------------------------------------------------------- */
{
	core_all_wakeup(&mtx->obj.queue, event);
}

/* -------------------------------------------------------------------------- */
void mtx_reset( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	assert(mtx);

	sys_lock();
	{
		priv_mtx_reset(mtx, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
int priv_mtx_take( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	if (mtx->owner == NULL)
	{
		mtx->owner = System.cur;
		return E_SUCCESS;
	}

	if (mtx->owner != System.cur)
		return E_TIMEOUT;

	return E_FAILURE;
}

/* -------------------------------------------------------------------------- */
int mtx_take( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(mtx);

	sys_lock();
	{
		result = priv_mtx_take(mtx);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int mtx_waitFor( mtx_t *mtx, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(mtx);

	sys_lock();
	{
		result = priv_mtx_take(mtx);
		if (result == E_TIMEOUT)
		{
			result = core_tsk_waitFor(&mtx->obj.queue, delay);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int mtx_waitUntil( mtx_t *mtx, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(mtx);

	sys_lock();
	{
		result = priv_mtx_take(mtx);
		if (result == E_TIMEOUT)
		{
			result = core_tsk_waitUntil(&mtx->obj.queue, time);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
static
int priv_mtx_give( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	if (mtx->owner == System.cur)
	{
		mtx->owner = core_one_wakeup(&mtx->obj.queue, E_SUCCESS);
		return E_SUCCESS;
	}

	return E_FAILURE;
}

/* -------------------------------------------------------------------------- */
int mtx_give( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(mtx);

	sys_lock();
	{
		result = priv_mtx_give(mtx);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
