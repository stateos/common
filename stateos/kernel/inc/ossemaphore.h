/******************************************************************************

    @file    StateOS: ossemaphore.h
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

#ifndef __STATEOS_SEM_H
#define __STATEOS_SEM_H

#include "oskernel.h"
#include "osclock.h"

/* -------------------------------------------------------------------------- */

#define semDirect    ( 0U )   // direct semaphore (producer can't free the semaphore, it can post the semaphore only to the pending task)
#define semBinary    ( 1U )   // binary semaphore
#define semCounting  ( 0U-1 ) // counting semaphore
#define semDefault     semCounting

/******************************************************************************
 *
 * Name              : semaphore
 *                     like a POSIX semaphore
 *
 ******************************************************************************/

typedef struct __sem sem_t;

struct __sem
{
	obj_t    obj;   // object header

	unsigned count; // current value of the semaphore counter
	unsigned limit; // limit value of the semaphore counter
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

/******************************************************************************
 *
 * Name              : OS_SEM
 * Static alias      : static_SEM
 *
 * Description       : define and initialize a semaphore object
 *
 * Parameters
 *   sem             : name of a pointer to semaphore object
 *   init            : initial value of semaphore counter
 *   limit           : (optional) maximum value of semaphore counter
 *                     semDirect: direct semaphore
 *                     semBinary: binary semaphore
 *                     semCounting: counting semaphore (default)
 *                     otherwise: limited semaphore
 *
 ******************************************************************************/

#define             OS_SEM( sem, init, ... ) \
                       sem_t sem[] = { _SEM_INIT( init, _VA_SEM(__VA_ARGS__) ) }

#define         static_SEM( sem, init, ... ) \
                static sem_t sem[] = { _SEM_INIT( init, _VA_SEM(__VA_ARGS__) ) }

/******************************************************************************
 *
 * Name              : SEM_INIT
 *
 * Description       : create and initialize a semaphore object
 *
 * Parameters
 *   init            : initial value of semaphore counter
 *   limit           : (optional) maximum value of semaphore counter
 *                     semDirect: direct semaphore
 *                     semBinary: binary semaphore
 *                     semCounting: counting semaphore (default)
 *                     otherwise: limited semaphore
 *
 * Return            : semaphore object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                SEM_INIT( init, ... ) \
                      _SEM_INIT( init, _VA_SEM(__VA_ARGS__) )
#endif

/******************************************************************************
 *
 * Name              : SEM_CREATE
 * Alias             : SEM_NEW
 *
 * Description       : create and initialize a semaphore object
 *
 * Parameters
 *   init            : initial value of semaphore counter
 *   limit           : (optional) maximum value of semaphore counter
 *                     semDirect: direct semaphore
 *                     semBinary: binary semaphore
 *                     semCounting: counting semaphore (default)
 *                     otherwise: limited semaphore
 *
 * Return            : semaphore object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                SEM_CREATE( init, ... ) \
                     { SEM_INIT  ( init, _VA_SEM(__VA_ARGS__) ) }
#define                SEM_NEW \
                       SEM_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : sem_init
 *
 * Description       : initialize a semaphore object
 *
 * Parameters
 *   sem             : pointer to semaphore object
 *   init            : initial value of semaphore counter
 *   limit           : maximum value of semaphore counter
 *                     semDirect: direct semaphore
 *                     semBinary: binary semaphore
 *                     semCounting: counting semaphore
 *                     otherwise: limited semaphore
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void sem_init( sem_t *sem, unsigned init, unsigned limit );

/******************************************************************************
 *
 * Name              : sem_create
 * Alias             : sem_new
 *
 * Description       : create and initialize a new semaphore object
 *
 * Parameters
 *   init            : initial value of semaphore counter
 *   limit           : maximum value of semaphore counter
 *                     semDirect: direct semaphore
 *                     semBinary: binary semaphore
 *                     semCounting: counting semaphore
 *                     otherwise: limited semaphore
 *
 * Return            : pointer to semaphore object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

sem_t *sem_create( unsigned init, unsigned limit );

__STATIC_INLINE
sem_t *sem_new( unsigned init, unsigned limit ) { return sem_create(init, limit); }

/******************************************************************************
 *
 * Name              : sem_reset
 * Alias             : sem_kill
 *
 * Description       : reset the semaphore object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   sem             : pointer to semaphore object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void sem_reset( sem_t *sem );

__STATIC_INLINE
void sem_kill( sem_t *sem ) { sem_reset(sem); }

/******************************************************************************
 *
 * Name              : sem_destroy
 * Alias             : sem_delete
 *
 * Description       : reset the semaphore object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * Parameters
 *   sem             : pointer to semaphore object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void sem_destroy( sem_t *sem );

__STATIC_INLINE
void sem_delete( sem_t *sem ) { sem_destroy(sem); }

/******************************************************************************
 *
 * Name              : sem_take
 * Alias             : sem_tryWait
 * ISR alias         : sem_takeISR
 * Async alias       : sem_takeAsync
 *
 * Description       : try to lock the semaphore object,
 *                     don't wait if the semaphore object can't be locked immediately
 *
 * Parameters
 *   sem             : pointer to semaphore object
 *
 * Return
 *   E_SUCCESS       : semaphore object was successfully locked
 *   E_TIMEOUT       : semaphore object can't be locked immediately, try again
 *
 * Note              : can be used in both thread and handler mode
 *                     use ISR alias in blockable interrupt handlers
 *                     use Async alias for communication with unmasked interrupt handlers
 *
 ******************************************************************************/

int sem_take( sem_t *sem );

__STATIC_INLINE
int sem_tryWait( sem_t *sem ) { return sem_take(sem); }

__STATIC_INLINE
int sem_takeISR( sem_t *sem ) { return sem_take(sem); }

#if OS_ATOMICS
int sem_takeAsync( sem_t *sem );
#endif

/******************************************************************************
 *
 * Name              : sem_waitFor
 *
 * Description       : try to lock the semaphore object,
 *                     wait for given duration of time if the semaphore object can't be locked immediately
 *
 * Parameters
 *   sem             : pointer to semaphore object
 *   delay           : duration of time (maximum number of ticks to wait for lock the semaphore object)
 *                     IMMEDIATE: don't wait if the semaphore object can't be locked immediately
 *                     INFINITE:  wait indefinitely until the semaphore object has been locked
 *
 * Return
 *   E_SUCCESS       : semaphore object was successfully locked
 *   E_STOPPED       : semaphore object was reseted before the specified timeout expired
 *   E_DELETED       : semaphore object was deleted before the specified timeout expired
 *   E_TIMEOUT       : semaphore object was not locked before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int sem_waitFor( sem_t *sem, cnt_t delay );

/******************************************************************************
 *
 * Name              : sem_waitUntil
 *
 * Description       : try to lock the semaphore object,
 *                     wait until given timepoint if the semaphore object can't be locked immediately
 *
 * Parameters
 *   sem             : pointer to semaphore object
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : semaphore object was successfully locked
 *   E_STOPPED       : semaphore object was reseted before the specified timeout expired
 *   E_DELETED       : semaphore object was deleted before the specified timeout expired
 *   E_TIMEOUT       : semaphore object was not locked before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int sem_waitUntil( sem_t *sem, cnt_t time );

/******************************************************************************
 *
 * Name              : sem_wait
 * Async alias       : sem_waitAsync
 *
 * Description       : try to lock the semaphore object,
 *                     wait indefinitely if the semaphore object can't be locked immediately
 *
 * Parameters
 *   sem             : pointer to semaphore object
 *
 * Return
 *   E_SUCCESS       : semaphore object was successfully locked
 *   E_STOPPED       : semaphore object was reseted (unavailable for async version)
 *   E_DELETED       : semaphore object was deleted (unavailable for async version)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
int sem_wait( sem_t *sem ) { return sem_waitFor(sem, INFINITE); }

#if OS_ATOMICS
int sem_waitAsync( sem_t *sem );
#endif

/******************************************************************************
 *
 * Name              : sem_give
 * Alias             : sem_post
 * ISR alias         : sem_giveISR
 * Async alias       : sem_giveAsync
 *
 * Description       : try to unlock the semaphore object,
 *                     don't wait if the semaphore object can't be unlocked immediately
 *
 * Parameters
 *   sem             : pointer to semaphore object
 *
 * Return
 *   E_SUCCESS       : semaphore object was successfully unlocked
 *   E_TIMEOUT       : semaphore object can't be unlocked immediately, try again
 *
 * Note              : can be used in both thread and handler mode
 *                     use ISR alias in blockable interrupt handlers
 *                     use Async alias for communication with unmasked interrupt handlers
 *
 ******************************************************************************/

int sem_give( sem_t *sem );

__STATIC_INLINE
int sem_post( sem_t *sem ) { return sem_give(sem); }

__STATIC_INLINE
int sem_giveISR( sem_t *sem ) { return sem_give(sem); }

#if OS_ATOMICS
int sem_giveAsync( sem_t *sem );
#endif

/******************************************************************************
 *
 * Name              : sem_release
 * ISR alias         : sem_releaseISR
 *
 * Description       : try to increment the internal semaphore counter by the value of num
 *
 * Parameters
 *   sem             : pointer to semaphore object
 *   num             : update value of semaphore counter
 *
 * Return
 *   E_SUCCESS       : semaphore counter was successfully updated
 *   E_TIMEOUT       : semaphore counter was updated to the limit value
 *
 * Note              : can be used in both thread and handler mode
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

int sem_release( sem_t *sem, unsigned num );

__STATIC_INLINE
int sem_releaseISR( sem_t *sem, unsigned num ) { return sem_release(sem, num); }

/******************************************************************************
 *
 * Name              : sem_getValue
 *
 * Description       : return current value of semaphore
 *
 * Parameters
 *   sem             : pointer to semaphore object
 *
 * Return            : current value of semaphore
 *
 ******************************************************************************/

unsigned sem_getValue( sem_t *sem );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace stateos {

/******************************************************************************
 *
 * Class             : Semaphore
 *
 * Description       : create and initialize a semaphore object
 *
 * Constructor parameters
 *   init            : initial value of semaphore counter
 *   limit           : maximum value of semaphore counter
 *                     semDirect: direct semaphore
 *                     semBinary: binary semaphore
 *                     semCounting: counting semaphore (default)
 *                     otherwise: limited semaphore
 *
 ******************************************************************************/

struct Semaphore : public __sem
{
	constexpr
	Semaphore(): __sem _SEM_INIT(0, semDefault) {}
	constexpr
	Semaphore( const unsigned _init, const unsigned _limit = semDefault ): __sem _SEM_INIT(_init, _limit) {}

	~Semaphore() { assert(__sem::obj.queue == nullptr); }

	Semaphore( Semaphore&& ) = default;
	Semaphore( const Semaphore& ) = delete;
	Semaphore& operator=( Semaphore&& ) = delete;
	Semaphore& operator=( const Semaphore& ) = delete;

	void     reset     ()                  {        sem_reset     (this); }
	void     kill      ()                  {        sem_kill      (this); }
	void     destroy   ()                  {        sem_destroy   (this); }
	int      take      ()                  { return sem_take      (this); }
	int      tryWait   ()                  { return sem_tryWait   (this); }
	int      takeISR   ()                  { return sem_takeISR   (this); }
	template<typename T>
	int      waitFor   ( const T& _delay ) { return sem_waitFor   (this, Clock::count(_delay)); }
	template<typename T>
	int      waitUntil ( const T& _time )  { return sem_waitUntil (this, Clock::until(_time)); }
	int      wait      ()                  { return sem_wait      (this); }
	int      give      ()                  { return sem_give      (this); }
	int      post      ()                  { return sem_post      (this); }
	int      giveISR   ()                  { return sem_giveISR   (this); }
	int      release   ( unsigned _num )   { return sem_release   (this, _num); }
	int      releaseISR( unsigned _num )   { return sem_releaseISR(this, _num); }
	unsigned getValue  ()                  { return sem_getValue  (this); }
#if OS_ATOMICS
	int      takeAsync ()                  { return sem_takeAsync (this); }
	int      waitAsync ()                  { return sem_waitAsync (this); }
	int      giveAsync ()                  { return sem_giveAsync (this); }
#endif

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<Semaphore>;
#else
	using Ptr = Semaphore *;
#endif

/******************************************************************************
 *
 * Name              : Semaphore::Direct
 *
 * Description       : create and initialize static direct semaphore
 *
 * Parameters        : none
 *
 * Return            : Semaphore object
 *
 ******************************************************************************/

	static
	Semaphore Direct()
	{
		return { 0, semDirect };
	}

/******************************************************************************
 *
 * Name              : Semaphore::Binary
 *
 * Description       : create and initialize static binary semaphore
 *
 * Parameters
 *   init            : initial value of semaphore counter
 *
 * Return            : Semaphore object
 *
 ******************************************************************************/

	static
	Semaphore Binary( const unsigned _init = 0 )
	{
		return { _init, semBinary };
	}

/******************************************************************************
 *
 * Name              : Semaphore::Counting
 *
 * Description       : create and initialize static counting semaphore
 *
 * Parameters
 *   init            : initial value of semaphore counter
 *
 * Return            : Semaphore object
 *
 ******************************************************************************/

	static
	Semaphore Counting( const unsigned _init = 0 )
	{
		return { _init, semCounting };
	}

/******************************************************************************
 *
 * Name              : Semaphore::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters
 *   init            : initial value of semaphore counter
 *
 * Return            : std::unique_pointer / pointer to Semaphore object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

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

#endif//__STATEOS_SEM_H
