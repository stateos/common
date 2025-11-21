/******************************************************************************

    @file    IntrOS: osconditionvariable.h
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

#ifndef __INTROS_CND_H
#define __INTROS_CND_H

#include "oskernel.h"
#include "osclock.h"
#include "osmutex.h"

/* -------------------------------------------------------------------------- */

#define cndOne       ( false ) // notify one task
#define cndAll       ( true  ) // notify all tasks (broadcast)

/******************************************************************************
 *
 * Name              : condition variable
 *
 ******************************************************************************/

typedef struct __cnd cnd_t;

struct __cnd
{
	obj_t    obj;   // object header
};

typedef struct __cnd cnd_id [];

/******************************************************************************
 *
 * Name              : _CND_INIT
 *
 * Description       : create and initialize a condition variable object
 *
 * Parameters        : none
 *
 * Return            : condition variable object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _CND_INIT() { _OBJ_INIT() }

/******************************************************************************
 *
 * Name              : OS_CND
 * Static alias      : static_CND
 *
 * Description       : define and initialize a condition variable object
 *
 * Parameters
 *   cnd             : name of a pointer to condition variable object
 *
 ******************************************************************************/

#define             OS_CND( cnd ) \
                       cnd_t cnd[] = { _CND_INIT() }

#define         static_CND( cnd ) \
                static cnd_t cnd[] = { _CND_INIT() }

/******************************************************************************
 *
 * Name              : CND_INIT
 *
 * Description       : create and initialize a condition variable object
 *
 * Parameters        : none
 *
 * Return            : condition variable object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                CND_INIT() \
                      _CND_INIT()
#endif

/******************************************************************************
 *
 * Name              : CND_CREATE
 * Alias             : CND_NEW
 *
 * Description       : create and initialize a condition variable object
 *
 * Parameters        : none
 *
 * Return            : condition variable object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                CND_CREATE() \
                     { CND_INIT  () }
#define                CND_NEW \
                       CND_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : cnd_init
 *
 * Description       : initialize a condition variable object
 *
 * Parameters
 *   cnd             : pointer to condition variable object
 *
 * Return            : none
 *
 ******************************************************************************/

void cnd_init( cnd_t *cnd );

/******************************************************************************
 *
 * Name              : cnd_reset
 * Alias             : cnd_kill
 *
 * Description       : reset the condition variable object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   cnd             : pointer to condition variable object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void cnd_reset( cnd_t *cnd );

__STATIC_INLINE
void cnd_kill( cnd_t *cnd ) { cnd_reset(cnd); }

/******************************************************************************
 *
 * Name              : cnd_waitFor
 *
 * Description       : wait for given duration of time on the condition variable releasing the currently owned mutex,
 *                     and finally lock the mutex again
 *
 * Parameters
 *   cnd             : pointer to condition variable object
 *   mtx             : currently owned mutex
 *   delay           : duration of time (maximum number of ticks to wait on the condition variable object)
 *                     IMMEDIATE: don't wait on the condition variable object
 *                     INFINITE:  wait indefinitely on the condition variable object
 *
 * Return
 *   E_SUCCESS       : condition variable object was successfully signalled; owned mutex was locked again
 *   E_FAILURE       : mutex object can't be unlocked
 *   E_STOPPED       : condition variable or mutex object was reseted
 *   E_TIMEOUT       : condition variable object was not signalled before the specified timeout expired; owned mutex was locked again
 *
 ******************************************************************************/

int cnd_waitFor( cnd_t *cnd, mtx_t *mtx, cnt_t delay );

/******************************************************************************
 *
 * Name              : cnd_waitUntil
 *
 * Description       : wait until given timepoint on the condition variable releasing the currently owned mutex,
 *                     and finally lock the mutex again
 *
 * Parameters
 *   cnd             : pointer to condition variable object
 *   mtx             : currently owned mutex
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : condition variable object was successfully signalled; owned mutex was locked again
 *   E_FAILURE       : mutex object can't be unlocked
 *   E_STOPPED       : condition variable or mutex object was reseted
 *   E_TIMEOUT       : condition variable object was not signalled before the specified timeout expired; owned mutex was locked again
 *
 ******************************************************************************/

int cnd_waitUntil( cnd_t *cnd, mtx_t *mtx, cnt_t time );

