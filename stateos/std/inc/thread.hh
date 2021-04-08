/******************************************************************************

    @file    StateOS: thread.hh
    @author  Rajmund Szymanski
    @date    08.04.2021
    @brief   This file contains definitions for StateOS.

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

#ifndef __STATEOS_THREAD_HH
#define __STATEOS_THREAD_HH

#include "inc/ostask.h"

struct ostask_t : public tsk_t
{
	void (*fun)(void *);
	void  *arg;
	stk_t  stk[];

	static
	void handler();
};

inline
void ostask_t::handler()
{
	ostask_t *task = reinterpret_cast<ostask_t *>(tsk_this());
	task->fun(task->arg);
}

static inline
int __gthread_create(__gthread_t *thread, void (*func)(void *), void *args)
{
	constexpr size_t offset = sizeof(ostask_t) - sizeof(tsk_t);
	ostask_t *task = static_cast<ostask_t *>(wrk_create(OS_MAIN_PRIO, ostask_t::handler, OS_STACK_SIZE + offset, false, false));
	if (task != nullptr)
	{
		task->fun = func;
		task->arg = args;
		core_stk_move(task, offset);
		tsk_start(task);
		*thread = task;
		return 0;
	}
	return 1;
}

static inline
int __gthread_join(__gthread_t thread, void **/*value_ptr*/)
{
	return tsk_join(thread);
}

static inline
int __gthread_detach(__gthread_t thread)
{
	return tsk_detach(thread);
}

static inline
int __gthread_equal(const __gthread_t t1, const __gthread_t t2)
{
	return t1 == t2 ? 0 : 1;
}

static inline
__gthread_t __gthread_self()
{
	return reinterpret_cast<ostask_t *>(tsk_this());
}

static inline
int __gthread_yield()
{
	tsk_yield();
	return 0;
}

#endif//__STATEOS_THREAD_HH
