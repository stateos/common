/******************************************************************************

    @file    StateOS: oscriticalsection.h
    @author  Rajmund Szymanski
    @date    03.04.2021
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

#ifndef __STATEOS_CRI_H
#define __STATEOS_CRI_H

#include "oskernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : core_set_lock
 *
 * Description       : disable interrupts
 *
 * Parameters        : none
 *
 * Return            : previous interrupts state
 *
 * Note              : for internal use
 *
 ******************************************************************************/

__STATIC_INLINE
lck_t core_set_lock( void )
{
	lck_t prv = port_get_lock();
	port_set_lock();
	__COMPILER_BARRIER();
	return prv;
}

/******************************************************************************
 *
 * Name              : core_clr_lock
 *
 * Description       : enable interrupts
 *
 * Parameters        : none
 *
 * Return            : previous interrupts state
 *
 * Note              : for internal use
 *
 ******************************************************************************/

__STATIC_INLINE
lck_t core_clr_lock( void )
{
	lck_t prv = port_get_lock();
	port_clr_lock();
	__COMPILER_BARRIER();
	return prv;
}

/******************************************************************************
 *
 * Name              : core_put_lock
 *
 * Description       : restore interrupts state
 *
 * Parameters
 *   lck             : interrupts state
 *
 * Return            : none
 *
 * Note              : for internal use
 *
 ******************************************************************************/

__STATIC_INLINE
void core_put_lock( lck_t lck )
{
	__COMPILER_BARRIER();
	port_put_lock(lck);
}

/******************************************************************************
 *
 * Name              : core_get_lock
 *
 * Description       : return current interrupts state
 *
 * Parameters        : none
 *
 * Return            : current interrupts state
 *
 * Note              : for internal use
 *
 ******************************************************************************/

__STATIC_INLINE
lck_t core_get_lock( void )
{
	return port_get_lock();
}

/******************************************************************************
 *
 * Name              : sys_lock
 * ISR alias         : sys_lockISR
 *
 * Description       : save interrupts state then disable interrupts
 *                   / enter into critical section
 *
 * Parameters        : none
 *
 * Return            : none
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

#define                sys_lock() \
                       do { lck_t __LOCK = core_set_lock()

#define                sys_lockISR() \
                       sys_lock()

/******************************************************************************
 *
 * Name              : sys_unlock
 * ISR alias         : sys_unlockISR
 *
 * Description       : restore saved interrupts state
 *                   / exit from critical section
 *
 * Parameters        : none
 *
 * Return            : none
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

#define                sys_unlock() \
                       core_put_lock(__LOCK); } while (0)

#define                sys_unlockISR() \
                       sys_unlock()

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace stateos {

/******************************************************************************
 *
 * Class             : CriticalSection
 *
 * Description       : create and initialize a critical section guard object
 *
 * Constructor parameters
 *                   : none
 *
 ******************************************************************************/

struct CriticalSection
{
	CriticalSection() { lck_ = core_set_lock(); }

	~CriticalSection() { core_put_lock(lck_); }

	CriticalSection( CriticalSection&& ) = delete;
	CriticalSection( const CriticalSection& ) = delete;
	CriticalSection& operator=( CriticalSection&& ) = delete;
	CriticalSection& operator=( const CriticalSection& ) = delete;

	private:
	lck_t lck_;
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__STATEOS_CRI_H
