/******************************************************************************

    @file    IntrOS: osmemorypool.c
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

#include "inc/osmemorypool.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_mem_bind( mem_t *mem )
/* -------------------------------------------------------------------------- */
{
	que_t *ptr = mem->lst.head.next = mem->data;
	size_t cnt = mem->limit;

	assert(mem->size);
	assert(mem->data);

	while (--cnt > 0)
		ptr = ptr->next = ptr + 1 + mem->size;

	ptr->next = NULL;
	mem->limit = 0;
}

/* -------------------------------------------------------------------------- */
void mem_init( mem_t *mem, size_t size, que_t *data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert(mem);
	assert(size);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		memset(mem, 0, sizeof(mem_t));

		mem->data  = data;
		mem->size  = MEM_SIZE(size);
		mem->limit = bufsize / (1 + mem->size) / sizeof(que_t);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
void *priv_lst_popFront( lst_t *lst )
/* -------------------------------------------------------------------------- */
{
	void *data = lst->head.next + 1;
	lst->head.next = lst->head.next->next;
	return data;
}

/* -------------------------------------------------------------------------- */
static
void *priv_mem_take( mem_t *mem )
/* -------------------------------------------------------------------------- */
{
	if (mem->limit)
		priv_mem_bind(mem);

	if (mem->lst.head.next == NULL)
		return NULL;

	return priv_lst_popFront(&mem->lst);
}

/* -------------------------------------------------------------------------- */
void *mem_take( mem_t *mem )
/* -------------------------------------------------------------------------- */
{
	void *result;

	assert(mem);

	sys_lock();
	{
		result = priv_mem_take(mem);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
void *mem_wait( mem_t *mem )
/* -------------------------------------------------------------------------- */
{
	void *result;

	while (result = mem_take(mem), result == NULL)
		core_ctx_switch();

	return result;
}

/* -------------------------------------------------------------------------- */
static
void priv_lst_pushBack( lst_t *lst, void *data )
/* -------------------------------------------------------------------------- */
{
	que_t *ptr = &lst->head;
	while (ptr->next) ptr = ptr->next;
	ptr = ptr->next = (que_t *)data - 1;
	ptr->next = NULL;
}

/* -------------------------------------------------------------------------- */
static
void priv_mem_give( mem_t *mem, void *data )
/* -------------------------------------------------------------------------- */
{
	if (mem->limit)
		priv_mem_bind(mem);

	priv_lst_pushBack(&mem->lst, data);
}

/* -------------------------------------------------------------------------- */
void mem_give( mem_t *mem, void *data )
/* -------------------------------------------------------------------------- */
{
	assert(mem);
	assert(data);

	sys_lock();
	{
		priv_mem_give(mem, data);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
