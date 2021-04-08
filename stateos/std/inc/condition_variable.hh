/******************************************************************************

    @file    StateOS: condition_variable.hh
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

#ifndef __STATEOS_CONDITION_VARIABLE_HH
#define __STATEOS_CONDITION_VARIABLE_HH

#include "inc/osconditionvariable.h"

static inline
int __gthread_cond_signal(__gthread_cond_t *cond)
{
	cnd_notifyOne(cond);
}

static inline
int __gthread_cond_broadcast(__gthread_cond_t *cond)
{
	cnd_notifyAll(cond);
}

static inline
int __gthread_cond_wait(__gthread_cond_t *cond, __gthread_mutex_t *mutex)
{
	cnd_wait(cond, mutex);
}

static inline
int __gthread_cond_wait_recursive(__gthread_cond_t *cond, __gthread_recursive_mutex_t *mutex)
{
	cnd_wait(cond, mutex);
}

static inline
int __gthread_cond_timedwait(__gthread_cond_t *cond, __gthread_mutex_t *mutex, const __gthread_time_t *abs_timeout)
{
	cnd_waitUntil(cond, mutex, static_cast<cnt_t>(*abs_timeout));
}

#endif//__STATEOS_CONDITION_VARIABLE_HH
