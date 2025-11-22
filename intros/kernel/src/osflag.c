/******************************************************************************

    @file    IntrOS: osflag.c
    @author  Rajmund Szymanski
    @date    18.11.2025
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

#include "inc/osflag.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_flg_init( flg_t *flg, unsigned init )
/* -------------------------------------------------------------------------- */
{
	memset(flg, 0, sizeof(flg_t));

	flg->flags = init;
}

/* -------------------------------------------------------------------------- */
void flg_init( flg_t *flg, unsigned init )
/* -------------------------------------------------------------------------- */
{
	assert(flg);

	sys_lock();
	{
		priv_flg_init(flg, init);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
void priv_flg_reset( flg_t *flg, int event )
/* -------------------------------------------------------------------------- */
{
	flg->flags = 0;

	core_all_wakeup(&flg->obj.queue, event);
}

/* -------------------------------------------------------------------------- */
void flg_reset( flg_t *flg )
/* -------------------------------------------------------------------------- */
{
	assert(flg);

	sys_lock();
	{
		priv_flg_reset(flg, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_flg_take( flg_t *flg, unsigned flags, unsigned mode )
/* -------------------------------------------------------------------------- */
{
	unsigned result = flags;

	if ((mode & flgNew) == 0)
		result &= ~flg->flags;

	if (result != flags && (mode & flgAll) == 0)
		result = 0;

	return result;
}

/* -------------------------------------------------------------------------- */
unsigned flg_take( flg_t *flg, unsigned flags, unsigned mode )
/* -------------------------------------------------------------------------- */
{
	assert(flg);

	sys_lock();
	{
		flags = priv_flg_take(flg, flags, mode);
	}
	sys_unlock();

	return flags;
}

/* -------------------------------------------------------------------------- */
int flg_waitFor( flg_t *flg, unsigned flags, unsigned mode, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result = E_SUCCESS;

	assert(flg);

	sys_lock();
	{
		flags = priv_flg_take(flg, flags, mode);
		if (flags != 0)
		{
			System.cur->tmp.flg.flags = flags;
			System.cur->tmp.flg.mode  = mode;
			result = core_tsk_waitFor(&flg->obj.queue, delay);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int flg_waitUntil( flg_t *flg, unsigned flags, unsigned mode, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result = E_SUCCESS;

	assert(flg);

	sys_lock();
	{
		flags = priv_flg_take(flg, flags, mode);
		if (flags != 0)
		{
			System.cur->tmp.flg.flags = flags;
			System.cur->tmp.flg.mode  = mode;
			result = core_tsk_waitUntil(&flg->obj.queue, time);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
unsigned flg_give( flg_t *flg, unsigned flags )
/* -------------------------------------------------------------------------- */
{
	unsigned result;
	tsk_t ** que;

	assert(flg);

	sys_lock();
	{
		flg->flags |= flags;
		result = flg->flags;

		que = &flg->obj.queue;
		while (*que)
		{
			if ((*que)->tmp.flg.flags & flags)
			{
				(*que)->tmp.flg.flags &= ~flags;
				if ((*que)->tmp.flg.flags == 0 || ((*que)->tmp.flg.mode & flgAll) == 0)
				{
					core_one_wakeup(que, E_SUCCESS);
					continue;
				}
			}
			que = &(*que)->obj.queue;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
unsigned flg_clear( flg_t *flg, unsigned flags )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	assert(flg);

	sys_lock();
	{
		result = flg->flags;
		flg->flags &= ~flags;
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
unsigned flg_get( flg_t *flg )
/* -------------------------------------------------------------------------- */
{
	unsigned flags;

	assert(flg);

	sys_lock();
	{
		flags = flg->flags;
	}
	sys_unlock();

	return flags;
}

/* -------------------------------------------------------------------------- */
