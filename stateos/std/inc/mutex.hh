/******************************************************************************

    @file    StateOS: mutex.hh
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

#ifndef __STATEOS_MUTEX_HH
#define __STATEOS_MUTEX_HH

#include "inc/osmutex.h"
#include "inc/osonceflag.h"

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
int __gthread_mutex_timedlock(__gthread_mutex_t *m, const __gthread_time_t *abs_timeout)
{
	return mtx_waitUntil(m, static_cast<cnt_t>(*abs_timeout));
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
int __gthread_recursive_mutex_timedlock(__gthread_recursive_mutex_t *m, const __gthread_time_t *abs_timeout)
{
	return mtx_waitUntil(m, static_cast<cnt_t>(*abs_timeout));
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

#endif//__STATEOS_MUTEX_HH
