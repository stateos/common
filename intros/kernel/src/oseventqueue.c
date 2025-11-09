/******************************************************************************

    @file    IntrOS: oseventqueue.c
    @author  Rajmund Szymanski
    @date    22.07.2022
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

#include "inc/oseventqueue.h"
#include "inc/oscriticalsection.h"
#include "inc/ostask.h"

/* -------------------------------------------------------------------------- */
void evq_init( evq_t *evq, unsigned *data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert(evq);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		memset(evq, 0, sizeof(evq_t));

		evq->limit = bufsize / sizeof(unsigned);
		evq->data  = data;
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
bool priv_evq_empty( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
#if OS_ATOMICS
	return atomic_load((atomic_size_t *)&evq->count) == 0;
#else
	return evq->count == 0;
#endif
}

/* -------------------------------------------------------------------------- */
static
bool priv_evq_full( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
#if OS_ATOMICS
	return atomic_load((atomic_size_t *)&evq->count) == evq->limit;
#else
	return evq->count == evq->limit;
#endif
}

/* -------------------------------------------------------------------------- */
static
void priv_evq_dec( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	evq->head = evq->head + 1 < evq->limit ? evq->head + 1 : 0;
#if OS_ATOMICS
	atomic_fetch_sub((atomic_size_t *)&evq->count, 1);
#else
	evq->count -= 1;
#endif
}

/* -------------------------------------------------------------------------- */
static
void priv_evq_inc( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	evq->tail = evq->tail + 1 < evq->limit ? evq->tail + 1 : 0;
#if OS_ATOMICS
	atomic_fetch_add((atomic_size_t *)&evq->count, 1);
#else
	evq->count += 1;
#endif
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_evq_get( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	unsigned event = evq->data[evq->head];

	priv_evq_dec(evq);

	return event;
}

/* -------------------------------------------------------------------------- */
static
void priv_evq_put( evq_t *evq, const unsigned event )
/* -------------------------------------------------------------------------- */
{
	evq->data[evq->tail] = event;

	priv_evq_inc(evq);
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_evq_take( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	if (priv_evq_empty(evq))
		return FAILURE;

	return priv_evq_get(evq);
}

/* -------------------------------------------------------------------------- */
unsigned evq_take( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	assert(evq);
	assert(evq->data);
	assert(evq->limit);

	sys_lock();
	{
		result = priv_evq_take(evq);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
unsigned evq_wait( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	while (result = evq_take(evq), result == FAILURE)
		tsk_yield();

	return result;
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_evq_give( evq_t *evq, unsigned event )
/* -------------------------------------------------------------------------- */
{
	if (priv_evq_full(evq))
		return FAILURE;

	priv_evq_put(evq, event);

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned evq_give( evq_t *evq, unsigned event )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	assert(evq);
	assert(evq->data);
	assert(evq->limit);

	sys_lock();
	{
		result = priv_evq_give(evq, event);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
void evq_send( evq_t *evq, unsigned event )
/* -------------------------------------------------------------------------- */
{
	while (evq_give(evq, event) != SUCCESS)
		tsk_yield();
}

/* -------------------------------------------------------------------------- */
void evq_push( evq_t *evq, unsigned event )
/* -------------------------------------------------------------------------- */
{
	assert(evq);
	assert(evq->data);
	assert(evq->limit);

	sys_lock();
	{
		if (priv_evq_full(evq))
			priv_evq_dec(evq);
		priv_evq_put(evq, event);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
unsigned evq_getCount( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	unsigned count;

	assert(evq);

	sys_lock();
	{
		count = evq->count;
	}
	sys_unlock();

	return count;
}

/* -------------------------------------------------------------------------- */
unsigned evq_getSpace( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	unsigned space;

	assert(evq);

	sys_lock();
	{
		space = evq->limit - evq->count;
	}
	sys_unlock();

	return space;
}

/* -------------------------------------------------------------------------- */
unsigned evq_getLimit( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	unsigned limit;

	assert(evq);

	sys_lock();
	{
		limit = evq->limit;
	}
	sys_unlock();

	return limit;
}

/* -------------------------------------------------------------------------- */
