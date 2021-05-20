/******************************************************************************

    @file    StateOS: oseventqueue.c
    @author  Rajmund Szymanski
    @date    20.05.2021
    @brief   This file provides set of functions for StateOS.

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

#include "inc/oseventqueue.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_evq_init( evq_t *evq, unsigned *data, size_t bufsize, void *res )
/* -------------------------------------------------------------------------- */
{
	memset(evq, 0, sizeof(evq_t));

	core_obj_init(&evq->obj, res);

	evq->limit = bufsize / sizeof(unsigned);
	evq->data  = data;
}

/* -------------------------------------------------------------------------- */
void evq_init( evq_t *evq, unsigned *data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(evq);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		priv_evq_init(evq, data, bufsize, NULL);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
evq_t *evq_create( unsigned limit )
/* -------------------------------------------------------------------------- */
{
	struct evq_T { evq_t evq; unsigned buf[]; } *tmp;
	evq_t *evq = NULL;
	size_t bufsize;

	assert_tsk_context();
	assert(limit);

	sys_lock();
	{
		bufsize = limit * sizeof(unsigned);
		tmp = malloc(sizeof(struct evq_T) + bufsize);
		if (tmp)
			priv_evq_init(evq = &tmp->evq, tmp->buf, bufsize, tmp);
	}
	sys_unlock();

	return evq;
}

/* -------------------------------------------------------------------------- */
static
void priv_evq_reset( evq_t *evq, int event )
/* -------------------------------------------------------------------------- */
{
	evq->count = 0;
	evq->head  = 0;
	evq->tail  = 0;

	core_all_wakeup(evq->obj.queue, event);
}

/* -------------------------------------------------------------------------- */
void evq_reset( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(evq);
	assert(evq->obj.res!=RELEASED);

	sys_lock();
	{
		priv_evq_reset(evq, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void evq_destroy( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(evq);
	assert(evq->obj.res!=RELEASED);

	sys_lock();
	{
		priv_evq_reset(evq, evq->obj.res ? E_DELETED : E_STOPPED);
		core_res_free(&evq->obj);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
bool priv_evq_empty( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	return evq->count == 0;
}

/* -------------------------------------------------------------------------- */
static
bool priv_evq_full( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	return evq->count == evq->limit;
}

/* -------------------------------------------------------------------------- */
static
void priv_evq_dec( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	evq->head = evq->head + 1 < evq->limit ? evq->head + 1 : 0;
	evq->count -= 1;
}

/* -------------------------------------------------------------------------- */
static
void priv_evq_inc( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	evq->tail = evq->tail + 1 < evq->limit ? evq->tail + 1 : 0;
	evq->count += 1;
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
unsigned priv_evq_getUpdate( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	unsigned event = priv_evq_get(evq);

	tsk = core_one_wakeup(evq->obj.queue, E_SUCCESS);
	if (tsk)
		priv_evq_put(evq, tsk->tmp.evq.event);

	return event;
}

/* -------------------------------------------------------------------------- */
static
void priv_evq_putUpdate( evq_t *evq, const unsigned event )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	priv_evq_put(evq, event);

	tsk = core_one_wakeup(evq->obj.queue, E_SUCCESS);
	if (tsk)
		tsk->tmp.evq.event = priv_evq_get(evq);
}

/* -------------------------------------------------------------------------- */
static
void priv_evq_skipUpdate( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	while (priv_evq_full(evq))
	{
		priv_evq_dec(evq);

		tsk = core_one_wakeup(evq->obj.queue, E_SUCCESS);
		if (tsk)
			priv_evq_put(evq, tsk->tmp.evq.event);
	}
}

/* -------------------------------------------------------------------------- */
static
int priv_evq_take( evq_t *evq, unsigned *event )
/* -------------------------------------------------------------------------- */
{
	unsigned evt;

	if (priv_evq_empty(evq))
		return E_TIMEOUT;

	evt = priv_evq_getUpdate(evq);
	if (event != NULL)
		*event = evt;

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int evq_take( evq_t *evq, unsigned *event )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(evq);
	assert(evq->obj.res!=RELEASED);
	assert(evq->data);
	assert(evq->limit);

	sys_lock();
	{
		result = priv_evq_take(evq, event);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int evq_waitFor( evq_t *evq, unsigned *event, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(evq);
	assert(evq->obj.res!=RELEASED);
	assert(evq->data);
	assert(evq->limit);

	sys_lock();
	{
		result = priv_evq_take(evq, event);
		if (result == E_TIMEOUT)
		{
			result = core_tsk_waitFor(&evq->obj.queue, delay);
			if (result == E_SUCCESS && event != NULL)
				*event = System.cur->tmp.evq.event;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int evq_waitUntil( evq_t *evq, unsigned *event, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(evq);
	assert(evq->obj.res!=RELEASED);
	assert(evq->data);
	assert(evq->limit);

	sys_lock();
	{
		result = priv_evq_take(evq, event);
		if (result == E_TIMEOUT)
		{
			result = core_tsk_waitUntil(&evq->obj.queue, time);
			if (result == E_SUCCESS && event != NULL)
				*event = System.cur->tmp.evq.event;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
static
int priv_evq_give( evq_t *evq, unsigned event )
/* -------------------------------------------------------------------------- */
{
	if (priv_evq_full(evq))
		return E_TIMEOUT;

	priv_evq_putUpdate(evq, event);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int evq_give( evq_t *evq, unsigned event )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(evq);
	assert(evq->obj.res!=RELEASED);
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
int evq_sendFor( evq_t *evq, unsigned event, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(evq);
	assert(evq->obj.res!=RELEASED);
	assert(evq->data);
	assert(evq->limit);

	sys_lock();
	{
		result = priv_evq_give(evq, event);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.evq.event = event;
			result = core_tsk_waitFor(&evq->obj.queue, delay);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int evq_sendUntil( evq_t *evq, unsigned event, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(evq);
	assert(evq->obj.res!=RELEASED);
	assert(evq->data);
	assert(evq->limit);

	sys_lock();
	{
		result = priv_evq_give(evq, event);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.evq.event = event;
			result = core_tsk_waitUntil(&evq->obj.queue, time);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
void evq_push( evq_t *evq, unsigned event )
/* -------------------------------------------------------------------------- */
{
	assert(evq);
	assert(evq->obj.res!=RELEASED);
	assert(evq->data);
	assert(evq->limit);

	sys_lock();
	{
		priv_evq_skipUpdate(evq);
		priv_evq_putUpdate(evq, event);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
unsigned evq_count( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	unsigned count;

	assert(evq);
	assert(evq->obj.res!=RELEASED);

	sys_lock();
	{
		count = evq->count;
	}
	sys_unlock();

	return count;
}

/* -------------------------------------------------------------------------- */
unsigned evq_space( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	unsigned space;

	assert(evq);
	assert(evq->obj.res!=RELEASED);

	sys_lock();
	{
		space = evq->limit - evq->count;
	}
	sys_unlock();

	return space;
}

/* -------------------------------------------------------------------------- */
unsigned evq_limit( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	unsigned limit;

	assert(evq);
	assert(evq->obj.res!=RELEASED);

	sys_lock();
	{
		limit = evq->limit;
	}
	sys_unlock();

	return limit;
}

/* -------------------------------------------------------------------------- */

#if OS_ATOMICS

/* -------------------------------------------------------------------------- */
static
bool priv_evq_emptyAsync( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	return atomic_load((atomic_size_t *)&evq->count) == 0;
}

/* -------------------------------------------------------------------------- */
static
bool priv_evq_fullAsync( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	return atomic_load((atomic_size_t *)&evq->count) == evq->limit;
}

/* -------------------------------------------------------------------------- */
static
void priv_evq_decAsync( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	evq->head = evq->head + 1 < evq->limit ? evq->head + 1 : 0;
	atomic_fetch_sub((atomic_size_t *)&evq->count, 1);
}

/* -------------------------------------------------------------------------- */
static
void priv_evq_incAsync( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	evq->tail = evq->tail + 1 < evq->limit ? evq->tail + 1 : 0;
	atomic_fetch_add((atomic_size_t *)&evq->count, 1);
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_evq_getAsync( evq_t *evq )
/* -------------------------------------------------------------------------- */
{
	unsigned event = evq->data[evq->head];

	priv_evq_decAsync(evq);

	return event;
}

/* -------------------------------------------------------------------------- */
static
void priv_evq_putAsync( evq_t *evq, const unsigned event )
/* -------------------------------------------------------------------------- */
{
	evq->data[evq->tail] = event;

	priv_evq_incAsync(evq);
}

/* -------------------------------------------------------------------------- */
static
int priv_evq_takeAsync( evq_t *evq, unsigned *event )
/* -------------------------------------------------------------------------- */
{
	unsigned evt;

	if (priv_evq_emptyAsync(evq))
		return E_TIMEOUT;

	evt = priv_evq_getAsync(evq);
	if (event != NULL)
		*event = evt;

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int evq_takeAsync( evq_t *evq, unsigned *event )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(evq);
	assert(evq->obj.res!=RELEASED);
	assert(evq->data);
	assert(evq->limit);

	sys_lock();
	{
		result = priv_evq_takeAsync(evq, event);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int evq_waitAsync( evq_t *evq, unsigned *event )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();

	while (evq_takeAsync(evq, event) != E_SUCCESS)
		core_ctx_switch();

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
static
int priv_evq_giveAsync( evq_t *evq, unsigned event )
/* -------------------------------------------------------------------------- */
{
	if (priv_evq_fullAsync(evq))
		return E_TIMEOUT;

	priv_evq_putAsync(evq, event);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int evq_giveAsync( evq_t *evq, unsigned event )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(evq);
	assert(evq->obj.res!=RELEASED);
	assert(evq->data);
	assert(evq->limit);

	sys_lock();
	{
		result = priv_evq_giveAsync(evq, event);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int evq_sendAsync( evq_t *evq, unsigned event )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();

	while (evq_giveAsync(evq, event) != E_SUCCESS)
		core_ctx_switch();

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */

#endif//OS_ATOMICS
