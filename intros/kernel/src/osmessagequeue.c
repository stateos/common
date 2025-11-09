/******************************************************************************

    @file    IntrOS: osmessagequeue.c
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

#include "inc/osmessagequeue.h"
#include "inc/oscriticalsection.h"
#include "inc/ostask.h"

/* -------------------------------------------------------------------------- */
void msg_init( msg_t *msg, size_t size, void *data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert(msg);
	assert(size);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		memset(msg, 0, sizeof(msg_t));

		msg->data  = data;
		msg->size  = MSG_SIZE(size);
		msg->limit = (bufsize / msg->size) * msg->size;
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
bool priv_msg_empty( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
#if OS_ATOMICS
	return atomic_load((atomic_size_t *)&msg->count) == 0;
#else
	return msg->count == 0;
#endif
}

/* -------------------------------------------------------------------------- */
static
bool priv_msg_full( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
#if OS_ATOMICS
	return atomic_load((atomic_size_t *)&msg->count) == msg->limit;
#else
	return msg->count == msg->limit;
#endif
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_dec( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	msg->head = msg->head + msg->size < msg->limit ? msg->head + msg->size : 0;
#if OS_ATOMICS
	atomic_fetch_sub((atomic_size_t *)&msg->count, msg->size);
#else
	msg->count -= msg->size;
#endif
}

/* -------------------------------------------------------------------------- */
static
void priv_msg_inc( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	msg->tail = msg->tail + msg->size < msg->limit ? msg->tail + msg->size : 0;
#if OS_ATOMICS
	atomic_fetch_add((atomic_size_t *)&msg->count, msg->size);
#else
	msg->count += msg->size;
#endif
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
size_t priv_msg_take( msg_t *msg, void *data )
/* -------------------------------------------------------------------------- */
{
	if (priv_msg_empty(msg))
		return 0;

	return priv_msg_get(msg, data);
}

/* -------------------------------------------------------------------------- */
size_t msg_take( msg_t *msg, void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t result;

	assert(msg);
	assert(msg->data);
	assert(msg->limit);
	assert(data);
	assert(size);

	if (MSG_SIZE(size) < msg->size)
		return 0;

	sys_lock();
	{
		result = priv_msg_take(msg, data);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
size_t msg_wait( msg_t *msg, void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t result;

	if (MSG_SIZE(size) < msg->size)
		return 0;

	while (result = msg_take(msg, data, size), result == 0)
		tsk_yield();

	return result;
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_msg_give( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (priv_msg_full(msg))
		return FAILURE;

	priv_msg_put(msg, data, size);

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned msg_give( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	assert(msg);
	assert(msg->data);
	assert(msg->limit);
	assert(data);
	assert(size);

	if (MSG_SIZE(size) > msg->size)
		return FAILURE;

	sys_lock();
	{
		result = priv_msg_give(msg, data, size);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
unsigned msg_send( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (MSG_SIZE(size) > msg->size)
		return FAILURE;

	while (msg_give(msg, data, size) != SUCCESS)
		tsk_yield();

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned msg_push( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	assert(msg);
	assert(msg->data);
	assert(msg->limit);
	assert(data);
	assert(size);

	if (MSG_SIZE(size) > msg->size)
		return FAILURE;

	sys_lock();
	{
		if (priv_msg_full(msg))
			priv_msg_dec(msg);
		priv_msg_put(msg, data, size);
	}
	sys_unlock();

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned msg_getCount( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	size_t count;

	assert(msg);

	sys_lock();
	{
		count = msg->count / msg->size;
	}
	sys_unlock();

	return count;
}

/* -------------------------------------------------------------------------- */
unsigned msg_getSpace( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	size_t space;

	assert(msg);

	sys_lock();
	{
		space = (msg->limit - msg->count) / msg->size;
	}
	sys_unlock();

	return space;
}

/* -------------------------------------------------------------------------- */
unsigned msg_getLimit( msg_t *msg )
/* -------------------------------------------------------------------------- */
{
	size_t limit;

	assert(msg);

	sys_lock();
	{
		limit = msg->limit / msg->size;
	}
	sys_unlock();

	return limit;
}

/* -------------------------------------------------------------------------- */
