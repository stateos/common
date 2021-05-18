/******************************************************************************

    @file    IntrOS: osrawbuffer.c
    @author  Rajmund Szymanski
    @date    07.05.2021
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

#include "inc/osrawbuffer.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
void raw_init( raw_t *raw, void *data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert(raw);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		memset(raw, 0, sizeof(raw_t));

		raw->limit = bufsize;
		raw->data  = data;
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
	return size;
}

/* -------------------------------------------------------------------------- */
static
void priv_raw_putUpdate( raw_t *raw, const char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	priv_raw_put(raw, data, size);
}

/* -------------------------------------------------------------------------- */
static
void priv_raw_skipUpdate( raw_t *raw, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (raw->count + size > raw->limit)
		priv_raw_skip(raw, raw->count + size - raw->limit);
}

/* -------------------------------------------------------------------------- */
static
size_t priv_raw_take( raw_t *raw, void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (raw->count == 0)
		return 0;

	return priv_raw_getUpdate(raw, data, size);
}

/* -------------------------------------------------------------------------- */
size_t raw_take( raw_t *raw, void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t result;

	assert(raw);
	assert(raw->data);
	assert(raw->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_raw_take(raw, data, size);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
size_t raw_wait( raw_t *raw, void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t result;

	while (result = raw_take(raw, data, size), result == 0)
		core_ctx_switch();

	return result;
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_raw_give( raw_t *raw, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (raw->count + size > raw->limit)
		return FAILURE;

	priv_raw_putUpdate(raw, data, size);

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned raw_give( raw_t *raw, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	assert(raw);
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
unsigned raw_send( raw_t *raw, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (size > raw->limit)
		return FAILURE;

	while (raw_give(raw, data, size) != SUCCESS)
		core_ctx_switch();

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_raw_push( raw_t *raw, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (size > raw->limit)
		return FAILURE;

	priv_raw_skipUpdate(raw, size);
	priv_raw_putUpdate(raw, data, size);

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned raw_push( raw_t *raw, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	assert(raw);
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

	sys_lock();
	{
		limit = raw->limit;
	}
	sys_unlock();

	return limit;
}

/* -------------------------------------------------------------------------- */
