/******************************************************************************

    @file    IntrOS: ossemaphore.h
    @author  Rajmund Szymanski
    @date    18.05.2021
    @brief   This file contains definitions for IntrOS.

 ******************************************************************************

   Copyright (c) 2018-2021 Rajmund Szymanski. All rights reserved.

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

/* -------------------------------------------------------------------------- */

#define semBinary    ( 1U )   // binary semaphore
#define semCounting  ( 0U-1 ) // counting semaphore
#define semDefault     semCounting

/******************************************************************************
 *
 * Name              : semaphore
 *
 ******************************************************************************/

typedef struct __sem sem_t, * const sem_id;

struct __sem
{
	unsigned count; // current value of the semaphore counter
	unsigned limit; // limit value of the semaphore counter
};

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : _SEM_INIT
 *
 * Description       : create and initialize a semaphore object
 *
 * Parameters
 *   init            : initial value of semaphore counter
 *   limit           : maximum value of semaphore counter
 *                     semBinary: binary semaphore
 *                     semCounting: counting semaphore
 *                     otherwise: limited semaphore
 *
 * Return            : semaphore object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _SEM_INIT( _init, _limit ) { _init < _limit ? _init : _limit, _limit }

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
 *                     semBinary: binary semaphore
 *                     semCounting: counting semaphore (default)
 *                     otherwise: limited semaphore
 *
 ******************************************************************************/

#define             OS_SEM( sem, init, ... )                                      \
                       sem_t sem##__sem = _SEM_INIT( init, _VA_SEM(__VA_ARGS__) ); \
                       sem_id sem = & sem##__sem

#define         static_SEM( sem, init, ... )                                      \
                static sem_t sem##__sem = _SEM_INIT( init, _VA_SEM(__VA_ARGS__) ); \
                static sem_id sem = & sem##__sem

/******************************************************************************
 *
 * Name              : SEM_INIT
 *
 * Description       : create and initialize a semaphore object
 *
 * Parameters
 *   init            : initial value of semaphore counter
 *   limit           : (optional) maximum value of semaphore counter
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
 *                     semBinary: binary semaphore
 *                     semCounting: counting semaphore (default)
 *                     otherwise: limited semaphore
 *
 * Return            : pointer to semaphore object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                SEM_CREATE( init, ... ) \
           (sem_t[]) { SEM_INIT  ( init, _VA_SEM(__VA_ARGS__) ) }
#define                SEM_NEW \
                       SEM_CREATE
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
 *                     semBinary: binary semaphore
 *                     semCounting: counting semaphore
 *                     otherwise: limited semaphore
 *
 * Return            : none
 *
 ******************************************************************************/

void sem_init( sem_t *sem, unsigned init, unsigned limit );

/******************************************************************************
 *
 * Name              : sem_take
 * Alias             : sem_tryWait
 * Async alias       : sem_takeAsync
 *
 * Description       : try to lock the semaphore object,
 *                     don't wait if the semaphore object can't be locked immediately
 *
 * Parameters
 *   sem             : pointer to semaphore object
 *
 * Return
 *   SUCCESS         : semaphore object was successfully locked
 *   FAILURE         : semaphore object can't be locked immediately
 *
 ******************************************************************************/

unsigned sem_take( sem_t *sem );

__STATIC_INLINE
unsigned sem_tryWait( sem_t *sem ) { return sem_take(sem); }

#if OS_ATOMICS
unsigned sem_takeAsync( sem_t *sem );
#endif

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
 * Return            : none
 *
 ******************************************************************************/

void sem_wait( sem_t *sem );

#if OS_ATOMICS
void sem_waitAsync( sem_t *sem );
#endif

/******************************************************************************
 *
 * Name              : sem_give
 * Alias             : sem_post
 * Async alias       : sem_giveAsync
 *
 * Description       : try to unlock the semaphore object,
 *                     don't wait if the semaphore object can't be unlocked immediately
 *
 * Parameters
 *   sem             : pointer to semaphore object
 *
 * Return
 *   SUCCESS         : semaphore object was successfully unlocked
 *   FAILURE         : semaphore object can't be unlocked immediately
 *
 ******************************************************************************/

unsigned sem_give( sem_t *sem );

__STATIC_INLINE
unsigned sem_post( sem_t *sem ) { return sem_give(sem); }

#if OS_ATOMICS
unsigned sem_giveAsync( sem_t *sem );
#endif

/******************************************************************************
 *
 * Name              : sem_release
 *
 * Description       : try to increment the internal semaphore counter by the value of num
 *
 * Parameters
 *   sem             : pointer to semaphore object
 *   num             : update value of semaphore counter
 *
 * Return
 *   SUCCESS         : semaphore counter was successfully updated
 *   FAILURE         : semaphore counter was updated to the limit value
 *
 ******************************************************************************/

unsigned sem_release( sem_t *sem, unsigned num );

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

#ifdef __cplusplus
namespace intros {

/******************************************************************************
 *
 * Class             : Semaphore
 *
 * Description       : create and initialize a semaphore object
 *
 * Constructor parameters
 *   init            : initial value of semaphore counter
 *   limit           : maximum value of semaphore counter
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

	Semaphore( Semaphore&& ) = default;
	Semaphore( const Semaphore& ) = delete;
	Semaphore& operator=( Semaphore&& ) = delete;
	Semaphore& operator=( const Semaphore& ) = delete;

	unsigned take     ()                { return sem_take     (this); }
	unsigned tryWait  ()                { return sem_tryWait  (this); }
	void     wait     ()                {        sem_wait     (this); }
	unsigned give     ()                { return sem_give     (this); }
	unsigned post     ()                { return sem_post     (this); }
	unsigned release  ( unsigned _num ) { return sem_release  (this, _num); }
	unsigned getValue ()                { return sem_getValue (this); }
#if OS_ATOMICS
	unsigned takeAsync()                { return sem_takeAsync(this); }
	void     waitAsync()                {        sem_waitAsync(this); }
	unsigned giveAsync()                { return sem_giveAsync(this); }
#endif

/******************************************************************************
 *
 * Name              : Semaphore::Binary
 *
 * Description       : create and initialize binary semaphore object
 *
 * Parameters
 *   init            : initial value of semaphore counter
 *
 * Return            : Semaphore object
 *
 ******************************************************************************/

	static constexpr
	Semaphore Binary( const unsigned _init = 0 )
	{
		return { _init, semBinary };
	}

/******************************************************************************
 *
 * Name              : Semaphore::Counting
 *
 * Description       : create and initialize counting semaphore object
 *
 * Parameters
 *   init            : initial value of semaphore counter
 *
 * Return            : Semaphore object
 *
 ******************************************************************************/

	static constexpr
	Semaphore Counting( const unsigned _init = 0 )
	{
		return { _init, semCounting };
	}

};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_SEM_H
