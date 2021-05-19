/******************************************************************************

    @file    IntrOS: osmailboxqueue.c
    @author  Rajmund Szymanski
    @date    19.05.2021
    @brief   This file provides set of functions for IntrOS.

 ******************************************************************************

   Copyright (c) 2018-2021 Rajmund Szymanski. All rights reserved.

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

#include "inc/osmailboxqueue.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
void box_init( box_t *box, size_t size, void *data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert(box);
	assert(size);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		memset(box, 0, sizeof(box_t));

		box->data  = data;
		box->size  = size;
		box->limit = (bufsize / size) * size;
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
bool priv_box_empty( box_t *box )
/* -------------------------------------------------------------------------- */
{
#if OS_ATOMICS
	return atomic_load((atomic_size_t *)&box->count) == 0;
#else
	return box->count == 0;
#endif
}

/* -------------------------------------------------------------------------- */
static
bool priv_box_full( box_t *box )
/* -------------------------------------------------------------------------- */
{
#if OS_ATOMICS
	return atomic_load((atomic_size_t *)&box->count) == box->limit;
#else
	return box->count == box->limit;
#endif
}

/* -------------------------------------------------------------------------- */
static
void priv_box_dec( box_t *box )
/* -------------------------------------------------------------------------- */
{
	box->head = box->head + box->size < box->limit ? box->head + box->size : 0;
#if OS_ATOMICS
	atomic_fetch_sub((atomic_size_t *)&box->count, box->size);
#else
	box->count -= box->size;
#endif
}

/* -------------------------------------------------------------------------- */
static
void priv_box_inc( box_t *box )
/* -------------------------------------------------------------------------- */
{
	box->tail = box->tail + box->size < box->limit ? box->tail + box->size : 0;
#if OS_ATOMICS
	atomic_fetch_add((atomic_size_t *)&box->count, box->size);
#else
	box->count += box->size;
#endif
}

/* -------------------------------------------------------------------------- */
static
void priv_box_get( box_t *box, char *data )
/* -------------------------------------------------------------------------- */
{
	memcpy(data, &box->data[box->head], box->size);
	priv_box_dec(box);
}

/* -------------------------------------------------------------------------- */
static
void priv_box_put( box_t *box, const char *data )
/* -------------------------------------------------------------------------- */
{
	memcpy(&box->data[box->tail], data, box->size);
	priv_box_inc(box);
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_box_take( box_t *box, void *data )
/* -------------------------------------------------------------------------- */
{
	if (priv_box_empty(box))
		return FAILURE;

	priv_box_get(box, data);

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned box_take( box_t *box, void *data )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	assert(box);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		result = priv_box_take(box, data);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
void box_wait( box_t *box, void *data )
/* -------------------------------------------------------------------------- */
{
	while (box_take(box, data) != SUCCESS)
		core_ctx_switch();
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_box_give( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	if (priv_box_full(box))
		return FAILURE;

	priv_box_put(box, data);

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned box_give( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	assert(box);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		result = priv_box_give(box, data);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
void box_send( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	while (box_give(box, data) != SUCCESS)
		core_ctx_switch();
}

/* -------------------------------------------------------------------------- */
void box_push( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	assert(box);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		if (priv_box_full(box))
			priv_box_dec(box);
		priv_box_put(box, data);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
unsigned box_count( box_t *box )
/* -------------------------------------------------------------------------- */
{
	unsigned count;

	assert(box);

	sys_lock();
	{
		count = box->count / box->size;
	}
	sys_unlock();

	return count;
}

/* -------------------------------------------------------------------------- */
unsigned box_space( box_t *box )
/* -------------------------------------------------------------------------- */
{
	unsigned space;

	assert(box);

	sys_lock();
	{
		space = (box->limit - box->count) / box->size;
	}
	sys_unlock();

	return space;
}

/* -------------------------------------------------------------------------- */
unsigned box_limit( box_t *box )
/* -------------------------------------------------------------------------- */
{
	unsigned limit;

	assert(box);

	sys_lock();
	{
		limit = box->limit / box->size;
	}
	sys_unlock();

	return limit;
}

/* -------------------------------------------------------------------------- */
