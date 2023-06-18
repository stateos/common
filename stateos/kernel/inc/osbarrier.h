/******************************************************************************

    @file    StateOS: osbarrier.h
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

#ifndef __STATEOS_BAR_H
#define __STATEOS_BAR_H

#include "oskernel.h"
#include "osclock.h"

/******************************************************************************
 *
 * Name              : barrier
 *                     like a POSIX pthread_barrier_t
 *
 ******************************************************************************/

typedef struct __bar bar_t;

struct __bar
{
	obj_t    obj;   // object header

	unsigned limit; // limit of tasks blocked on the barrier object
};

typedef struct __bar bar_id [];

/******************************************************************************
 *
 * Name              : _BAR_INIT
 *
 * Description       : create and initialize a barrier object
 *
 * Parameters
 *   limit           : number of tasks that must call bar_wait[Until|For] function to release the barrier object
 *
 * Return            : barrier object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _BAR_INIT( _limit ) { _OBJ_INIT(), _limit }

/******************************************************************************
 *
 * Name              : OS_BAR
 * Static alias      : static_BAR
 *
 * Description       : define and initialize a barrier object
 *
 * Parameters
 *   bar             : name of a pointer to barrier object
 *   limit           : number of tasks that must call bar_wait[Until|For] function to release the barrier object
 *
 ******************************************************************************/

#define             OS_BAR( bar, limit ) \
                       bar_t bar[] = { _BAR_INIT( limit ) }

#define         static_BAR( bar, limit ) \
                static bar_t bar[] = { _BAR_INIT( limit ) }

/******************************************************************************
 *
 * Name              : BAR_INIT
 *
 * Description       : create and initialize a barrier object
 *
 * Parameters
 *   limit           : number of tasks that must call bar_wait[Until|For] function to release the barrier object
 *
 * Return            : barrier object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                BAR_INIT( limit ) \
                      _BAR_INIT( limit )
#endif

/******************************************************************************
 *
 * Name              : BAR_CREATE
 * Alias             : BAR_NEW
 *
 * Description       : create and initialize a barrier object
 *
 * Parameters
 *   limit           : number of tasks that must call bar_wait[Until|For] function to release the barrier object
 *
 * Return            : barrier object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                BAR_CREATE( limit ) \
                     { BAR_INIT  ( limit ) }
#define                BAR_NEW \
                       BAR_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : bar_init
 *
 * Description       : initialize a barrier object
 *
 * Parameters
 *   bar             : pointer to barrier object
 *   limit           : number of tasks that must call bar_wait[Until|For] function to release the barrier object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void bar_init( bar_t *bar, unsigned limit );

/******************************************************************************
 *
 * Name              : bar_create
 * Alias             : bar_new
 *
 * Description       : create and initialize a new barrier object
 *
 * Parameters
 *   limit           : number of tasks that must call bar_wait[Until|For] function to release the barrier object
 *
 * Return            : pointer to barrier object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

bar_t *bar_create( unsigned limit );

__STATIC_INLINE
bar_t *bar_new( unsigned limit ) { return bar_create(limit); }

/******************************************************************************
 *
 * Name              : bar_reset
 * Alias             : bar_kill
 *
 * Description       : reset the barrier object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   bar             : pointer to barrier object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void bar_reset( bar_t *bar );

__STATIC_INLINE
void bar_kill( bar_t *bar ) { bar_reset(bar); }

/******************************************************************************
 *
 * Name              : bar_destroy
 * Alias             : bar_delete
 *
 * Description       : reset the barrier object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * Parameters
 *   bar             : pointer to barrier object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void bar_destroy( bar_t *bar );

__STATIC_INLINE
void bar_delete( bar_t *bar ) { bar_destroy(bar); }

/******************************************************************************
 *
 * Name              : bar_waitFor
 *
 * Description       : wait for release the barrier object for given duration of time
 *
 * Parameters
 *   bar             : pointer to barrier object
 *   delay           : duration of time (maximum number of ticks to wait for release the barrier object)
 *                     IMMEDIATE: don't wait if the barrier object can't be released
 *                     INFINITE:  wait indefinitely until the barrier object has been released
 *
 * Return
 *   E_SUCCESS       : barrier object was successfully released
 *   E_STOPPED       : barrier object was reseted before the specified timeout expired
 *   E_DELETED       : barrier object was deleted before the specified timeout expired
 *   E_TIMEOUT       : barrier object was not released before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int bar_waitFor( bar_t *bar, cnt_t delay );

/******************************************************************************
 *
 * Name              : bar_waitUntil
 *
 * Description       : wait for release the barrier object until given timepoint
 *
 * Parameters
 *   bar             : pointer to barrier object
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : barrier object was successfully released
 *   E_STOPPED       : barrier object was reseted before the specified timeout expired
 *   E_DELETED       : barrier object was deleted before the specified timeout expired
 *   E_TIMEOUT       : barrier object was not released before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int bar_waitUntil( bar_t *bar, cnt_t time );

/******************************************************************************
 *
 * Name              : bar_wait
 *
 * Description       : wait indefinitely until the barrier object has been released
 *
 * Parameters
 *   bar             : pointer to barrier object
 *
 * Return
 *   E_SUCCESS       : barrier object was successfully released
 *   E_STOPPED       : barrier object was reseted
 *   E_DELETED       : barrier object was deleted
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
int bar_wait( bar_t *bar ) { return bar_waitFor(bar, INFINITE); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace stateos {

/******************************************************************************
 *
 * Class             : Barrier
 *
 * Description       : create and initialize a barrier object
 *
 * Constructor parameters
 *   limit           : number of tasks that must call wait[Until|For] function to release the barrier object
 *
 ******************************************************************************/

struct Barrier : public __bar
{
	constexpr
	Barrier( const unsigned _limit ): __bar _BAR_INIT(_limit) {}

	~Barrier() { assert(__bar::obj.queue == nullptr); }

	Barrier( Barrier&& ) = default;
	Barrier( const Barrier& ) = delete;
	Barrier& operator=( Barrier&& ) = delete;
	Barrier& operator=( const Barrier& ) = delete;

	void reset    ()                  {        bar_reset    (this); }
	void kill     ()                  {        bar_kill     (this); }
	void destroy  ()                  {        bar_destroy  (this); }
	template<typename T>
	int  waitFor  ( const T& _delay ) { return bar_waitFor  (this, Clock::count(_delay)); }
	template<typename T>
	int  waitUntil( const T& _time )  { return bar_waitUntil(this, Clock::until(_time)); }
	int  wait     ()                  { return bar_wait     (this); }

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<Barrier>;
#else
	using Ptr = Barrier *;
#endif

/******************************************************************************
 *
 * Name              : Barrier::Create
 *
 * Description       : create and initialize dynamic object with manageable resources
 *
 * Parameters
 *   limit           : number of tasks that must call bar_wait[Until|For] function to release the barrier object
 *
 * Return            : std::unique_pointer / pointer to Barrier object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create( const unsigned _limit )
	{
		auto bar = new (std::nothrow) Barrier(_limit);
		if (bar != nullptr)
			bar->__bar::obj.res = bar;
		return Ptr(bar);
	}

};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__STATEOS_BAR_H
