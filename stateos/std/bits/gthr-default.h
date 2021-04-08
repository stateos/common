/******************************************************************************

    @file    StateOS: gthr-default.h
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

#ifndef __STATEOS_GLIBCXX_GCC_GTHR_H
#define __STATEOS_GLIBCXX_GCC_GTHR_H
#ifdef  __GNUC__

#include "os.h"

//-----------------------------------------------------------------------------

#if     OS_TASK_EXIT == 0
#error  osconfig.h: Invalid OS_TASK_EXIT value! It must be a value other than 0.
#endif

//-----------------------------------------------------------------------------

#define __GTHREADS 1
#define __GTHREADS_CXX0X 1
#define __GTHREAD_HAS_COND
#define  _GLIBCXX_USE_SCHED_YIELD

//-----------------------------------------------------------------------------

typedef int   __gthread_key_t;
typedef one_t __gthread_once_t;
typedef mtx_t __gthread_mutex_t;
typedef mtx_t __gthread_recursive_mutex_t;
typedef cnd_t __gthread_cond_t;

struct  ostime_t;
typedef ostime_t  __gthread_time_t;

struct  ostask_t;
typedef ostask_t *__gthread_t;

//-----------------------------------------------------------------------------

#define __GTHREAD_ONCE_INIT            _ONE_INIT()
#define __GTHREAD_MUTEX_INIT           _MTX_INIT(mtxPrioInherit|mtxErrorCheck, 0)
#define __GTHREAD_RECURSIVE_MUTEX_INIT _MTX_INIT(mtxPrioInherit|mtxRecursive, 0)
#define __GTHREAD_COND_INIT            _CND_INIT()

//-----------------------------------------------------------------------------

static inline
int __gthread_active_p()
{
	return 1;
}

//-----------------------------------------------------------------------------

#include "inc/chrono.hh"
#include "inc/critical_section.hh"
#include "inc/key.hh"
#include "inc/condition_variable.hh"
#include "inc/mutex.hh"
#include "inc/semaphore.hh"
#include "inc/thread.hh"

//-----------------------------------------------------------------------------

#endif //__GNUC__
#endif //__STATEOS_GLIBCXX_GCC_GTHR_H
