/******************************************************************************

    @file    IntrOS: osmemorypool.c
    @author  Rajmund Szymanski
    @date    30.04.2021
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
	que_t *ptr = mem->data;

	assert(mem->size);
	assert(mem->data);

	do
	{
		mem_give(mem, ++ptr);
		ptr += mem->size;
	}
	while (--mem->limit);
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

		mem->limit = bufsize / (1 + MEM_SIZE(size)) / sizeof(que_t);
		mem->size  = MEM_SIZE(size);
		mem->data  = data;
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void *mem_take( mem_t *mem )
/* -------------------------------------------------------------------------- */
{
	void *result = NULL;

	assert(mem);

	sys_lock();
	{
		if (mem->limit)
			priv_mem_bind(mem);

		if (mem->lst.head.next)
		{
			result = mem->lst.head.next + 1;
			mem->lst.head.next = mem->lst.head.next->next;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
void *mem_wait( mem_t *mem )
/* -------------------------------------------------------------------------- */
{
	void *result;

	while (result = mem_take(mem), result == NULL) core_ctx_switch();

	return result;
}

/* -------------------------------------------------------------------------- */
