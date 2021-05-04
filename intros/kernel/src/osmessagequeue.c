/******************************************************************************

    @file    IntrOS: osmessagequeue.c
    @author  Rajmund Szymanski
    @date    04.05.2021
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

#include "inc/osmessagequeue.h"
#include "inc/oscriticalsection.h"

typedef struct __mqh mqh_t;

struct __mqh         // message queue header
{
	__PACKED
	size_t   size;   // size of the message
	char     data[]; // message data
};

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
		msg->size  = sizeof(size_t) + size;
		msg->limit = (bufsize / msg->size) * msg->size;
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
size_t priv_msg_take( msg_t *msg, void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (msg->count == 0)
		return 0;

	size = priv_msg_getSize(msg);
	priv_msg_get(msg, data, size);

	return size;
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

	if (sizeof(size_t) + size < msg->size)
		return 0;

	sys_lock();
	{
		result = priv_msg_take(msg, data, size);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
size_t msg_wait( msg_t *msg, void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t result;

	if (sizeof(size_t) + size < msg->size)
		return 0;

	while (result = msg_take(msg, data, size), result == 0)
		core_ctx_switch();

	return result;
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_msg_give( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (msg->count == msg->limit)
		return FAILURE;

	priv_msg_putSize(msg, size);
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

	if (sizeof(size_t) + size > msg->size)
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
	if (sizeof(size_t) + size > msg->size)
		return FAILURE;

	while (msg_give(msg, data, size) != SUCCESS)
		core_ctx_switch();

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

	if (sizeof(size_t) + size > msg->size)
		return FAILURE;

	sys_lock();
	{
		if (msg->count == msg->limit)
			priv_msg_skip(msg);
		priv_msg_putSize(msg, size);
		priv_msg_put(msg, data, size);
	}
	sys_unlock();

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned msg_count( msg_t *msg )
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
unsigned msg_space( msg_t *msg )
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
unsigned msg_limit( msg_t *msg )
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
size_t priv_msg_takeAsync( msg_t *msg, void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (atomic_load((atomic_size_t *)&msg->count) == 0)
		return 0;

	size = priv_msg_getSize(msg);
	priv_msg_getAsync(msg, data, size);

	return size;
}

/* -------------------------------------------------------------------------- */
size_t msg_takeAsync( msg_t *msg, void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t result;

	assert(msg);
	assert(msg->data);
	assert(msg->limit);
	assert(data);
	assert(size);

	if (sizeof(size_t) + size < msg->size)
		return 0;

	sys_lock();
	{
		result = priv_msg_takeAsync(msg, data, size);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
size_t msg_waitAsync( msg_t *msg, void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t result;

	if (sizeof(size_t) + size < msg->size)
		return 0;

	while (result = msg_takeAsync(msg, data, size), result == 0)
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
unsigned priv_msg_giveAsync( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (atomic_load((atomic_size_t *)&msg->count) == msg->limit)
		return FAILURE;

	priv_msg_putSize(msg, size);
	priv_msg_putAsync(msg, data, size);

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned msg_giveAsync( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	assert(msg);
	assert(msg->data);
	assert(msg->limit);
	assert(data);
	assert(size);

	if (sizeof(size_t) + size > msg->size)
		return FAILURE;

	sys_lock();
	{
		result = priv_msg_giveAsync(msg, data, size);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
unsigned msg_sendAsync( msg_t *msg, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (sizeof(size_t) + size > msg->size)
		return FAILURE;

	while (msg_giveAsync(msg, data, size) != SUCCESS)
		core_ctx_switch();

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */

#endif//OS_ATOMICS
