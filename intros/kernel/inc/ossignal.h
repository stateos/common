/******************************************************************************

    @file    IntrOS: ossignal.h
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

#ifndef __INTROS_SIG_H
#define __INTROS_SIG_H

#include "oskernel.h"
#include "osclock.h"

/* -------------------------------------------------------------------------- */

#define SIG_LIMIT     (sizeof(unsigned) * CHAR_BIT)

#define SIGSET(signo) (((signo) < (SIG_LIMIT)) ? 1U << (signo) : 0U) // signal mask from the given signal number
#define sigAny        ( 0U )                                         // signal mask for any signal
#define sigAll        ( 0U-1 )                                       // signal mask for all signals

/******************************************************************************
 *
 * Name              : signal
 *
 ******************************************************************************/

typedef struct __sig sig_t;

struct __sig
{
	obj_t    obj;   // object header

	unsigned sigset;// pending signals
	unsigned mask;  // protection mask
};

typedef struct __sig sig_id [];

/******************************************************************************
 *
 * Name              : _SIG_INIT
 *
 * Description       : create and initialize a signal object
 *
 * Parameters
 *   mask            : protection mask of signal object
 *
 * Return            : signal object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _SIG_INIT( _mask ) { _OBJ_INIT(), 0, _mask }

/******************************************************************************
 *
 * Name              : _VA_SIG
 *
 * Description       : calculate protection mask from optional parameter
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _VA_SIG( _mask ) ( _mask + 0 )

/******************************************************************************
 *
 * Name              : OS_SIG
 * Static alias      : static_SIG
 *
 * Description       : define and initialize a signal object
 *
 * Parameters
 *   sig             : name of a pointer to signal object
 *   mask            : (optional) protection mask; default: 0
 *
 ******************************************************************************/

#define             OS_SIG( sig, ... ) \
                       sig_t sig[] = { _SIG_INIT( _VA_SIG(__VA_ARGS__) ) }

#define         static_SIG( sig, ... ) \
                static sig_t sig[] = { _SIG_INIT( _VA_SIG(__VA_ARGS__) ) }

/******************************************************************************
 *
 * Name              : SIG_INIT
 *
 * Description       : create and initialize a signal object
 *
 * Parameters
 *   mask            : (optional) protection mask; default: 0
 *
 * Return            : signal object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                SIG_INIT( ... ) \
                      _SIG_INIT( _VA_SIG(__VA_ARGS__) )
#endif

/******************************************************************************
 *
 * Name              : SIG_CREATE
 * Alias             : SIG_NEW
 *
 * Description       : create and initialize a signal object
 *
 * Parameters
 *   mask            : (optional) protection mask; default: 0
 *
 * Return            : signal object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                SIG_CREATE( ... ) \
                     { SIG_INIT  ( _VA_SIG(__VA_ARGS__) ) }
#define                SIG_NEW \
                       SIG_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : sig_init
 *
 * Description       : initialize a signal object
 *
 * Parameters
 *   sig             : pointer to signal object
 *   mask            : protection mask of signal object
 *
 * Return            : none
 *
 ******************************************************************************/

void sig_init( sig_t *sig, unsigned mask );

/******************************************************************************
 *
 * Name              : sig_reset
 * Alias             : sig_kill
 *
 * Description       : reset the signal object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   sig             : pointer to signal object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void sig_reset( sig_t *sig );

__STATIC_INLINE
void sig_kill( sig_t *sig ) { sig_reset(sig); }

/******************************************************************************
 *
 * Name              : sig_take
 * Alias             : sig_tryWait
 *
 * Description       : check signal object for a given set of signals
 *                     and return the lowest one in the 'signo' variable
 *
 * Parameters
 *   sig             : pointer to signal object
 *   sigset          : set of expected signals
 *   signo           : pointer to the variable getting signal number
 *
 * Return
 *   E_SUCCESS       : variable 'singno' contains the lowest number of expected signal from the set of all pending signals
 *   E_TIMEOUT       : no expected signal has been set, try again
 *
 ******************************************************************************/

int sig_take( sig_t *sig, unsigned sigset, unsigned *signo );

__STATIC_INLINE
int sig_tryWait( sig_t *sig, unsigned sigset, unsigned *signo ) { return sig_take(sig, sigset, signo); }

