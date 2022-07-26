/******************************************************************************

    @file    StateOS: oslibc.c
    @author  Rajmund Szymanski
    @date    20.04.2021
    @brief   This file provides set of variables and functions for StateOS.

 ******************************************************************************

   Copyright (c) 2018-2022 Rajmund Szymanski. All rights reserved.

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

#include <sys/lock.h>
#include "oskernel.h"
#include "inc/osmutex.h"

/* -------------------------------------------------------------------------- */
#ifdef _RETARGETABLE_LOCKING

struct __lock { mtx_t mtx; };

struct __lock __lock___arc4random_mutex       = { _MTX_INIT(mtxPrioInherit|mtxErrorCheck, 0) };
struct __lock __lock___at_quick_exit_mutex    = { _MTX_INIT(mtxPrioInherit|mtxErrorCheck, 0) };
struct __lock __lock___atexit_recursive_mutex = { _MTX_INIT(mtxPrioInherit|mtxRecursive,  0) };
struct __lock __lock___dd_hash_mutex          = { _MTX_INIT(mtxPrioInherit|mtxErrorCheck, 0) };
struct __lock __lock___env_recursive_mutex    = { _MTX_INIT(mtxPrioInherit|mtxRecursive,  0) };
struct __lock __lock___malloc_recursive_mutex = { _MTX_INIT(mtxPrioInherit|mtxRecursive,  0) };
struct __lock __lock___sfp_recursive_mutex    = { _MTX_INIT(mtxPrioInherit|mtxRecursive,  0) };
struct __lock __lock___sinit_recursive_mutex  = { _MTX_INIT(mtxPrioInherit|mtxRecursive,  0) };
struct __lock __lock___tz_mutex               = { _MTX_INIT(mtxPrioInherit|mtxErrorCheck, 0) };

/* -------------------------------------------------------------------------- */

void __retarget_lock_init(_LOCK_T * const lock)
{
	*lock = (_LOCK_T)mtx_new(mtxPrioInherit|mtxErrorCheck, 0);
}

void __retarget_lock_init_recursive(_LOCK_T * const lock)
{
	*lock = (_LOCK_T)mtx_new(mtxPrioInherit|mtxRecursive, 0);
}

void __retarget_lock_close(const _LOCK_T lock)
{
	mtx_delete(&lock->mtx);
}

void __retarget_lock_close_recursive(const _LOCK_T lock)
{
	mtx_delete(&lock->mtx);
}

void __retarget_lock_acquire(const _LOCK_T lock)
{
	mtx_lock(&lock->mtx);
}

void __retarget_lock_acquire_recursive(const _LOCK_T lock)
{
	mtx_lock(&lock->mtx);
}

int __retarget_lock_try_acquire(const _LOCK_T lock)
{
	return mtx_tryLock(&lock->mtx);
}

int __retarget_lock_try_acquire_recursive(const _LOCK_T lock)
{
	return mtx_tryLock(&lock->mtx);
}

void __retarget_lock_release(const _LOCK_T lock)
{
	mtx_unlock(&lock->mtx);
}

void __retarget_lock_release_recursive(const _LOCK_T lock)
{
	mtx_unlock(&lock->mtx);
}

#endif
/* -------------------------------------------------------------------------- */

static lck_t    LCK = 0;
static unsigned CNT = 0;

void __malloc_lock(struct _reent *reent)
{
	(void) reent;
	assert(CNT<UINT_MAX);
	lck_t lock = port_get_lock();
	port_set_lock();
	if (CNT++ == 0U)
		LCK = lock;
}

void __malloc_unlock(struct _reent *reent)
{
	(void) reent;
	assert(CNT>0U);
	if (--CNT == 0U)
		port_put_lock(LCK);
}

/* -------------------------------------------------------------------------- */

__NO_RETURN void __cxa_pure_virtual(void)
{
	abort();
}

/* -------------------------------------------------------------------------- */
