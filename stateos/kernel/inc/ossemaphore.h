/******************************************************************************

    @file    StateOS: ossemaphore.h
    @author  Rajmund Szymanski
    @date    18.11.2025
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

#ifndef __STATEOS_SEM_H
#define __STATEOS_SEM_H

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
 *
 * @note    use only in thread mode
 */

void sem_init( sem_t *sem, unsigned init, unsigned limit );

/**
 * @brief   create and initialize a new semaphore object
 *
 * @param   init       initial value of semaphore counter
 * @param   limit      maximum value of semaphore counter
 *                     - semDirect: direct semaphore
 *                     - semBinary: binary semaphore
 *                     - semCounting: counting semaphore
 *                     - otherwise: limited semaphore
 *
 * @return  pointer to semaphore object
 * @retval  NULL       object not created (not enough free memory)
 *
 * @note    use only in thread mode
 */

sem_t *sem_create( unsigned init, unsigned limit );

/** alias of sem_create() */
__STATIC_INLINE
sem_t *sem_new( unsigned init, unsigned limit ) { return sem_create(init, limit); }

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
 * @brief   reset the semaphore object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * @param   sem        pointer to semaphore object
 *
 * @note    use only in thread mode
 */

void sem_destroy( sem_t *sem );

/** alias of sem_destroy() */
__STATIC_INLINE
void sem_delete( sem_t *sem ) { sem_destroy(sem); }

/**
 * @brief   try to lock the semaphore object,
 *          don't wait if the semaphore object can't be locked immediately
 *
 * @param   sem        pointer to semaphore object
 *
 * @retval  E_SUCCESS  semaphore object was successfully locked
 * @retval  E_TIMEOUT  semaphore object can't be locked immediately, try again
 *
 * @note    can be used in both thread and handler mode
 * @note    use ISR alias in blockable interrupt handlers
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

int sem_take( sem_t *sem );

/** alias of sem_take() */
__STATIC_INLINE
int sem_tryWait( sem_t *sem ) { return sem_take(sem); }

/** ISR alias of sem_take() */
__STATIC_INLINE
int sem_takeISR( sem_t *sem ) { return sem_take(sem); }

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
 * @retval  E_DELETED  semaphore object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  semaphore object was not locked before the specified timeout expired
 *
 * @note    use only in thread mode
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
 * @retval  E_DELETED  semaphore object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  semaphore object was not locked before the specified timeout expired
 *
 * @note    use only in thread mode
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
 * @retval  E_DELETED  semaphore object was deleted (unavailable for async version)
 *
 * @note    use only in thread mode
 * @note    use Async alias for communication with unmasked interrupt handlers
 * @note    the Async alias is intended for tasks of equal priority: it waits in a loop with tsk_yield,
 *          lower priority tasks do not run meanwhile
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
 * @note    can be used in both thread and handler mode
 * @note    use ISR alias in blockable interrupt handlers
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

int sem_give( sem_t *sem );

/** alias of sem_give() */
__STATIC_INLINE
int sem_post( sem_t *sem ) { return sem_give(sem); }

/** ISR alias of sem_give() */
__STATIC_INLINE
int sem_giveISR( sem_t *sem ) { return sem_give(sem); }

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
 *
 * @note    can be used in both thread and handler mode
 * @note    use ISR alias in blockable interrupt handlers
 */

int sem_release( sem_t *sem, unsigned num );

/** ISR alias of sem_release() */
__STATIC_INLINE
int sem_releaseISR( sem_t *sem, unsigned num ) { return sem_release(sem, num); }

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

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(__STATEOS_STDCXX)
namespace stateos {

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
	void     destroy   ()                  {        sem_destroy   (this); }                       ///< sem_destroy()
	int      take      ()                  { return sem_take      (this); }                       ///< sem_take()
	int      tryWait   ()                  { return sem_tryWait   (this); }                       ///< sem_tryWait()
	int      takeISR   ()                  { return sem_takeISR   (this); }                       ///< sem_takeISR()
	template<typename T>
	int      waitFor   ( const T& _delay ) { return sem_waitFor   (this, Clock::count(_delay)); } ///< sem_waitFor()
	template<typename T>
	int      waitUntil ( const T& _time )  { return sem_waitUntil (this, Clock::until(_time)); }  ///< sem_waitUntil()
	int      wait      ()                  { return sem_wait      (this); }                       ///< sem_wait()
	int      give      ()                  { return sem_give      (this); }                       ///< sem_give()
	int      post      ()                  { return sem_post      (this); }                       ///< sem_post()
	int      giveISR   ()                  { return sem_giveISR   (this); }                       ///< sem_giveISR()
	int      release   ( unsigned _num )   { return sem_release   (this, _num); }                 ///< sem_release()
	int      releaseISR( unsigned _num )   { return sem_releaseISR(this, _num); }                 ///< sem_releaseISR()
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

/**
 * @brief   create dynamic object with manageable resources
 *
 * @param   _init      initial value of semaphore counter
 * @param   _limit     maximum value of semaphore counter (default: semCounting)
 *
 * @return  std::unique_pointer / pointer to Semaphore object
 *
 * @note    use only in thread mode
 */

	static
	Ptr Create( const unsigned _init, const unsigned _limit = semDefault )
	{
		auto sem = new (std::nothrow) Semaphore(_init, _limit);
		if (sem != nullptr)
			sem->__sem::obj.res = sem;
		return Ptr(sem);
	}

};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

/** @} */

#endif//__STATEOS_SEM_H