/******************************************************************************
 *
 * Name              : cnd_wait
 *
 * Description       : wait indefinitely on the condition variable releasing the currently owned mutex,
 *                     and finally lock the mutex again
 *
 * Parameters
 *   cnd             : pointer to condition variable object
 *   mtx             : currently owned mutex
 *
 * Return
 *   E_SUCCESS       : condition variable object was successfully signalled; owned mutex was locked again
 *   E_FAILURE       : mutex object can't be unlocked
 *   E_STOPPED       : condition variable or mutex object was reseted
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
int cnd_wait( cnd_t *cnd, mtx_t *mtx ) { return cnd_waitFor(cnd, mtx, INFINITE); }

/******************************************************************************
 *
 * Name              : cnd_give
 *
 * Description       : signal one or all tasks that are waiting on the condition variable
 *
 * Parameters
 *   cnd             : pointer to condition variable object
 *   all             : signal receiver
 *                     cndOne: notify one task that is waiting on the condition variable
 *                     cndAll: notify all tasks that are waiting on the condition variable
 *
 * Return            : none
 *
 ******************************************************************************/

void cnd_give( cnd_t *cnd, bool all );

/******************************************************************************
 *
 * Name              : cnd_notifyOne
 *
 * Description       : signal one task waiting on the condition variable
 *
 * Parameters
 *   cnd             : pointer to condition variable object
 *
 * Return            : none
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *
 ******************************************************************************/

__STATIC_INLINE
void cnd_notifyOne( cnd_t *cnd ) { cnd_give(cnd, cndOne); }

/******************************************************************************
 *
 * Name              : cnd_notifyAll
 *
 * Description       : signal all tasks waiting on the condition variable
 *
 * Parameters
 *   cnd             : pointer to condition variable object
 *
 * Return            : none
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *
 ******************************************************************************/

__STATIC_INLINE
void cnd_notifyAll( cnd_t *cnd ) { cnd_give(cnd, cndAll); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus)
namespace intros {

/******************************************************************************
 *
 * Class             : ConditionVariable
 *
 * Description       : create and initialize a condition variable object
 *
 * Constructor parameters
 *                   : none
 *
 ******************************************************************************/

struct ConditionVariable : public __cnd
{
	constexpr
	ConditionVariable(): __cnd _CND_INIT() {}

	~ConditionVariable() { assert(__cnd::obj.queue == nullptr); }

	ConditionVariable( ConditionVariable&& ) = default;
	ConditionVariable( const ConditionVariable& ) = delete;
	ConditionVariable& operator=( ConditionVariable&& ) = delete;
	ConditionVariable& operator=( const ConditionVariable& ) = delete;

	void reset    ()                               {        cnd_reset    (this); }
	void kill     ()                               {        cnd_kill     (this); }
	template<typename T>
	int  waitFor  ( mtx_t& _mtx, const T& _delay ) { return cnd_waitFor  (this, &_mtx, Clock::count(_delay)); }
	template<typename T>
	int  waitUntil( mtx_t& _mtx, const T& _time )  { return cnd_waitUntil(this, &_mtx, Clock::until(_time)); }
	int  wait     ( mtx_t& _mtx )                  { return cnd_wait     (this, &_mtx); }
	void give     ( bool   _all = cndAll )         {        cnd_give     (this,  _all); }
	void notifyOne()                               {        cnd_notifyOne(this); }
	void notifyAll()                               {        cnd_notifyAll(this); }
	template<typename T, typename F>
	int  waitFor  ( mtx_t& _mtx, const T& _delay, F _stopWaiting )
	{
		auto _time = Clock::count(_delay);
		return (_time == INFINITE) ? wait(_mtx, _stopWaiting)
		                           : waitUntil(_mtx, sys_time() + _time, _stopWaiting);
	}
	template<typename T, typename F>
	int  waitUntil( mtx_t& _mtx, const T& _time, F _stopWaiting )
	{
		int result;
		while (!_stopWaiting() && (result = waitUntil(_mtx, _time)) == E_SUCCESS);
		return result;
	}
	template<typename F>
	int  wait     ( mtx_t& _mtx, F _stopWaiting )
	{
		int result;
		while (!_stopWaiting() && (result = wait(_mtx)) == E_SUCCESS);
		return result;
	}
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_CND_H