/******************************************************************************
 *
 * Name              : sig_waitFor
 *
 * Description       : wait for a signal from the given set of signals for given duration of time
 *                     and return the lowest one in the 'signo' variable
 *
 * Parameters
 *   sig             : pointer to signal object
 *   sigset          : set of expected signals
 *   signo           : pointer to the variable getting signal number
 *   delay           : duration of time (maximum number of ticks to wait for release the signal object)
 *                     IMMEDIATE: don't wait until the signal object has been released
 *                     INFINITE:  wait indefinitely until the signal object has been released
 *
 * Return
 *   E_SUCCESS       : variable 'singno' contains the lowest number of expected signal from the set of all pending signals
 *   E_STOPPED       : signal object was reseted before the specified timeout expired
 *   E_TIMEOUT       : no expected signal has been set before the specified timeout expired
 *
 ******************************************************************************/

int sig_waitFor( sig_t *sig, unsigned sigset, unsigned *signo, cnt_t delay );

/******************************************************************************
 *
 * Name              : sig_waitUntil
 *
 * Description       : wait for a signal from the given set of signals until given timepoint
 *                     and return the lowest one in the 'signo' variable
 *
 * Parameters
 *   sig             : pointer to signal object
 *   sigset          : set of expected signals
 *   signo           : pointer to the variable getting signal number
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : variable 'singno' contains the lowest number of expected signal from the set of all pending signals
 *   E_STOPPED       : signal object was reseted before the specified timeout expired
 *   E_TIMEOUT       : no expected signal has been set before the specified timeout expired
 *
 ******************************************************************************/

int sig_waitUntil( sig_t *sig, unsigned sigset, unsigned *signo, cnt_t time );

/******************************************************************************
 *
 * Name              : sig_wait
 *
 * Description       : wait indefinitely for a signal from the given set of signals
 *                     and return the lowest one in the 'signo' variable
 *
 * Parameters
 *   sig             : pointer to signal object
 *   sigset          : set of expected signals
 *   signo           : pointer to the variable getting signal number
 *
 * Return
 *   E_SUCCESS       : variable 'singno' contains the lowest number of expected signal from the set of all pending signals
 *   E_STOPPED       : signal object was reseted
 *
 ******************************************************************************/

__STATIC_INLINE
int sig_wait( sig_t *sig, unsigned sigset, unsigned *signo ) { return sig_waitFor(sig, sigset, signo, INFINITE); }

/******************************************************************************
 *
 * Name              : sig_give
 * Alias             : sig_set
 *
 * Description       : set given signal in the signal object
 *
 * Parameters
 *   sig             : pointer to signal object
 *   signo           : signal number to set
 *
 * Return            : none
 *
 ******************************************************************************/

void sig_give( sig_t *sig, unsigned signo );

__STATIC_INLINE
void sig_set( sig_t *sig, unsigned signo ) { sig_give(sig, signo); }

/******************************************************************************
 *
 * Name              : sig_clear
 *
 * Description       : reset given signal in the signal object
 *
 * Parameters
 *   sig             : pointer to signal object
 *   signo           : signal number to clear
 *
 * Return            : none
 *
 ******************************************************************************/

void sig_clear( sig_t *sig, unsigned signo );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus)
namespace intros {

/******************************************************************************
 *
 * Class             : Signal
 *
 * Description       : create and initialize a signal object
 *
 * Constructor parameters
 *   mask            : protection mask of signal object
 *
 ******************************************************************************/

struct Signal : public __sig
{
	constexpr
	Signal( const unsigned _mask = 0 ): __sig _SIG_INIT(_mask) {}

	~Signal() { assert(__sig::obj.queue == nullptr); }

	Signal( Signal&& ) = default;
	Signal( const Signal& ) = delete;
	Signal& operator=( Signal&& ) = delete;
	Signal& operator=( const Signal& ) = delete;

	void reset    ()                                                       {        sig_reset    (this); }
	void kill     ()                                                       {        sig_kill     (this); }
	int  take     ( unsigned _sigset, unsigned *_signo = nullptr )         { return sig_take     (this, _sigset, _signo); }
	int  tryWait  ( unsigned _sigset, unsigned *_signo = nullptr )         { return sig_tryWait  (this, _sigset, _signo); }
	template<typename T>
	int  waitFor  ( unsigned _sigset, unsigned *_signo,  const T& _delay ) { return sig_waitFor  (this, _sigset, _signo, Clock::count(_delay)); }
	template<typename T>
	int  waitUntil( unsigned _sigset, unsigned *_signo,  const T& _time )  { return sig_waitUntil(this, _sigset, _signo, Clock::until(_time)); }
	int  wait     ( unsigned _sigset, unsigned *_signo = nullptr )         { return sig_wait     (this, _sigset, _signo); }
	void give     ( unsigned _signo )                                      {        sig_give     (this, _signo); }
	void set      ( unsigned _signo )                                      {        sig_set      (this, _signo); }
	void clear    ( unsigned _signo )                                      {        sig_clear    (this, _signo); }
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_SIG_H
