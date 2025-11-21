/******************************************************************************

    @file    IntrOS: osbarrier.h
    @author  Rajmund Szymanski
    @date    17.11.2025
    @brief   This file contains definitions for IntrOS.

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

#ifndef __INTROS_BAR_H
#define __INTROS_BAR_H

#include "oskernel.h"
#include "osclock.h"

/******************************************************************************
 *
 * Name              : barrier
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
 ******************************************************************************/

void bar_init( bar_t *bar, unsigned limit );

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
 *   E_TIMEOUT       : barrier object was not released before the specified timeout expired
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
 *   E_TIMEOUT       : barrier object was not released before the specified timeout expired
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
 *
 ******************************************************************************/

__STATIC_INLINE
int bar_wait( bar_t *bar ) { return bar_waitFor(bar, INFINITE); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus)
namespace intros {

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
	template<typename T>
	int  waitFor  ( const T& _delay ) { return bar_waitFor  (this, Clock::count(_delay)); }
	template<typename T>
	int  waitUntil( const T& _time )  { return bar_waitUntil(this, Clock::until(_time)); }
	int  wait     ()                  { return bar_wait     (this); }
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_BAR_H
