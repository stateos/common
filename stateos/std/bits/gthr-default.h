/******************************************************************************

    @file    StateOS: gthr-default.h
    @author  Rajmund Szymanski
    @date    12.04.2021
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

#ifndef _GLIBCXX_GCC_GTHR_STATEOS_H
#define _GLIBCXX_GCC_GTHR_STATEOS_H

#include "os.h"
#include "inc/chrono.hh"
#include "inc/critical_section.hh"
#if    !__has_include(<semaphore>)
#include "inc/semaphore.hh"
#endif

//-----------------------------------------------------------------------------

#if   __cplusplus < 201709L
#error  Required C++20 or above.
#endif

#if     OS_TASK_EXIT == 0
#error  osconfig.h: Invalid OS_TASK_EXIT value! It must be a value other than 0.
#endif

//-----------------------------------------------------------------------------

#define __GTHREADS 1
#define __GTHREADS_CXX0X 1
#define __GTHREAD_HAS_COND
#define  _GLIBCXX_USE_SCHED_YIELD

//-----------------------------------------------------------------------------

typedef one_t  __gthread_once_t;
typedef mtx_t  __gthread_mutex_t;
typedef mtx_t  __gthread_recursive_mutex_t;
typedef cnd_t  __gthread_cond_t;
typedef tsk_t *__gthread_t;

struct  oskey_t;
typedef oskey_t *__gthread_key_t;

struct  ostime_t;
typedef ostime_t __gthread_time_t;

#define __GTHREAD_ONCE_INIT            _ONE_INIT()
#define __GTHREAD_MUTEX_INIT           _MTX_INIT(mtxPrioInherit|mtxErrorCheck, 0)
#define __GTHREAD_RECURSIVE_MUTEX_INIT _MTX_INIT(mtxPrioInherit|mtxRecursive, 0)
#define __GTHREAD_COND_INIT            _CND_INIT()

//-----------------------------------------------------------------------------

#ifndef _LIBOBJC

static inline
int __gthread_active_p()
{
	return 1;
}

static inline
int __gthread_mutex_destroy(__gthread_mutex_t *mutex)
{
	mtx_destroy(mutex);
	return 0;
}

static inline
int __gthread_recursive_mutex_destroy(__gthread_recursive_mutex_t *mutex)
{
	mtx_destroy(mutex);
	return 0;
}

static inline
int __gthread_mutex_lock(__gthread_mutex_t *mutex)
{
	// EINVAL, EAGAIN, EBUSY, EINVAL, EDEADLK(may)
	return mtx_lock(mutex);
}

static inline
int __gthread_mutex_trylock(__gthread_mutex_t *mutex)
{
	// XXX EINVAL, EAGAIN, EBUSY
	return mtx_tryLock(mutex);
}

static inline
int __gthread_mutex_timedlock(__gthread_mutex_t *mutex, const __gthread_time_t *abs_timeout)
{
	return mtx_waitUntil(mutex, abs_timeout->to_ticks());
}

static inline
int __gthread_mutex_unlock(__gthread_mutex_t *mutex)
{
	// XXX EINVAL, EAGAIN, EBUSY
	return mtx_unlock(mutex);
}

static inline
int __gthread_recursive_mutex_lock(__gthread_recursive_mutex_t *mutex)
{
	// EINVAL, EAGAIN, EBUSY, EINVAL, EDEADLK(may)
	return mtx_lock(mutex);
}

static inline
int __gthread_recursive_mutex_trylock(__gthread_recursive_mutex_t *mutex)
{
	// XXX EINVAL, EAGAIN, EBUSY
	return mtx_tryLock(mutex);
}

static inline
int __gthread_recursive_mutex_timedlock(__gthread_recursive_mutex_t *mutex, const __gthread_time_t *abs_timeout)
{
	return mtx_waitUntil(mutex, abs_timeout->to_ticks());
}

static inline
int __gthread_recursive_mutex_unlock(__gthread_recursive_mutex_t *mutex)
{
	// XXX EINVAL, EAGAIN, EBUSY
	return mtx_unlock(mutex);
}

static inline
int __gthread_once(__gthread_once_t *once, void(*func)())
{
	one_call(once, func);
	return 0;
}

static inline
int __gthread_cond_signal(__gthread_cond_t *cond)
{
	cnd_notifyOne(cond);
	return 0;
}

static inline
int __gthread_cond_broadcast(__gthread_cond_t *cond)
{
	cnd_notifyAll(cond);
	return 0;
}

static inline
int __gthread_cond_wait(__gthread_cond_t *cond, __gthread_mutex_t *mutex)
{
	return cnd_wait(cond, mutex);
}

static inline
int __gthread_cond_wait_recursive(__gthread_cond_t *cond, __gthread_recursive_mutex_t *mutex)
{
	return cnd_wait(cond, mutex);
}

static inline
int __gthread_cond_timedwait(__gthread_cond_t *cond, __gthread_mutex_t *mutex, const __gthread_time_t *abs_timeout)
{
	return cnd_waitUntil(cond, mutex, abs_timeout->to_ticks());
}

static inline
int __gthread_create(__gthread_t *thread, void (*func)(void *), void *args)
{
	*thread = thd_create(OS_MAIN_PRIO, func, args, OS_STACK_SIZE);
	return *thread == nullptr;
}

static inline
int __gthread_join(__gthread_t thread, void **)
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
	return t1 != t2;
}

static inline
__gthread_t __gthread_self()
{
	return tsk_this();
}

static inline
int __gthread_yield()
{
	tsk_yield();
	return 0;
}

int   __gthread_key_create(__gthread_key_t *keyp, void(*dtor)(void *));

int   __gthread_key_delete(__gthread_key_t key);

void *__gthread_getspecific(__gthread_key_t key);

int   __gthread_setspecific(__gthread_key_t key, const void *ptr);

int   __gthread_atexit(__gthread_t thread);

#endif//!_LIBOBJC

//-----------------------------------------------------------------------------

#endif //_GLIBCXX_GCC_GTHR_STATEOS_H
