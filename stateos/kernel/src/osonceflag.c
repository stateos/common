/******************************************************************************

    @file    StateOS: osonceflag.c
    @author  Rajmund Szymanski
    @date    13.10.2026
    @brief   This file provides set of functions for StateOS.

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

#include "inc/osonceflag.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */

static
tsk_t *OnceQueue = NULL; // tasks waiting for the end of once functions

/* -------------------------------------------------------------------------- */
bool one_enter( one_t *one )
/* -------------------------------------------------------------------------- */
{
	one_t flag;

	assert(one);

	sys_lock();
	{
		flag = *one;

		while (flag == _ONE_BUSY() && !port_isr_context())
		{
			core_tsk_waitFor(&OnceQueue, INFINITE); // wait for one_leave
			flag = *one;
		}

		if (flag == _ONE_INIT())
			*one = _ONE_BUSY();
	}
	sys_unlock();

	return flag == _ONE_INIT();
}

/* -------------------------------------------------------------------------- */
void one_leave( one_t *one, one_t state )
/* -------------------------------------------------------------------------- */
{
	assert(one);
	assert(*one == _ONE_BUSY());

	sys_lock();
	{
		*one = state;
		core_all_wakeup(&OnceQueue, E_SUCCESS);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
