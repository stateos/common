/******************************************************************************

    @file    IntrOS: osmutex.c
    @author  Rajmund Szymanski
    @date    08.05.2021
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
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
void mtx_init( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	assert(mtx);

	sys_lock();
	{
		memset(mtx, 0, sizeof(mtx_t));
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_mtx_take( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	if (mtx->owner != NULL)
		return FAILURE;
	
	mtx->owner = System.cur;

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned mtx_take( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	assert(mtx);
	assert(mtx->owner != System.cur);

	sys_lock();
	{
		result = priv_mtx_take(mtx);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
void mtx_wait( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	while (mtx_take(mtx) != SUCCESS)
		core_ctx_switch();
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_mtx_give( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	if (mtx->owner != System.cur)
		return FAILURE;

    mtx->owner = NULL;

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned mtx_give( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	assert(mtx);

	sys_lock();
	{
		result = priv_mtx_give(mtx);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
