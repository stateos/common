/******************************************************************************

    @file    StateOS: osmessagequeue.c
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

#include "inc/osmessagequeue.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_msg_init( msg_t *msg, size_t size, void *data, size_t bufsize, void *res )
/* -------------------------------------------------------------------------- */
{
	memset(msg, 0, sizeof(msg_t));

	core_obj_init(&msg->obj, res);

	msg->data  = data;
	msg->size  = MSG_SIZE(size);
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
	struct msg_T { msg_t msg; char buf[]; } *tmp;
	msg_t *msg = NULL;
	size_t bufsize;

	assert_tsk_context();
	assert(limit);
	assert(size);

	sys_lock();
	{
		bufsize = limit * MSG_SIZE(size);
		tmp = malloc(sizeof(struct msg_T) + bufsize);
		if (tmp)
			priv_msg_init(msg = &tmp->msg, size, tmp->buf, bufsize, tmp);
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
bool priv_msg_empty( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	return msg->count == 0;
}

/* -------------------------------------------------------------------------- */
static
bool priv_msg_full( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	return msg->count == msg->limit;
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_dec( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	msg->head = msg->head + msg->size < msg->limit ? msg->head + msg->size : 0;
	msg->count -= msg->size;
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_inc( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	msg->tail = msg->tail + msg->size < msg->limit ? msg->tail + msg->size : 0;
	msg->count += msg->size;
}

/* -------------------------------------------------------------------------- */
static
size_t priv_msg_get( msg_t *msg, char *data )
/* -------------------------------------------------------------------------- */
{
	size_t size;

	msh_t *msh = (msh_t *)&msg->data[msg->head];

	size = msh->size;
	memcpy(data, msh->data, size);

	priv_msg_dec(msg);

	return size;
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_put( msg_t *msg, const char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	msh_t *msh = (msh_t *)&msg->data[msg->tail];

	msh->size = size;
	memcpy(msh->data, data, size);

	priv_msg_inc(msg);
}

/* -------------------------------------------------------------------------- */
static
size_t priv_msg_getUpdate( msg_t *msg, char *data )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	size_t size = priv_msg_get(msg, data);
	tsk = core_one_wakeup(msg->obj.queue, E_SUCCESS);
	if (tsk)
		priv_msg_put(msg, tsk->tmp.msg.data.out, tsk->tmp.msg.size);

	return size;
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_putUpdate( msg_t *msg, const char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	priv_msg_put(msg, data, size);
	tsk = core_one_wakeup(msg->obj.queue, E_SUCCESS);
	if (tsk)
		tsk->tmp.msg.size = priv_msg_get(msg, tsk->tmp.msg.data.in);
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_skipUpdate( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	while (priv_msg_full(msg))
	{
		priv_msg_dec(msg);
		tsk = core_one_wakeup(msg->obj.queue, E_SUCCESS);
		if (tsk)
			priv_msg_put(msg, tsk->tmp.msg.data.out, tsk->tmp.msg.size);
	}
}

/* -------------------------------------------------------------------------- */
static
int priv_msg_take( msg_t *msg, char *data, size_t size, size_t *read )
/* -------------------------------------------------------------------------- */
{
	if (MSG_SIZE(size) < msg->size)
		return E_FAILURE;

	if (priv_msg_empty(msg))
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
	if (MSG_SIZE(size) > msg->size)
		return E_FAILURE;

	if (priv_msg_full(msg))
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
	if (MSG_SIZE(size) > msg->limit)
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
bool priv_msg_emptyAsync( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	return atomic_load((atomic_size_t *)&msg->count) == 0;
}

/* -------------------------------------------------------------------------- */
static
bool priv_msg_fullAsync( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	return atomic_load((atomic_size_t *)&msg->count) == msg->limit;
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_decAsync( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	msg->head = msg->head + msg->size < msg->limit ? msg->head + msg->size : 0;
	atomic_fetch_sub((atomic_size_t *)&msg->count, msg->size);
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_incAsync( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	msg->tail = msg->tail + msg->size < msg->limit ? msg->tail + msg->size : 0;
	atomic_fetch_add((atomic_size_t *)&msg->count, msg->size);
}

/* -------------------------------------------------------------------------- */
static
size_t priv_msg_getAsync( msg_t *msg, char *data )
/* -------------------------------------------------------------------------- */
{
	size_t size;

	msh_t *msh = (msh_t *)&msg->data[msg->head];

	size = msh->size;
	memcpy(data, msh->data, size);

	priv_msg_decAsync(msg);

	return size;
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_putAsync( msg_t *msg, const char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	msh_t *msh = (msh_t *)&msg->data[msg->tail];

	msh->size = size;
	memcpy(msh->data, data, size);

	priv_msg_incAsync(msg);
}

/* -------------------------------------------------------------------------- */
static
int priv_msg_takeAsync( msg_t *msg, void *data, size_t size, size_t *read )
/* -------------------------------------------------------------------------- */
{
	if (MSG_SIZE(size) < msg->size)
		return E_FAILURE;

	if (priv_msg_emptyAsync(msg))
		return E_TIMEOUT;

	size = priv_msg_getAsync(msg, data);
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
		core_ctx_switchNow();

	return result;
}

/* -------------------------------------------------------------------------- */
static
int priv_msg_giveAsync( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (MSG_SIZE(size) > msg->size)
		return E_FAILURE;

	if (priv_msg_fullAsync(msg))
		return E_TIMEOUT;

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
		core_ctx_switchNow();

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */

#endif//OS_ATOMICS
