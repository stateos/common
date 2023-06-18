/******************************************************************************

    @file    StateOS: osfastmutex.h
    @author  Rajmund Szymanski
    @date    18.06.2023
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

#ifndef __STATEOS_MUT_H
#define __STATEOS_MUT_H

#include "oskernel.h"
#include "osclock.h"

/******************************************************************************
 *
 * Name              : fast mutex (error checking)
 *
 * Note              : use only to synchronize tasks with the same priority
 *
 ******************************************************************************/

typedef struct __mut mut_t;

struct __mut
{
	obj_t    obj;   // object header

	tsk_t  * owner; // mutex owner
};

typedef struct __mut mut_id [];

/******************************************************************************
 *
 * Name              : _MUT_INIT
 *
 * Description       : create and initialize a fast mutex object
 *
 * Parameters        : none
 *
 * Return            : fast mutex object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _MUT_INIT() { _OBJ_INIT(), NULL }

/******************************************************************************
 *
 * Name              : OS_MUT
 * Static alias      : static_MUT
 *
 * Description       : define and initialize a fast mutex object
 *
 * Parameters
 *   mut             : name of a pointer to fast mutex object
 *
 ******************************************************************************/

#define             OS_MUT( mut ) \
                       mut_t mut[] = { _MUT_INIT() }

#define         static_MUT( mut ) \
                static mut_t mut[] = { _MUT_INIT() }

/******************************************************************************
 *
 * Name              : MUT_INIT
 *
 * Description       : create and initialize a fast mutex object
 *
 * Parameters        : none
 *
 * Return            : fast mutex object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                MUT_INIT() \
                      _MUT_INIT()
#endif

/******************************************************************************
 *
 * Name              : MUT_CREATE
 * Alias             : MUT_NEW
 *
 * Description       : create and initialize a fast mutex object
 *
 * Parameters        : none
 *
 * Return            : fast mutex object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                MUT_CREATE() \
                     { MUT_INIT  () }
#define                MUT_NEW \
                       MUT_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : mut_init
 *
 * Description       : initialize a fast mutex object
 *
 * Parameters
 *   mut             : pointer to fast mutex object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void mut_init( mut_t *mut );

/******************************************************************************
 *
 * Name              : mut_create
 * Alias             : mut_new
 *
 * Description       : create and initialize a new fast mutex object
 *
 * Parameters        : none
 *
 * Return            : pointer to fast mutex object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

mut_t *mut_create( void );

__STATIC_INLINE
mut_t *mut_new( void ) { return mut_create(); }

/******************************************************************************
 *
 * Name              : mut_reset
 * Alias             : mut_kill
 *
 * Description       : reset the fast mutex object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   mut             : pointer to fast mutex object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void mut_reset( mut_t *mut );

__STATIC_INLINE
void mut_kill( mut_t *mut ) { mut_reset(mut); }

/******************************************************************************
 *
 * Name              : mut_destroy
 * Alias             : mut_delete
 *
 * Description       : reset the fast mutex object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * Parameters
 *   mut             : pointer to fast mutex object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void mut_destroy( mut_t *mut );

__STATIC_INLINE
void mut_delete( mut_t *mut ) { mut_destroy(mut); }

/******************************************************************************
 *
 * Name              : mut_take
 * Alias             : mut_tryLock
 *
 * Description       : try to lock the fast mutex object,
 *                     don't wait if the fast mutex object can't be locked immediately
 *
 * Parameters
 *   mut             : pointer to fast mutex object
 *
 * Return
 *   E_SUCCESS       : fast mutex object was successfully locked
 *   E_FAILURE       : fast mutex object can't be locked by owner task
 *   E_TIMEOUT       : fast mutex object can't be locked immediately, try again
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int mut_take( mut_t *mut );

__STATIC_INLINE
int mut_tryLock( mut_t *mut ) { return mut_take(mut); }

/******************************************************************************
 *
 * Name              : mut_waitFor
 *
 * Description       : try to lock the fast mutex object,
 *                     wait for given duration of time if the fast mutex object can't be locked immediately
 *
 * Parameters
 *   mut             : pointer to fast mutex object
 *   delay           : duration of time (maximum number of ticks to wait for lock the fast mutex object)
 *                     IMMEDIATE: don't wait if the fast mutex object can't be locked immediately
 *                     INFINITE:  wait indefinitely until the fast mutex object has been locked
 *
 * Return
 *   E_SUCCESS       : fast mutex object was successfully locked
 *   E_FAILURE       : fast mutex object can't be locked by owner task
 *   E_STOPPED       : fast mutex object was reseted before the specified timeout expired
 *   E_DELETED       : fast mutex object was deleted before the specified timeout expired
 *   E_TIMEOUT       : fast mutex object was not locked before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int mut_waitFor( mut_t *mut, cnt_t delay );

/******************************************************************************
 *
 * Name              : mut_waitUntil
 *
 * Description       : try to lock the fast mutex object,
 *                     wait until given timepoint if the fast mutex object can't be locked immediately
 *
 * Parameters
 *   mut             : pointer to fast mutex object
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : fast mutex object was successfully locked
 *   E_FAILURE       : fast mutex object can't be locked by owner task
 *   E_STOPPED       : fast mutex object was reseted before the specified timeout expired
 *   E_DELETED       : fast mutex object was deleted before the specified timeout expired
 *   E_TIMEOUT       : fast mutex object was not locked before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int mut_waitUntil( mut_t *mut, cnt_t time );

/******************************************************************************
 *
 * Name              : mut_wait
 * Alias             : mut_lock
 *
 * Description       : try to lock the fast mutex object,
 *                     wait indefinitely if the fast mutex object can't be locked immediately
 *
 * Parameters
 *   mut             : pointer to fast mutex object
 *
 * Return
 *   E_SUCCESS       : fast mutex object was successfully locked
 *   E_FAILURE       : fast mutex object can't be locked by owner task
 *   E_STOPPED       : fast mutex object was reseted
 *   E_DELETED       : fast mutex object was deleted
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
int mut_wait( mut_t *mut ) { return mut_waitFor(mut, INFINITE); }

__STATIC_INLINE
int mut_lock( mut_t *mut ) { return mut_wait(mut); }

/******************************************************************************
 *
 * Name              : mut_give
 * Alias             : mut_unlock
 *
 * Description       : try to unlock the fast mutex object (only owner task can unlock fast mutex object),
 *                     don't wait if the fast mutex object can't be unlocked
 *
 * Parameters
 *   mut             : pointer to fast mutex object
 *
 * Return
 *   E_SUCCESS       : fast mutex object was successfully unlocked
 *   E_FAILURE       : fast mutex object can't be unlocked
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int mut_give( mut_t *mut );

__STATIC_INLINE
int mut_unlock( mut_t *mut ) { return mut_give(mut); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace stateos {

/******************************************************************************
 *
 * Class             : FastMutex
 *
 * Description       : create and initialize a fast mutex object
 *
 * Constructor parameters
 *                   : none
 *
 ******************************************************************************/

