/******************************************************************************

    @file    StateOS: osspinlock.h
    @author  Rajmund Szymanski
    @date    26.07.2022
    @brief   This file contains definitions for StateOS.

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

#ifndef __STATEOS_SPN_H
#define __STATEOS_SPN_H

#include "oskernel.h"
#include "oscriticalsection.h"

/******************************************************************************
 *
 * Name              : spin lock
 *
 ******************************************************************************/

typedef uint_fast8_t spn_t;
typedef uint_fast8_t spn_id [];

/******************************************************************************
 *
 * Name              : _SPN_INIT
 *
 * Description       : create and initialize a spin lock object
 *
 * Parameters        : none
 *
 * Return            : spin lock object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _SPN_INIT()   0

/******************************************************************************
 *
 * Name              : OS_SPN
 * Static alias      : static_SPN
 *
 * Description       : define and initialize a spin lock object
 *
 * Parameters
 *   spn             : name of a pointer to spin lock object
 *
 ******************************************************************************/

#define             OS_SPN( spn ) \
                       spn_t spn[] = { _SPN_INIT() }

#define         static_SPN( spn ) \
                static spn_t spn[] = { _SPN_INIT() }

/******************************************************************************
 *
 * Name              : SPN_INIT
 *
 * Description       : create and initialize a spin lock object
 *
 * Parameters        : none
 *
 * Return            : spin lock object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                SPN_INIT() \
                      _SPN_INIT()
#endif

/******************************************************************************
 *
 * Name              : SPN_CREATE
 * Alias             : SPN_NEW
 *
 * Description       : create and initialize a spin lock object
 *
 * Parameters        : none
 *
 * Return            : spin lock object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                SPN_CREATE() \
                     { SPN_INIT  () }
#define                SPN_NEW \
                       SPN_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : core_spn_tryLock
 *
 * Description       : try lock the spin lock object
 *
 * Parameters
 *   spn             : pointer to spin lock object
 *
 * Return
 *   true            : spin lock object was successfully locked or OS_ATOMICS is zero
 *   false           : otherwise
 *
 * Note              : for internal use
 *
 ******************************************************************************/

__STATIC_INLINE
bool core_spn_tryLock( spn_t *spn )
{
#if OS_ATOMICS
	return __STD atomic_exchange((__STD atomic_uint_fast8_t *)spn, 1) == 0;
#else
	(void) spn;
	return true;
#endif
}

/******************************************************************************
 *
 * Name              : core_spn_lock
 *
 * Description       : lock the spin lock object
 *                     (wait indefinitely if the spin lock object can't be locked immediately)
 *                     or do nothing if OS_ATOMICS is zero
 *
 * Parameters
 *   spn             : pointer to spin lock object
 *
 * Return            : none
 *
 * Note              : for internal use
 *
 ******************************************************************************/

__STATIC_INLINE
void core_spn_lock( spn_t *spn )
{
#if OS_ATOMICS
	while (!core_spn_tryLock(spn));
#else
	(void) spn;
#endif
}

/******************************************************************************
 *
 * Name              : core_spn_unlock
 *
 * Description       : unlock the spin lock object
 *                     or do nothing if OS_ATOMICS is zero
 *
 * Parameters
 *   spn             : pointer to spin lock object
 *
 * Return            : none
 *
 * Note              : for internal use
 *
 ******************************************************************************/

__STATIC_INLINE
void core_spn_unlock( spn_t *spn )
{
#if OS_ATOMICS
	__STD atomic_store((__STD atomic_uint_fast8_t *)spn, 0);
#else
	(void) spn;
#endif
}

/******************************************************************************
 *
 * Name              : spn_init
 *
 * Description       : initialize a spin lock object
 *
 * Parameters
 *   spn             : pointer to spin lock object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
void spn_init( spn_t *spn )
{
	*spn = 0;
}

/******************************************************************************
 *
 * Name              : spn_lock
 *
 * Description       : save interrupts state, disable interrupts then lock the spin lock object
 *                     (wait indefinitely if the spin lock object can't be locked immediately)
 *                     or do nothing if OS_ATOMICS is zero
 *                   / enter into critical section
 *
 * Parameters
 *   spn             : pointer to spin lock object
 *
 * Return            : none
 *
 * Note              : do not use waiting functions inside spn_lock / spn_unlock
 *                     use only in thread mode
 *
 ******************************************************************************/

#define                spn_lock(spn) \
                       sys_lock(); core_spn_lock(spn)

/******************************************************************************
 *
 * Name              : spn_unlock
 *
 * Description       : unlock the spin lock object
 *                     or do nothing if OS_ATOMICS is zero
 *                     then restore saved interrupts state
 *                   / exit from critical section
 *
 * Parameters
 *   spn             : pointer to spin lock object
 *
 * Return            : none
 *
 * Note              : do not use waiting functions inside spn_lock / spn_unlock
 *                     use only in thread mode
 *
 ******************************************************************************/

#define                spn_unlock(spn) \
                       core_spn_unlock(spn); sys_unlock()

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace stateos {

/******************************************************************************
 *
 * Class             : SpinLock
 *
 * Description       : use a spin lock object
 *
 * Constructor parameters
 *   spn             : pointer to spin lock object
 *
 ******************************************************************************/

struct SpinLock : private CriticalSection
{
	explicit
	SpinLock( spn_t *_spn ): spn_{ _spn} { core_spn_lock(spn_); }
	explicit
	SpinLock( spn_t& _spn ): spn_{&_spn} { core_spn_lock(spn_); }

	~SpinLock() { core_spn_unlock(spn_); }

	SpinLock( SpinLock&& ) = delete;
	SpinLock( const SpinLock& ) = delete;
	SpinLock& operator=( SpinLock&& ) = delete;
	SpinLock& operator=( const SpinLock& ) = delete;

	private:
	spn_t *spn_;
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__STATEOS_SPN_H
