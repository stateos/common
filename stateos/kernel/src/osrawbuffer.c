/******************************************************************************

    @file    StateOS: osrawbuffer.c
    @author  Rajmund Szymanski
    @date    18.05.2021
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

#include "inc/osrawbuffer.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_raw_init( raw_t *raw, void *data, size_t bufsize, void *res )
/* -------------------------------------------------------------------------- */
{
	memset(raw, 0, sizeof(raw_t));

	core_obj_init(&raw->obj, res);

	raw->limit = bufsize;
	raw->data  = data;
}

/* -------------------------------------------------------------------------- */
void raw_init( raw_t *raw, void *data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(raw);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		priv_raw_init(raw, data, bufsize, NULL);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
raw_t *raw_create( size_t limit )
/* -------------------------------------------------------------------------- */
{
	struct raw_T { raw_t raw; char buf[]; } *tmp;
	raw_t *raw = NULL;
	size_t bufsize;

	assert_tsk_context();
	assert(limit);

	sys_lock();
	{
		bufsize = limit;
		tmp = malloc(sizeof(struct raw_T) + bufsize);
		if (tmp)
			priv_raw_init(raw = &tmp->raw, tmp->buf, bufsize, tmp);
	}
	sys_unlock();

	return raw;
}

/* -------------------------------------------------------------------------- */
static
void priv_raw_reset( raw_t *raw, int event )
/* -------------------------------------------------------------------------- */
{
	raw->count = 0;
	raw->head  = 0;
	raw->tail  = 0;

	core_all_wakeup(raw->obj.queue, event);
}

/* -------------------------------------------------------------------------- */
void raw_reset( raw_t *raw )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(raw);
	assert(raw->obj.res!=RELEASED);

	sys_lock();
	{
		priv_raw_reset(raw, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void raw_destroy( raw_t *raw )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(raw);
	assert(raw->obj.res!=RELEASED);

	sys_lock();
	{
		priv_raw_reset(raw, raw->obj.res ? E_DELETED : E_STOPPED);
		core_res_free(&raw->obj);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
void priv_raw_get( raw_t *raw, char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t head = raw->head;

	raw->count -= size;
	while (size--)
	{
		*data++ = raw->data[head++];
		if (head == raw->limit) head = 0;
	}
	raw->head = head;
}

/* -------------------------------------------------------------------------- */
static
void priv_raw_put( raw_t *raw, const char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t tail = raw->tail;

	raw->count += size;
	while (size--)
	{
		raw->data[tail++] = *data++;
		if (tail == raw->limit) tail = 0;
	}
	raw->tail = tail;
}

/* -------------------------------------------------------------------------- */
static
void priv_raw_skip( raw_t *raw, size_t size )
/* -------------------------------------------------------------------------- */
{
	raw->count -= size;
	raw->head  += size;
	if (raw->head >= raw->limit) raw->head -= raw->limit;
}

/* -------------------------------------------------------------------------- */
static
size_t priv_raw_getUpdate( raw_t *raw, char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (size > raw->count) size = raw->count;
	priv_raw_get(raw, data, size);

	while (raw->obj.queue != 0 && raw->count + raw->obj.queue->tmp.raw.size <= raw->limit)
	{
		priv_raw_put(raw, raw->obj.queue->tmp.raw.data.out, raw->obj.queue->tmp.raw.size);
		core_one_wakeup(raw->obj.queue, E_SUCCESS);
	}

	return size;
}

/* -------------------------------------------------------------------------- */
static
void priv_raw_putUpdate( raw_t *raw, const char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	priv_raw_put(raw, data, size);

	while (raw->obj.queue != 0 && raw->count > 0)
	{
		size = raw->obj.queue->tmp.raw.size;
		if (size > raw->count) size = raw->count;
		raw->obj.queue->tmp.raw.size = size;
		priv_raw_get(raw, raw->obj.queue->tmp.raw.data.in, size);
		core_one_wakeup(raw->obj.queue, E_SUCCESS);
	}
}

/* -------------------------------------------------------------------------- */
static
void priv_raw_skipUpdate( raw_t *raw, size_t size )
/* -------------------------------------------------------------------------- */
{
	while (raw->obj.queue != 0)
	{
		if (raw->count + raw->obj.queue->tmp.raw.size > raw->limit)
			priv_raw_skip(raw, raw->count + raw->obj.queue->tmp.raw.size - raw->limit);
		priv_raw_put(raw, raw->obj.queue->tmp.raw.data.out, raw->obj.queue->tmp.raw.size);
		core_one_wakeup(raw->obj.queue, E_SUCCESS);
	}

	if (raw->count + size > raw->limit)
		priv_raw_skip(raw, raw->count + size - raw->limit);
}

/* -------------------------------------------------------------------------- */
static
int priv_raw_take( raw_t *raw, char *data, size_t size, size_t *read )
/* -------------------------------------------------------------------------- */
{
	if (raw->count == 0)
		return E_TIMEOUT;

	size = priv_raw_getUpdate(raw, data, size);
	if (read != NULL)
		*read = size;

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int raw_take( raw_t *raw, void *data, size_t size, size_t *read )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(raw);
	assert(raw->obj.res!=RELEASED);
	assert(raw->data);
	assert(raw->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_raw_take(raw, data, size, read);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int raw_waitFor( raw_t *raw, void *data, size_t size, size_t *read, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(raw);
	assert(raw->obj.res!=RELEASED);
	assert(raw->data);
	assert(raw->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_raw_take(raw, data, size, read);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.raw.data.in = data;
			System.cur->tmp.raw.size = size;
			result = core_tsk_waitFor(&raw->obj.queue, delay);
			if (result == E_SUCCESS && read != NULL)
				*read = System.cur->tmp.raw.size;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int raw_waitUntil( raw_t *raw, void *data, size_t size, size_t *read, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(raw);
	assert(raw->obj.res!=RELEASED);
	assert(raw->data);
	assert(raw->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_raw_take(raw, data, size, read);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.raw.data.in = data;
			System.cur->tmp.raw.size = size;
			result = core_tsk_waitUntil(&raw->obj.queue, time);
			if (result == E_SUCCESS && read != NULL)
				*read = System.cur->tmp.raw.size;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
static
int priv_raw_give( raw_t *raw, const char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (size > raw->limit)
		return E_FAILURE;

	if (raw->count + size > raw->limit)
		return E_TIMEOUT;

	priv_raw_putUpdate(raw, data, size);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int raw_give( raw_t *raw, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(raw);
	assert(raw->obj.res!=RELEASED);
	assert(raw->data);
	assert(raw->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_raw_give(raw, data, size);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int raw_sendFor( raw_t *raw, const void *data, size_t size, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(raw);
	assert(raw->obj.res!=RELEASED);
	assert(raw->data);
	assert(raw->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_raw_give(raw, data, size);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.raw.data.out = data;
			System.cur->tmp.raw.size = size;
			result = core_tsk_waitFor(&raw->obj.queue, delay);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int raw_sendUntil( raw_t *raw, const void *data, size_t size, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(raw);
	assert(raw->obj.res!=RELEASED);
	assert(raw->data);
	assert(raw->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_raw_give(raw, data, size);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.raw.data.out = data;
			System.cur->tmp.raw.size = size;
			result = core_tsk_waitUntil(&raw->obj.queue, time);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
static
int priv_raw_push( raw_t *raw, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (size > raw->limit)
		return E_FAILURE;

	priv_raw_skipUpdate(raw, size);
	priv_raw_putUpdate(raw, data, size);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int raw_push( raw_t *raw, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(raw);
	assert(raw->obj.res!=RELEASED);
	assert(raw->data);
	assert(raw->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_raw_push(raw, data, size);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
size_t raw_count( raw_t *raw )
/* -------------------------------------------------------------------------- */
{
	size_t count;

	assert(raw);
	assert(raw->obj.res!=RELEASED);

	sys_lock();
	{
		count = raw->count;
	}
	sys_unlock();

	return count;
}

/* -------------------------------------------------------------------------- */
size_t raw_space( raw_t *raw )
/* -------------------------------------------------------------------------- */
{
	size_t space;

	assert(raw);
	assert(raw->obj.res!=RELEASED);

	sys_lock();
	{
		space = raw->limit - raw->count;
	}
	sys_unlock();

	return space;
}

/* -------------------------------------------------------------------------- */
size_t raw_limit( raw_t *raw )
/* -------------------------------------------------------------------------- */
{
	size_t limit;

	assert(raw);
	assert(raw->obj.res!=RELEASED);

	sys_lock();
	{
		limit = raw->limit;
	}
	sys_unlock();

	return limit;
}

/* -------------------------------------------------------------------------- */