struct FastMutex : public __mut
{
	constexpr
	FastMutex(): __mut _MUT_INIT() {}

	~FastMutex() { assert(__mut::owner == nullptr); }

	FastMutex( FastMutex&& ) = default;
	FastMutex( const FastMutex& ) = delete;
	FastMutex& operator=( FastMutex&& ) = delete;
	FastMutex& operator=( const FastMutex& ) = delete;

	void reset    ()                  {        mut_reset    (this); }
	void kill     ()                  {        mut_kill     (this); }
	void destroy  ()                  {        mut_destroy  (this); }
	int  take     ()                  { return mut_take     (this); }
	int  tryLock  ()                  { return mut_tryLock  (this); }
	template<typename T>
	int  waitFor  ( const T& _delay ) { return mut_waitFor  (this, Clock::count(_delay)); }
	template<typename T>
	int  waitUntil( const T& _time )  { return mut_waitUntil(this, Clock::until(_time)); }
	int  wait     ()                  { return mut_wait     (this); }
	int  lock     ()                  { return mut_lock     (this); }
	int  give     ()                  { return mut_give     (this); }
	int  unlock   ()                  { return mut_unlock   (this); }

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<FastMutex>;
#else
	using Ptr = FastMutex *;
#endif

/******************************************************************************
 *
 * Name              : FastMutex::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters        : none
 *
 * Return            : std::unique_pointer / pointer to FastMutex object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create()
	{
		auto mut = new (std::nothrow) FastMutex();
		if (mut != nullptr)
			mut->__mut::obj.res = mut;
		return Ptr(mut);
	}

};

/******************************************************************************
 *
 * Class             : FastLock
 *
 * Description       : create and initialize a lock guard object
 *
 ******************************************************************************/

struct FastLock
{
	explicit
	FastLock( FastMutex& _mut ): mut_(_mut)
	{
		int result = mut_.lock();
		assert(result == E_SUCCESS);
		(void) result;
	}

	~FastLock()
	{
		int result = mut_.unlock();
		assert(result == E_SUCCESS);
		(void) result;
	}

	FastLock( FastLock&& ) = default;
	FastLock( const FastLock& ) = delete;
	FastLock& operator=( FastLock&& ) = delete;
	FastLock& operator=( const FastLock& ) = delete;

	private:
	FastMutex& mut_;
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__STATEOS_MUT_H
