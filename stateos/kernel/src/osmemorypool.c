/******************************************************************************

    @file    StateOS: osmemorypool.c
    @author  Rajmund Szymanski
    @date    30.04.2021
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

#include "inc/osmemorypool.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_mem_bind( mem_t *mem )
/* -------------------------------------------------------------------------- */
{
	que_t *ptr = mem->data;

	assert(mem->size);
	assert(mem->data);

	mem->lst.head.next = 0;
	do
	{
		mem_give(mem, ++ptr);
		ptr += mem->size;
	}
	while (--mem->limit);
}

/* -------------------------------------------------------------------------- */
static
void priv_mem_init( mem_t *mem, size_t size, que_t *data, size_t bufsize, void *res )
/* -------------------------------------------------------------------------- */
{
	memset(mem, 0, sizeof(mem_t));

	core_obj_init(&mem->lst.obj, res);

	mem->limit = bufsize / (1 + MEM_SIZE(size)) / sizeof(que_t);
	mem->size  = MEM_SIZE(size);
	mem->data  = data;
}

/* -------------------------------------------------------------------------- */
void mem_init( mem_t *mem, size_t size, que_t *data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(mem);
	assert(size);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		priv_mem_init(mem, size, data, bufsize, NULL);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
mem_t *mem_create( unsigned limit, size_t size )
/* -------------------------------------------------------------------------- */
{
	struct mem_T { mem_t mem; que_t buf[]; } *tmp;
	mem_t *mem = NULL;
	size_t bufsize;

	assert_tsk_context();
	assert(limit);
	assert(size);

	sys_lock();
	{
		bufsize = limit * (1 + MEM_SIZE(size)) * sizeof(que_t);
		tmp = malloc(sizeof(struct mem_T) + bufsize);
		if (tmp)
			priv_mem_init(mem = &tmp->mem, size, tmp->buf, bufsize, tmp);
	}
	sys_unlock();

	return mem;
}

/* -------------------------------------------------------------------------- */
static
void priv_mem_reset( mem_t *mem, int event )
/* -------------------------------------------------------------------------- */
{
	core_all_wakeup(mem->lst.obj.queue, event);
}

/* -------------------------------------------------------------------------- */
void mem_reset( mem_t *mem )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(mem);
	assert(mem->lst.obj.res!=RELEASED);

	sys_lock();
	{
		priv_mem_reset(mem, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void mem_destroy( mem_t *mem )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(mem);
	assert(mem->lst.obj.res!=RELEASED);

	sys_lock();
	{
		priv_mem_reset(mem, mem->lst.obj.res ? E_DELETED : E_STOPPED);
		core_res_free(&mem->lst.obj);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
int priv_mem_take( mem_t *mem, void **data )
/* -------------------------------------------------------------------------- */
{
	if (mem->limit)
		priv_mem_bind(mem);

	if (mem->lst.head.next != NULL)
	{
		*data = mem->lst.head.next + 1;
		mem->lst.head.next = mem->lst.head.next->next;
		return E_SUCCESS;
	}

	return E_TIMEOUT;
}

/* -------------------------------------------------------------------------- */
int mem_take( mem_t *mem, void **data )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(mem);
	assert(mem->lst.obj.res!=RELEASED);
	assert(data);

	sys_lock();
	{
		result = priv_mem_take(mem, data);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int mem_waitFor( mem_t *mem, void **data, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(mem);
	assert(mem->lst.obj.res!=RELEASED);
	assert(data);

	sys_lock();
	{
		result = priv_mem_take(mem, data);
		if (result == E_TIMEOUT)
		{
			result = core_tsk_waitFor(&mem->lst.obj.queue, delay);
			if (result == E_SUCCESS)
				*data = System.cur->tmp.lst.data;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int mem_waitUntil( mem_t *mem, void **data, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(mem);
	assert(mem->lst.obj.res!=RELEASED);
	assert(data);

	sys_lock();
	{
		result = priv_mem_take(mem, data);
		if (result == E_TIMEOUT)
		{
			result = core_tsk_waitUntil(&mem->lst.obj.queue, time);
			if (result == E_SUCCESS)
				*data = System.cur->tmp.lst.data;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
