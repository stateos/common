/******************************************************************************

    @file    StateOS: osmailboxqueue.c
    @author  Rajmund Szymanski
    @date    01.11.2022
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

#include "inc/osmailboxqueue.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_box_init( box_t *box, size_t size, void *data, size_t bufsize, void *res )
/* -------------------------------------------------------------------------- */
{
	memset(box, 0, sizeof(box_t));

	core_obj_init(&box->obj, res);

	box->data  = data;
	box->size  = size;
	box->limit = (bufsize / size) * size;
}

/* -------------------------------------------------------------------------- */
void box_init( box_t *box, size_t size, void *data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(box);
	assert(size);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		priv_box_init(box, size, data, bufsize, NULL);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
box_t *box_create( unsigned limit, size_t size )
/* -------------------------------------------------------------------------- */
{
	struct box_T { box_t box; char buf[]; } *tmp;
	box_t *box = NULL;
	size_t bufsize;

	assert_tsk_context();
	assert(limit);
	assert(size);

	sys_lock();
	{
		bufsize = limit * size;
		tmp = malloc(sizeof(struct box_T) + bufsize);
		if (tmp)
			priv_box_init(box = &tmp->box, size, tmp->buf, bufsize, tmp);
	}
	sys_unlock();

	return box;
}

/* -------------------------------------------------------------------------- */
static
void priv_box_reset( box_t *box, int event )
/* -------------------------------------------------------------------------- */
{
	box->count = 0;
	box->head  = 0;
	box->tail  = 0;

	core_all_wakeup(box->obj.queue, event);
}

/* -------------------------------------------------------------------------- */
void box_reset( box_t *box )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(box);
	assert(box->obj.res!=RELEASED);

	sys_lock();
	{
		priv_box_reset(box, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void box_destroy( box_t *box )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(box);
	assert(box->obj.res!=RELEASED);

	sys_lock();
	{
		priv_box_reset(box, box->obj.res ? E_DELETED : E_STOPPED);
		core_res_free(&box->obj);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
bool priv_box_empty( box_t *box )
/* -------------------------------------------------------------------------- */
{
	return box->count == 0;
}

/* -------------------------------------------------------------------------- */
static
bool priv_box_full( box_t *box )
/* -------------------------------------------------------------------------- */
{
	return box->count == box->limit;
}

/* -------------------------------------------------------------------------- */
static
void priv_box_dec( box_t *box )
/* -------------------------------------------------------------------------- */
{
	box->head = box->head + box->size < box->limit ? box->head + box->size : 0;
	box->count -= box->size;
}

/* -------------------------------------------------------------------------- */
static
void priv_box_inc( box_t *box )
/* -------------------------------------------------------------------------- */
{
	box->tail = box->tail + box->size < box->limit ? box->tail + box->size : 0;
	box->count += box->size;
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
void priv_box_getUpdate( box_t *box, char *data )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	priv_box_get(box, data);
	tsk = core_one_wakeup(box->obj.queue, E_SUCCESS);
	if (tsk)
		priv_box_put(box, tsk->tmp.box.data.out);
}

/* -------------------------------------------------------------------------- */
static
void priv_box_putUpdate( box_t *box, const char *data )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	priv_box_put(box, data);
	tsk = core_one_wakeup(box->obj.queue, E_SUCCESS);
	if (tsk)
		priv_box_get(box, tsk->tmp.box.data.in);
}

/* -------------------------------------------------------------------------- */
static
void priv_box_skipUpdate( box_t *box )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	while (priv_box_full(box))
	{
		priv_box_dec(box);
		tsk = core_one_wakeup(box->obj.queue, E_SUCCESS);
		if (tsk)
			priv_box_put(box, tsk->tmp.box.data.out);
	}
}

/* -------------------------------------------------------------------------- */
static
int priv_box_take( box_t *box, void *data )
/* -------------------------------------------------------------------------- */
{
	if (priv_box_empty(box))
		return E_TIMEOUT;

	priv_box_getUpdate(box, data);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int box_take( box_t *box, void *data )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(box);
	assert(box->obj.res!=RELEASED);
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
int box_waitFor( box_t *box, void *data, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		result = priv_box_take(box, data);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.box.data.in = data;
			result = core_tsk_waitFor(&box->obj.queue, delay);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int box_waitUntil( box_t *box, void *data, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		result = priv_box_take(box, data);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.box.data.in = data;
			result = core_tsk_waitUntil(&box->obj.queue, time);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
static
int priv_box_give( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	if (priv_box_full(box))
		return E_TIMEOUT;

	priv_box_putUpdate(box, data);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int box_give( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(box);
	assert(box->obj.res!=RELEASED);
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
int box_sendFor( box_t *box, const void *data, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		result = priv_box_give(box, data);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.box.data.out = data;
			result = core_tsk_waitFor(&box->obj.queue, delay);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int box_sendUntil( box_t *box, const void *data, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		result = priv_box_give(box, data);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.box.data.out = data;
			result = core_tsk_waitUntil(&box->obj.queue, time);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
void box_push( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		priv_box_skipUpdate(box);
		priv_box_putUpdate(box, data);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
unsigned box_count( box_t *box )
/* -------------------------------------------------------------------------- */
{
	unsigned count;

	assert(box);
	assert(box->obj.res!=RELEASED);

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
	assert(box->obj.res!=RELEASED);

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
	assert(box->obj.res!=RELEASED);

	sys_lock();
	{
		limit = box->limit / box->size;
	}
	sys_unlock();

	return limit;
}

/* -------------------------------------------------------------------------- */

#if OS_ATOMICS

/* -------------------------------------------------------------------------- */
static
bool priv_box_emptyAsync( box_t *box )
/* -------------------------------------------------------------------------- */
{
	unsigned count = atomic_load(&box->count);
	return count == 0;
}

/* -------------------------------------------------------------------------- */
static
bool priv_box_fullAsync( box_t *box )
/* -------------------------------------------------------------------------- */
{
	unsigned count = atomic_load(&box->count);
	return count == box->limit;
}

/* -------------------------------------------------------------------------- */
static
void priv_box_decAsync( box_t *box )
/* -------------------------------------------------------------------------- */
{
	box->head = box->head + box->size < box->limit ? box->head + box->size : 0;
	atomic_fetch_sub(&box->count, box->size);
}

/* -------------------------------------------------------------------------- */
static
void priv_box_incAsync( box_t *box )
/* -------------------------------------------------------------------------- */
{
	box->tail = box->tail + box->size < box->limit ? box->tail + box->size : 0;
	atomic_fetch_add(&box->count, box->size);
}

/* -------------------------------------------------------------------------- */
static
void priv_box_getAsync( box_t *box, char *data )
/* -------------------------------------------------------------------------- */
{
	memcpy(data, &box->data[box->head], box->size);
	priv_box_decAsync(box);
}

/* -------------------------------------------------------------------------- */
static
void priv_box_putAsync( box_t *box, const char *data )
/* -------------------------------------------------------------------------- */
{
	memcpy(&box->data[box->tail], data, box->size);
	priv_box_incAsync(box);
}

/* -------------------------------------------------------------------------- */
static
int priv_box_takeAsync( box_t *box, void *data )
/* -------------------------------------------------------------------------- */
{
	if (priv_box_emptyAsync(box))
		return E_TIMEOUT;

	priv_box_getAsync(box, data);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int box_takeAsync( box_t *box, void *data )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		result = priv_box_takeAsync(box, data);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int box_waitAsync( box_t *box, void *data )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();

	while (box_takeAsync(box, data) != E_SUCCESS)
		core_ctx_switchNow();

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
static
int priv_box_giveAsync( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	if (priv_box_fullAsync(box))
		return E_TIMEOUT;

	priv_box_putAsync(box, data);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int box_giveAsync( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		result = priv_box_giveAsync(box, data);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int box_sendAsync( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();

	while (box_giveAsync(box, data) != E_SUCCESS)
		core_ctx_switchNow();

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */

#endif//OS_ATOMICS
