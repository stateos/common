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

/**
 * @defgroup signal Signal
 * @brief    signals with protection mask
 * @{
 */

/* -------------------------------------------------------------------------- */

#define SIG_LIMIT     (sizeof(unsigned) * CHAR_BIT)

#define SIGSET(signo) (((signo) < (SIG_LIMIT)) ? 1U << (signo) : 0U) ///< signal mask from the given signal number
#define sigAny        ( 0U )                                         ///< signal mask for any signal
#define sigAll        ( 0U-1 )                                       ///< signal mask for all signals

/**
 * @brief   signal
 */

typedef struct __sig sig_t;

struct __sig
{
	obj_t    obj;   ///< object header

	unsigned sigset;///< pending signals
};

typedef struct __sig sig_id [];

/******************************************************************************
 *
 * Name              : _SIG_INIT
 *
 * Description       : create and initialize a signal object
 *
 * Parameters        : none
 *
 * Return            : signal object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _SIG_INIT() { _OBJ_INIT(), 0 }

/**
 * @brief   define and initialize a signal object
 *
 * @param   sig        name of a pointer to signal object
 */

#define             OS_SIG( sig ) \
                       sig_t sig[] = { _SIG_INIT() }

/** static alias of OS_SIG() */
#define         static_SIG( sig ) \
                static sig_t sig[] = { _SIG_INIT() }

/**
 * @brief   create and initialize a signal object
 *
 * @return  signal object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                SIG_INIT() \
                      _SIG_INIT()
#endif

/**
 * @brief   create and initialize a signal object
 *
 * @return  signal object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                SIG_CREATE() \
                     { SIG_INIT  () }
/** alias of SIG_CREATE() */
#define                SIG_NEW \
                       SIG_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize a signal object
 *
 * @param   sig        pointer to signal object
 */

void sig_init( sig_t *sig );

/**
 * @brief   reset the signal object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * @param   sig        pointer to signal object
 *
 * @note    use only in thread mode
 */

void sig_reset( sig_t *sig );

/** alias of sig_reset() */
__STATIC_INLINE
void sig_kill( sig_t *sig ) { sig_reset(sig); }

/**
 * @brief   check signal object for a given set of signals
 *          and return the lowest one in the 'signo' variable
 *
 * @param   sig        pointer to signal object
 * @param   sigset     set of expected signals
 * @param   signo      pointer to the variable getting signal number
 *
 * @retval  E_SUCCESS  variable 'singno' contains the lowest number of expected signal from the set of all pending signals
 * @retval  E_TIMEOUT  no expected signal has been set, try again
 */

int sig_take( sig_t *sig, unsigned sigset, unsigned *signo );

/** alias of sig_take() */
__STATIC_INLINE
int sig_tryWait( sig_t *sig, unsigned sigset, unsigned *signo ) { return sig_take(sig, sigset, signo); }

/**
 * @brief   wait for a signal from the given set of signals for given duration of time
 *          and return the lowest one in the 'signo' variable
 *
 * @param   sig        pointer to signal object
 * @param   sigset     set of expected signals
 * @param   signo      pointer to the variable getting signal number
 * @param   delay      duration of time (maximum number of ticks to wait for release the signal object)
 *                     - IMMEDIATE: don't wait until the signal object has been released
 *                     - INFINITE:  wait indefinitely until the signal object has been released
 *
 * @retval  E_SUCCESS  variable 'singno' contains the lowest number of expected signal from the set of all pending signals
 * @retval  E_STOPPED  signal object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  no expected signal has been set before the specified timeout expired
 */

int sig_waitFor( sig_t *sig, unsigned sigset, unsigned *signo, cnt_t delay );

/**
 * @brief   wait for a signal from the given set of signals until given timepoint
 *          and return the lowest one in the 'signo' variable
 *
 * @param   sig        pointer to signal object
 * @param   sigset     set of expected signals
 * @param   signo      pointer to the variable getting signal number
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  variable 'singno' contains the lowest number of expected signal from the set of all pending signals
 * @retval  E_STOPPED  signal object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  no expected signal has been set before the specified timeout expired
 */

int sig_waitUntil( sig_t *sig, unsigned sigset, unsigned *signo, cnt_t time );

/**
 * @brief   wait indefinitely for a signal from the given set of signals
 *          and return the lowest one in the 'signo' variable
 *
 * @param   sig        pointer to signal object
 * @param   sigset     set of expected signals
 * @param   signo      pointer to the variable getting signal number
 *
 * @retval  E_SUCCESS  variable 'singno' contains the lowest number of expected signal from the set of all pending signals
 * @retval  E_STOPPED  signal object was reseted
 */

__STATIC_INLINE
int sig_wait( sig_t *sig, unsigned sigset, unsigned *signo ) { return sig_waitFor(sig, sigset, signo, INFINITE); }

/**
 * @brief   set given signal in the signal object
 *
 * @param   sig        pointer to signal object
 * @param   signo      signal number to set
 *                     signo >= SIG_LIMIT: no signal is set, but the tasks waiting for any signal (sigAny)
 *                     are woken up and get this number in the 'signo' variable
 */

void sig_give( sig_t *sig, unsigned signo );

/** alias of sig_give() */
__STATIC_INLINE
void sig_set( sig_t *sig, unsigned signo ) { sig_give(sig, signo); }

/**
 * @brief   reset given signal in the signal object
 *
 * @param   sig        pointer to signal object
 * @param   signo      signal number to clear
 */

void sig_clear( sig_t *sig, unsigned signo );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus)
namespace intros {

/**
 * @brief   create and initialize a signal object
 */

struct Signal : public __sig
{
	/// constructor, see the constructor parameters of the class
	constexpr
	Signal(): __sig _SIG_INIT() {}

	~Signal() { assert(__sig::obj.queue == nullptr); }

	/// move constructor
	Signal( Signal&& _src ): __sig(_src)
	{
		assert(_src.obj.queue == nullptr);
	}

	Signal( const Signal& ) = delete;
	Signal& operator=( Signal&& ) = delete;
	Signal& operator=( const Signal& ) = delete;

	void reset    ()                                                       {        sig_reset    (this); }                                        ///< sig_reset()
	void kill     ()                                                       {        sig_kill     (this); }                                        ///< sig_kill()
	int  take     ( unsigned _sigset, unsigned *_signo = nullptr )         { return sig_take     (this, _sigset, _signo); }                       ///< sig_take()
	int  tryWait  ( unsigned _sigset, unsigned *_signo = nullptr )         { return sig_tryWait  (this, _sigset, _signo); }                       ///< sig_tryWait()
	template<typename T>
	int  waitFor  ( unsigned _sigset, unsigned *_signo,  const T& _delay ) { return sig_waitFor  (this, _sigset, _signo, Clock::count(_delay)); } ///< sig_waitFor()
	template<typename T>
	int  waitUntil( unsigned _sigset, unsigned *_signo,  const T& _time )  { return sig_waitUntil(this, _sigset, _signo, Clock::until(_time)); }  ///< sig_waitUntil()
	int  wait     ( unsigned _sigset, unsigned *_signo = nullptr )         { return sig_wait     (this, _sigset, _signo); }                       ///< sig_wait()
	void give     ( unsigned _signo )                                      {        sig_give     (this, _signo); }                                ///< sig_give()
	void set      ( unsigned _signo )                                      {        sig_set      (this, _signo); }                                ///< sig_set()
	void clear    ( unsigned _signo )                                      {        sig_clear    (this, _signo); }                                ///< sig_clear()
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

/** @} */

#endif//__INTROS_SIG_H
