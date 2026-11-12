/******************************************************************************

    @file    IntrOS: ossemaphore.h
    @author  Rajmund Szymanski
    @date    18.11.2025
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

#ifndef __INTROS_SEM_H
#define __INTROS_SEM_H

#include "oskernel.h"
#include "osclock.h"

/**
 * @defgroup semaphore Semaphore
 * @brief    semaphores (binary, limited, counting)
 * @{
 */

/* -------------------------------------------------------------------------- */

#define semDirect    ( 0U )   ///< direct semaphore (producer can't free the semaphore, it can post the semaphore only to the pending task)
#define semBinary    ( 1U )   ///< binary semaphore
#define semCounting  ( 0U-1 ) ///< counting semaphore
#define semDefault     semCounting ///< default semaphore type

/**
 * @brief   semaphore
 */

typedef struct __sem sem_t;

struct __sem
{
	obj_t    obj;   ///< object header

	unsigned count; ///< current value of the semaphore counter
	unsigned limit; ///< limit value of the semaphore counter
};

typedef struct __sem sem_id [];

/******************************************************************************
 *
 * Name              : _SEM_INIT
 *
 * Description       : create and initialize a semaphore object
 *
 * Parameters
 *   init            : initial value of semaphore counter
 *   limit           : maximum value of semaphore counter
 *                     semDirect: direct semaphore
 *                     semBinary: binary semaphore
 *                     semCounting: counting semaphore
 *                     otherwise: limited semaphore
 *
 * Return            : semaphore object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _SEM_INIT( _init, _limit ) { _OBJ_INIT(), _init < _limit ? _init : _limit, _limit }

/******************************************************************************
 *
 * Name              : _VA_SEM
 *
 * Description       : calculate maximum value of semaphore counter from optional parameter
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _VA_SEM( _limit ) ( (_limit + 0) ? (_limit + 0) : (semDefault) )

/**
 * @brief   define and initialize a semaphore object
 *
 * @param   sem        name of a pointer to semaphore object
 * @param   init       initial value of semaphore counter
 * @param   ...        limit: (optional) maximum value of semaphore counter
 *                     - semDirect: direct semaphore
 *                     - semBinary: binary semaphore
 *                     - semCounting: counting semaphore (default)
 *                     - otherwise: limited semaphore
 */

#define             OS_SEM( sem, init, ... ) \
                       sem_t sem[] = { _SEM_INIT( init, _VA_SEM(__VA_ARGS__) ) }

/** static alias of OS_SEM() */
#define         static_SEM( sem, init, ... ) \
                static sem_t sem[] = { _SEM_INIT( init, _VA_SEM(__VA_ARGS__) ) }

/**
 * @brief   create and initialize a semaphore object
 *
 * @param   init       initial value of semaphore counter
 * @param   ...        limit: (optional) maximum value of semaphore counter
 *                     - semDirect: direct semaphore
 *                     - semBinary: binary semaphore
 *                     - semCounting: counting semaphore (default)
 *                     - otherwise: limited semaphore
 *
 * @return  semaphore object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                SEM_INIT( init, ... ) \
                      _SEM_INIT( init, _VA_SEM(__VA_ARGS__) )
#endif

/**
 * @brief   create and initialize a semaphore object
 *
 * @param   init       initial value of semaphore counter
 * @param   ...        limit: (optional) maximum value of semaphore counter
 *                     - semDirect: direct semaphore
 *                     - semBinary: binary semaphore
 *                     - semCounting: counting semaphore (default)
 *                     - otherwise: limited semaphore
 *
 * @return  semaphore object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                SEM_CREATE( init, ... ) \
                     { SEM_INIT  ( init, _VA_SEM(__VA_ARGS__) ) }
/** alias of SEM_CREATE() */
#define                SEM_NEW \
                       SEM_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize a semaphore object
 *
 * @param   sem        pointer to semaphore object
 * @param   init       initial value of semaphore counter
 * @param   limit      maximum value of semaphore counter
 *                     - semDirect: direct semaphore
 *                     - semBinary: binary semaphore
 *                     - semCounting: counting semaphore
 *                     - otherwise: limited semaphore
 */

void sem_init( sem_t *sem, unsigned init, unsigned limit );

/**
 * @brief   reset the semaphore object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * @param   sem        pointer to semaphore object
 *
 * @note    use only in thread mode
 */

void sem_reset( sem_t *sem );

/** alias of sem_reset() */
__STATIC_INLINE
void sem_kill( sem_t *sem ) { sem_reset(sem); }

/**
 * @brief   try to lock the semaphore object,
 *          don't wait if the semaphore object can't be locked immediately
 *
 * @param   sem        pointer to semaphore object
 *
 * @retval  E_SUCCESS  semaphore object was successfully locked
 * @retval  E_TIMEOUT  semaphore object can't be locked immediately, try again
 *
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

int sem_take( sem_t *sem );

/** alias of sem_take() */
__STATIC_INLINE
int sem_tryWait( sem_t *sem ) { return sem_take(sem); }

#if OS_ATOMICS
/** Async alias of sem_take() */
int sem_takeAsync( sem_t *sem );
#endif

/**
 * @brief   try to lock the semaphore object,
 *          wait for given duration of time if the semaphore object can't be locked immediately
 *
 * @param   sem        pointer to semaphore object
 * @param   delay      duration of time (maximum number of ticks to wait for lock the semaphore object)
 *                     - IMMEDIATE: don't wait if the semaphore object can't be locked immediately
 *                     - INFINITE:  wait indefinitely until the semaphore object has been locked
 *
 * @retval  E_SUCCESS  semaphore object was successfully locked
 * @retval  E_STOPPED  semaphore object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  semaphore object was not locked before the specified timeout expired
 */

