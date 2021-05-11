/******************************************************************************

    @file    StateOS: osmessagequeue.c
    @author  Rajmund Szymanski
    @date    11.05.2021
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

#include "inc/osmessagequeue.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

typedef struct __mqh mqh_t;

struct __mqh         // message queue header
{
	__PACKED
	size_t   size;   // size of the message
	char     data[]; // message data
};

/* -------------------------------------------------------------------------- */
static
void priv_msg_init( msg_t *msg, size_t size, void *data, size_t bufsize, void *res )
/* -------------------------------------------------------------------------- */
{
	memset(msg, 0, sizeof(msg_t));

	core_obj_init(&msg->obj, res);

	msg->data  = data;
	msg->size  = sizeof(size_t) + size;
	msg->limit = (bufsize / msg->size) * msg->size;
}

/* -------------------------------------------------------------------------- */
void msg_init( msg_t *msg, size_t size, void *data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(msg);
	assert(size);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		priv_msg_init(msg, size, data, bufsize, NULL);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
msg_t *msg_create( unsigned limit, size_t size )
/* -------------------------------------------------------------------------- */
{
	msg_t *msg;
	size_t bufsize;

	assert_tsk_context();
	assert(limit);
	assert(size);

	sys_lock();
	{
		bufsize = limit * (sizeof(size_t) + size);
		msg = malloc(sizeof(msg_t) + bufsize);
		if (msg)
			priv_msg_init(msg, size, msg->buffer, bufsize, msg);
	}
	sys_unlock();

	return msg;
}
/* -------------------------------------------------------------------------- */
static
void priv_msg_reset( msg_t *msg, int event )
/* -------------------------------------------------------------------------- */
{
	msg->count = 0;
	msg->head  = 0;
	msg->tail  = 0;

	core_all_wakeup(msg->obj.queue, event);
}


/* -------------------------------------------------------------------------- */
void msg_reset( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(msg);
	assert(msg->obj.res!=RELEASED);

	sys_lock();
	{
		priv_msg_reset(msg, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void msg_destroy( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(msg);
	assert(msg->obj.res!=RELEASED);

	sys_lock();
	{
		priv_msg_reset(msg, msg->obj.res ? E_DELETED : E_STOPPED);
		core_res_free(&msg->obj);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
size_t priv_msg_getSize( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	return ((mqh_t *)(msg->data + msg->head))->size;
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_putSize( msg_t *msg, size_t size )
/* -------------------------------------------------------------------------- */
{
	((mqh_t *)(msg->data + msg->tail))->size = size;
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_get( msg_t *msg, char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t head = msg->head + sizeof(size_t);

	while (size--)
		*data++ = msg->data[head++];

	msg->head = head < msg->limit ? head : 0;
	msg->count -= msg->size;
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_put( msg_t *msg, const char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t tail = msg->tail + sizeof(size_t);

	while (size--)
		msg->data[tail++] = *data++;

	msg->tail = tail < msg->limit ? tail : 0;
	msg->count += msg->size;
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_skip( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	size_t head = msg->head + msg->size;

	msg->head = head < msg->limit ? head : 0;
	msg->count -= msg->size;
}

/* -------------------------------------------------------------------------- */
static
size_t priv_msg_getUpdate( msg_t *msg, char *data )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;
	size_t size;

	size = priv_msg_getSize(msg);
	priv_msg_get(msg, data, size);
	tsk = core_one_wakeup(msg->obj.queue, E_SUCCESS);
	if (tsk)
	{
		priv_msg_putSize(msg, tsk->tmp.msg.size);
		priv_msg_put(msg, tsk->tmp.msg.data.out, tsk->tmp.msg.size);
	}

	return size;
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_putUpdate( msg_t *msg, const char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	priv_msg_putSize(msg, size);
	priv_msg_put(msg, data, size);
	tsk = core_one_wakeup(msg->obj.queue, E_SUCCESS);
	if (tsk)
	{
		tsk->tmp.msg.size = size;
		priv_msg_get(msg, tsk->tmp.msg.data.in, size);
	}
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_skipUpdate( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	while (msg->count == msg->limit)
	{
		priv_msg_skip(msg);
		tsk = core_one_wakeup(msg->obj.queue, E_SUCCESS);
		if (tsk)
		{
			priv_msg_putSize(msg, tsk->tmp.msg.size);
			priv_msg_put(msg, tsk->tmp.msg.data.out, tsk->tmp.msg.size);
		}
	}
}

/* -------------------------------------------------------------------------- */
static
int priv_msg_take( msg_t *msg, char *data, size_t size, size_t *read )
/* -------------------------------------------------------------------------- */
{
	if (sizeof(size_t) + size < msg->size)
		return E_FAILURE;

	if (msg->count == 0)
		return E_TIMEOUT;

	size = priv_msg_getUpdate(msg, data);
	if (read != NULL)
		*read = size;

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int msg_take( msg_t *msg, void *data, size_t size, size_t *read )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(msg);
	assert(msg->obj.res!=RELEASED);
	assert(msg->data);
	assert(msg->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_msg_take(msg, data, size, read);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int msg_waitFor( msg_t *msg, void *data, size_t size, size_t *read, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(msg);
	assert(msg->obj.res!=RELEASED);
	assert(msg->data);
	assert(msg->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_msg_take(msg, data, size, read);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.msg.data.in = data;
			System.cur->tmp.msg.size = size;
			result = core_tsk_waitFor(&msg->obj.queue, delay);
			if (result == E_SUCCESS && read != NULL)
				*read = System.cur->tmp.msg.size;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int msg_waitUntil( msg_t *msg, void *data, size_t size, size_t *read, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(msg);
	assert(msg->obj.res!=RELEASED);
	assert(msg->data);
	assert(msg->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_msg_take(msg, data, size, read);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.msg.data.in = data;
			System.cur->tmp.msg.size = size;
			result = core_tsk_waitUntil(&msg->obj.queue, time);
			if (result == E_SUCCESS && read != NULL)
				*read = System.cur->tmp.msg.size;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
static
int priv_msg_give( msg_t *msg, const char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (sizeof(size_t) + size > msg->size)
		return E_FAILURE;

	if (msg->count == msg->limit)
		return E_TIMEOUT;

	priv_msg_putUpdate(msg, data, size);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int msg_give( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(msg);
	assert(msg->obj.res!=RELEASED);
	assert(msg->data);
	assert(msg->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_msg_give(msg, data, size);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int msg_sendFor( msg_t *msg, const void *data, size_t size, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(msg);
	assert(msg->obj.res!=RELEASED);
	assert(msg->data);
	assert(msg->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_msg_give(msg, data, size);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.msg.data.out = data;
			System.cur->tmp.msg.size = size;
			result = core_tsk_waitFor(&msg->obj.queue, delay);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int msg_sendUntil( msg_t *msg, const void *data, size_t size, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(msg);
	assert(msg->obj.res!=RELEASED);
	assert(msg->data);
	assert(msg->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_msg_give(msg, data, size);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.msg.data.out = data;
			System.cur->tmp.msg.size = size;
			result = core_tsk_waitUntil(&msg->obj.queue, time);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
static
int priv_msg_push( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (sizeof(size_t) + size > msg->limit)
		return E_FAILURE;

	priv_msg_skipUpdate(msg);
	priv_msg_putUpdate(msg, data, size);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int msg_push( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(msg);
	assert(msg->obj.res!=RELEASED);
	assert(msg->data);
	assert(msg->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_msg_push(msg, data, size);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
size_t msg_count( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	size_t count;

	assert(msg);
	assert(msg->obj.res!=RELEASED);

	sys_lock();
	{
		count = msg->count / msg->size;
	}
	sys_unlock();

	return count;
}

/* -------------------------------------------------------------------------- */
size_t msg_space( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	size_t space;

	assert(msg);
	assert(msg->obj.res!=RELEASED);

	sys_lock();
	{
		space = (msg->limit - msg->count) / msg->size;
	}
	sys_unlock();

	return space;
}

/* -------------------------------------------------------------------------- */
size_t msg_limit( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	size_t limit;

	assert(msg);
	assert(msg->obj.res!=RELEASED);

	sys_lock();
	{
		limit = msg->limit / msg->size;
	}
	sys_unlock();

	return limit;
}

/* -------------------------------------------------------------------------- */

#if OS_ATOMICS

/* -------------------------------------------------------------------------- */
static
void priv_msg_getAsync( msg_t *msg, char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t head = msg->head + sizeof(size_t);

	while (size--)
		*data++ = msg->data[head++];

	msg->head = head < msg->limit ? head : 0;
	atomic_fetch_sub((atomic_size_t *)&msg->count, msg->size);
}

/* -------------------------------------------------------------------------- */
static
int priv_msg_takeAsync( msg_t *msg, void *data, size_t size, size_t *read )
/* -------------------------------------------------------------------------- */
{
	if (sizeof(size_t) + size < msg->size)
		return E_FAILURE;

	if (atomic_load((atomic_size_t *)&msg->count) == 0)
		return E_TIMEOUT;

	size = priv_msg_getSize(msg);
	priv_msg_getAsync(msg, data, size);
	if (read != NULL)
		*read = size;

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int msg_takeAsync( msg_t *msg, void *data, size_t size, size_t *read )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(msg);
	assert(msg->data);
	assert(msg->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_msg_takeAsync(msg, data, size, read);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int msg_waitAsync( msg_t *msg, void *data, size_t size, size_t *read )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();

	while (result = msg_takeAsync(msg, data, size, read), result == E_TIMEOUT)
		core_ctx_switch();

	return result;
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_putAsync( msg_t *msg, const char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t tail = msg->tail + sizeof(size_t);

	while (size--)
		msg->data[tail++] = *data++;

	msg->tail = tail < msg->limit ? tail : 0;
	atomic_fetch_add((atomic_size_t *)&msg->count, msg->size);
}

/* -------------------------------------------------------------------------- */
static
int priv_msg_giveAsync( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (sizeof(size_t) + size > msg->size)
		return E_FAILURE;

	if (atomic_load((atomic_size_t *)&msg->count) == msg->limit)
		return E_TIMEOUT;

	priv_msg_putSize(msg, size);
	priv_msg_putAsync(msg, data, size);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int msg_giveAsync( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(msg);
	assert(msg->data);
	assert(msg->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_msg_giveAsync(msg, data, size);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int msg_sendAsync( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();

	while (result = msg_giveAsync(msg, data, size), result == E_TIMEOUT)
		core_ctx_switch();

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */

#endif//OS_ATOMICS