int sem_waitFor( sem_t *sem, cnt_t delay );

/**
 * @brief   try to lock the semaphore object,
 *          wait until given timepoint if the semaphore object can't be locked immediately
 *
 * @param   sem        pointer to semaphore object
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  semaphore object was successfully locked
 * @retval  E_STOPPED  semaphore object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  semaphore object was not locked before the specified timeout expired
 */

int sem_waitUntil( sem_t *sem, cnt_t time );

/**
 * @brief   try to lock the semaphore object,
 *          wait indefinitely if the semaphore object can't be locked immediately
 *
 * @param   sem        pointer to semaphore object
 *
 * @retval  E_SUCCESS  semaphore object was successfully locked
 * @retval  E_STOPPED  semaphore object was reseted (unavailable for async version)
 *
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

__STATIC_INLINE
int sem_wait( sem_t *sem ) { return sem_waitFor(sem, INFINITE); }

#if OS_ATOMICS
/** Async alias of sem_wait() */
int sem_waitAsync( sem_t *sem );
#endif

/**
 * @brief   try to unlock the semaphore object,
 *          don't wait if the semaphore object can't be unlocked immediately
 *
 * @param   sem        pointer to semaphore object
 *
 * @retval  E_SUCCESS  semaphore object was successfully unlocked
 * @retval  E_TIMEOUT  semaphore object can't be unlocked immediately, try again
 *
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

int sem_give( sem_t *sem );

/** alias of sem_give() */
__STATIC_INLINE
int sem_post( sem_t *sem ) { return sem_give(sem); }

#if OS_ATOMICS
/** Async alias of sem_give() */
int sem_giveAsync( sem_t *sem );
#endif

/**
 * @brief   try to increment the internal semaphore counter by the value of num
 *
 * @param   sem        pointer to semaphore object
 * @param   num        update value of semaphore counter
 *
 * @retval  E_SUCCESS  semaphore counter was successfully updated
 * @retval  E_TIMEOUT  semaphore counter was updated to the limit value
 */

int sem_release( sem_t *sem, unsigned num );

/**
 * @brief   return current value of semaphore
 *
 * @param   sem        pointer to semaphore object
 *
 * @return  current value of semaphore
 */

unsigned sem_getValue( sem_t *sem );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus)
namespace intros {

/**
 * @brief   create and initialize a semaphore object
 *
 * @par     Constructor parameters
 *          - init: initial value of semaphore counter
 *          - limit: maximum value of semaphore counter
 *            - semDirect: direct semaphore
 *            - semBinary: binary semaphore
 *            - semCounting: counting semaphore (default)
 *            - otherwise: limited semaphore
 */

struct Semaphore : public __sem
{
	/// constructor, see the constructor parameters of the class
	constexpr
	Semaphore(): __sem _SEM_INIT(0, semDefault) {}
	/// constructor, see the constructor parameters of the class
	constexpr
	Semaphore( const unsigned _init, const unsigned _limit = semDefault ): __sem _SEM_INIT(_init, _limit) {}

	~Semaphore() { assert(__sem::obj.queue == nullptr); }

	/// move constructor
	Semaphore( Semaphore&& _src ): __sem(_src)
	{
		assert(_src.obj.queue == nullptr);
	}

	Semaphore( const Semaphore& ) = delete;
	Semaphore& operator=( Semaphore&& ) = delete;
	Semaphore& operator=( const Semaphore& ) = delete;

	void     reset     ()                  {        sem_reset     (this); }                       ///< sem_reset()
	void     kill      ()                  {        sem_kill      (this); }                       ///< sem_kill()
	int      take      ()                  { return sem_take      (this); }                       ///< sem_take()
	int      tryWait   ()                  { return sem_tryWait   (this); }                       ///< sem_tryWait()
	template<typename T>
	int      waitFor   ( const T& _delay ) { return sem_waitFor   (this, Clock::count(_delay)); } ///< sem_waitFor()
	template<typename T>
	int      waitUntil ( const T& _time )  { return sem_waitUntil (this, Clock::until(_time)); }  ///< sem_waitUntil()
	int      wait      ()                  { return sem_wait      (this); }                       ///< sem_wait()
	int      give      ()                  { return sem_give      (this); }                       ///< sem_give()
	int      post      ()                  { return sem_post      (this); }                       ///< sem_post()
	int      release   ( unsigned _num )   { return sem_release   (this, _num); }                 ///< sem_release()
	unsigned getValue  ()                  { return sem_getValue  (this); }                       ///< sem_getValue()
#if OS_ATOMICS
	int      takeAsync ()                  { return sem_takeAsync (this); }                       ///< sem_takeAsync()
	int      waitAsync ()                  { return sem_waitAsync (this); }                       ///< sem_waitAsync()
	int      giveAsync ()                  { return sem_giveAsync (this); }                       ///< sem_giveAsync()
#endif

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<Semaphore>; ///< result type of Create()
#else
	using Ptr = Semaphore *; ///< result type of Create()
#endif

/**
 * @brief   create and initialize static direct semaphore
 *
 * @return  Semaphore object
 */

	static
	Semaphore Direct()
	{
		return { 0, semDirect };
	}

/**
 * @brief   create and initialize static binary semaphore
 *
 * @param   _init      initial value of semaphore counter
 *
 * @return  Semaphore object
 */

	static
	Semaphore Binary( const unsigned _init = 0 )
	{
		return { _init, semBinary };
	}

/**
 * @brief   create and initialize static counting semaphore
 *
 * @param   _init      initial value of semaphore counter
 *
 * @return  Semaphore object
 */

	static
	Semaphore Counting( const unsigned _init = 0 )
	{
		return { _init, semCounting };
	}

};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

/** @} */

#endif//__INTROS_SEM_H
