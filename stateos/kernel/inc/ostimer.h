/******************************************************************************

    @file    StateOS: ostimer.h
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

#ifndef __STATEOS_TMR_H
#define __STATEOS_TMR_H

#include "oskernel.h"
#include "osclock.h"

/******************************************************************************
 *
 * Name              : timer
 *
 ******************************************************************************/

struct __tmr
{
	obj_t    obj;   // object header
	hdr_t    hdr;   // timer / task header

	fun_t *  proc;  // callback procedure
	void *   arg;   // reserved for internal use
	cnt_t    start;
	cnt_t    delay;
	cnt_t    period;
};

typedef struct __tmr tmr_id [];

/******************************************************************************
 *
 * Name              : _TMR_INIT
 *
 * Description       : create and initialize a timer object
 *
 * Parameters
 *   proc            : callback procedure
 *                     NULL: no callback
 *
 * Return            : timer object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _TMR_INIT( _proc ) \
                    { _OBJ_INIT(), _HDR_INIT(), _proc, NULL, 0, 0, 0 }

/******************************************************************************
 *
 * Name              : OS_TMR
 * Static alias      : static_TMR
 *
 * Description       : define and initialize a timer object
 *
 * Parameters
 *   tmr             : name of a pointer to timer object
 *   proc            : callback procedure
 *                     NULL: no callback
 *
 ******************************************************************************/

#define             OS_TMR( tmr, proc ) \
                       tmr_t tmr[] = { _TMR_INIT( proc ) }

#define         static_TMR( tmr, proc ) \
                static tmr_t tmr[] = { _TMR_INIT( proc ) }

/******************************************************************************
 *
 * Name              : OS_TMR_DEF
 * Static alias      : static_TMR_DEF
 *
 * Description       : define and initialize complete timer object
 *                     timer callback procedure (function body) must be defined immediately below
 *
 * Parameters
 *   tmr             : name of a pointer to timer object
 *
 ******************************************************************************/

#define             OS_TMR_DEF( tmr )          \
                static void tmr##__fun( void ); \
                    OS_TMR( tmr, tmr##__fun );   \
                static void tmr##__fun( void )

#define         static_TMR_DEF( tmr )          \
                static void tmr##__fun( void ); \
                static_TMR( tmr, tmr##__fun );   \
                static void tmr##__fun( void )

/******************************************************************************
 *
 * Name              : OS_TMR_START
 * Static alias      : static_TMR_START
 *
 * Description       : define, initialize and start complete timer object
 *                     timer callback procedure (function body) must be defined immediately below
 *
 * Parameters
 *   tmr             : name of a pointer to timer object
 *   delay           : duration of time (maximum number of ticks to countdown) for first expiration
 *                     IMMEDIATE: don't countdown
 *                     INFINITE:  countdown indefinitely
 *   period          : duration of time (maximum number of ticks to countdown) for all next expirations
 *                     IMMEDIATE: don't countdown
 *                     INFINITE:  countdown indefinitely
 *
 * Note              : only available for compilers supporting the "constructor" function attribute or its equivalent
 *
 ******************************************************************************/

#ifdef __CONSTRUCTOR
#define             OS_TMR_START( tmr, delay, period )                                     \
                static void tmr##__fun( void );                                             \
                    OS_TMR( tmr, tmr##__fun );                                               \
  __CONSTRUCTOR static void tmr##__run( void ) { sys_init(); tmr_start(tmr, delay, period); } \
                static void tmr##__fun( void )
#endif

#ifdef __CONSTRUCTOR
#define         static_TMR_START( tmr, delay, period )                                     \
                static void tmr##__fun( void );                                             \
                static_TMR( tmr, tmr##__fun );                                               \
  __CONSTRUCTOR static void tmr##__run( void ) { sys_init(); tmr_start(tmr, delay, period); } \
                static void tmr##__fun( void )
#endif

/******************************************************************************
 *
 * Name              : OS_TMR_UNTIL
 * Static alias      : static_TMR_UNTIL
 *
 * Description       : define, initialize and start complete timer object
 *                     timer callback procedure (function body) must be defined immediately below
 *
 * Parameters
 *   tmr             : name of a pointer to timer object
 *   time            : timepoint value
 *
 * Note              : only available for compilers supporting the "constructor" function attribute or its equivalent
 *
 ******************************************************************************/

#ifdef __CONSTRUCTOR
#define             OS_TMR_UNTIL( tmr, time )                                          \
                static void tmr##__fun( void );                                         \
                    OS_TMR( tmr, tmr##__fun );                                           \
  __CONSTRUCTOR static void tmr##__run( void ) { sys_init(); tmr_startUntil(tmr, time); } \
                static void tmr##__fun( void )
#endif

#ifdef __CONSTRUCTOR
#define         static_TMR_UNTIL( tmr, time )                                          \
                static void tmr##__fun( void );                                         \
                static_TMR( tmr, tmr##__fun );                                           \
  __CONSTRUCTOR static void tmr##__run( void ) { sys_init(); tmr_startUntil(tmr, time); } \
                static void tmr##__fun( void )
#endif

/******************************************************************************
 *
 * Name              : TMR_INIT
 *
 * Description       : create and initialize a timer object
 *
 * Parameters
 *   proc            : callback procedure
 *                     NULL: no callback
 *
 * Return            : timer object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                TMR_INIT( proc ) \
                      _TMR_INIT( proc )
#endif

/******************************************************************************
 *
 * Name              : TMR_CREATE
 * Alias             : TMR_NEW
 *
 * Description       : create and initialize a timer object
 *
 * Parameters
 *   proc            : callback procedure
 *                     NULL: no callback
 *
 * Return            : timer object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                TMR_CREATE( proc ) \
                     { TMR_INIT  ( proc ) }
#define                TMR_NEW \
                       TMR_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : tmr_thisISR
 *
 * Description       : return current timer object
 *
 * Parameters        : none
 *
 * Return            : current timer object
 *
 * Note              : use only in timer's callback procedure
 *
 ******************************************************************************/

__STATIC_INLINE
tmr_t *tmr_thisISR( void ) { return (tmr_t *) WAIT.hdr.next; }

/******************************************************************************
 *
 * Name              : tmr_init
 *
 * Description       : initialize a timer object
 *
 * Parameters
 *   tmr             : pointer to timer object
 *   proc            : callback procedure
 *                     NULL: no callback
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void tmr_init( tmr_t *tmr, fun_t *proc );

/******************************************************************************
 *
 * Name              : tmr_create
 * Alias             : tmr_new
 *
 * Description       : create and initialize a new timer object
 *
 * Parameters
 *   proc            : callback procedure
 *                     NULL: no callback
 *
 * Return            : pointer to timer object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

tmr_t *tmr_create( fun_t *proc );

__STATIC_INLINE
tmr_t *tmr_new( fun_t *proc ) { return tmr_create(proc); }

/******************************************************************************
 *
 * Name              : tmr_setup
 *
 * Description       : create and initialize a new timer object
 *
 * Parameters
 *   proc            : timer callback procedure
 *   arg             : timer callback procedure argument (for internal use)
 *
 * Return            : pointer to timer object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : for internal use
 *
 ******************************************************************************/

tmr_t *tmr_setup( fun_t *proc, void *arg );

/******************************************************************************
 *
 * Name              : tmr_reset
 * Alias             : tmr_kill
 *
 * Description       : reset the timer object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   tmr             : pointer to timer object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void tmr_reset( tmr_t *tmr );

__STATIC_INLINE
void tmr_kill( tmr_t *tmr ) { tmr_reset(tmr); }

/******************************************************************************
 *
 * Name              : tmr_destroy
 * Alias             : tmr_delete
 *
 * Description       : reset the timer object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * Parameters
 *   tmr             : pointer to timer object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void tmr_destroy( tmr_t *tmr );

__STATIC_INLINE
void tmr_delete( tmr_t *tmr ) { tmr_destroy(tmr); }

/******************************************************************************
 *
 * Name              : tmr_start
 *
 * Description       : start/restart periodic timer for given duration of time
 *                     when the timer has finished the countdown, the callback procedure is launched
 *                     do this periodically if period > 0
 *
 * Parameters
 *   tmr             : pointer to timer object
 *   delay           : duration of time (maximum number of ticks to countdown) for first expiration
 *                     IMMEDIATE: don't countdown
 *                     INFINITE:  countdown indefinitely
 *   period          : duration of time (maximum number of ticks to countdown) for all next expirations
 *                     IMMEDIATE: don't countdown
 *                     INFINITE:  countdown indefinitely
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void tmr_start( tmr_t *tmr, cnt_t delay, cnt_t period );

/******************************************************************************
 *
 * Name              : tmr_startFor
 *
 * Description       : start/restart one-shot timer for given duration of time
 *                     when the timer has finished the countdown, the callback procedure is launched
 *
 * Parameters
 *   tmr             : pointer to timer object
 *   delay           : duration of time (maximum number of ticks to countdown)
 *                     IMMEDIATE: don't countdown
 *                     INFINITE:  countdown indefinitely
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
void tmr_startFor( tmr_t *tmr, cnt_t delay ) { tmr_start(tmr, delay, 0); }

/******************************************************************************
 *
 * Name              : tmr_startPeriodic
 *
 * Description       : start/restart periodic timer for given duration of time
 *                     when the timer has finished the countdown, the callback procedure is launched
 *                     do this periodically
 *
 * Parameters
 *   tmr             : pointer to timer object
 *   period          : duration of time (maximum number of ticks to countdown)
 *                     IMMEDIATE: don't countdown
 *                     INFINITE:  countdown indefinitely
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
void tmr_startPeriodic( tmr_t *tmr, cnt_t period ) { tmr_start(tmr, period, period); }

/******************************************************************************
 *
 * Name              : tmr_startFrom
 *
 * Description       : start/restart periodic timer for given duration of time
 *                     when the timer has finished the countdown, the callback procedure is launched
 *                     do this periodically if period > 0
 *
 * Parameters
 *   tmr             : pointer to timer object
 *   delay           : duration of time (maximum number of ticks to countdown) for first expiration
 *                     IMMEDIATE: don't countdown
 *                     INFINITE:  countdown indefinitely
 *   period          : duration of time (maximum number of ticks to countdown) for all next expirations
 *                     IMMEDIATE: don't countdown
 *                     INFINITE:  countdown indefinitely
 *   proc            : callback procedure
 *                     NULL: no callback
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void tmr_startFrom( tmr_t *tmr, cnt_t delay, cnt_t period, fun_t *proc );

/******************************************************************************
 *
 * Name              : tmr_startNext
 *
 * Description       : restart the timer for given duration of time from the end of the previous countdown
 *                     when the timer has finished the countdown, the callback procedure is launched
 *
 * Parameters
 *   tmr             : pointer to timer object
 *   delay           : duration of time (maximum number of ticks to countdown)
 *                     IMMEDIATE: don't countdown
 *                     INFINITE:  countdown indefinitely
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void tmr_startNext( tmr_t *tmr, cnt_t delay );

/******************************************************************************
 *
 * Name              : tmr_startUntil
 *
 * Description       : start/restart one-shot timer until given timepoint
 *                     when the timer has finished the countdown, the callback procedure is launched
 *
 * Parameters
 *   tmr             : pointer to timer object
 *   time            : timepoint value
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void tmr_startUntil( tmr_t *tmr, cnt_t time );

/******************************************************************************
 *
 * Name              : tmr_stop
 *
 * Description       : stop timer
 *
 * Parameters
 *   tmr             : pointer to timer object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
void tmr_stop( tmr_t *tmr ) { tmr_start(tmr, 0, 0); }

/******************************************************************************
 *
 * Name              : tmr_take
 * Alias             : tmr_tryWait
 * ISR alias         : tmr_takeISR
 *
 * Description       : check if the timer finishes countdown
 *
 * Parameters
 *   tmr             : pointer to timer object
 *
 * Return
 *   E_SUCCESS       : timer object successfully finished countdown
 *   E_FAILURE       : timer has not yet been started
 *   E_TIMEOUT       : timer object has not yet completed counting, try again
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

int tmr_take( tmr_t *tmr );

__STATIC_INLINE
int tmr_tryWait( tmr_t *tmr ) { return tmr_take(tmr); }

__STATIC_INLINE
int tmr_takeISR( tmr_t *tmr ) { return tmr_take(tmr); }

/******************************************************************************
 *
 * Name              : tmr_waitFor
 *
 * Description       : wait for given duration of time until the timer finishes countdown
 *
 * Parameters
 *   tmr             : pointer to timer object
 *   delay           : duration of time (maximum number of ticks to wait for the timer finishes countdown)
 *                     IMMEDIATE: don't wait for the timer finishes countdown
 *                     INFINITE:  wait indefinitely until the timer finishes countdown
 *
 * Return
 *   E_SUCCESS       : timer object successfully finished countdown
 *   E_FAILURE       : timer has not yet been started
 *   E_STOPPED       : timer object was reseted before the specified timeout expired
 *   E_DELETED       : timer object was deleted before the specified timeout expired
 *   E_TIMEOUT       : timer object has not finished countdown before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int tmr_waitFor( tmr_t *tmr, cnt_t delay );

/******************************************************************************
 *
 * Name              : tmr_waitNext
 *
 * Description       : wait for given duration of time from the end of the previous countdown
 *                     until the timer finishes countdown
 *
 * Parameters
 *   tmr             : pointer to timer object
 *   delay           : duration of time (maximum number of ticks to wait for the timer finishes countdown)
 *                     IMMEDIATE: don't wait for the timer finishes countdown
 *                     INFINITE:  wait indefinitely until the timer finishes countdown
 *
 * Return
 *   E_SUCCESS       : timer object successfully finished countdown
 *   E_FAILURE       : timer has not yet been started
 *   E_STOPPED       : timer object was reseted before the specified timeout expired
 *   E_DELETED       : timer object was deleted before the specified timeout expired
 *   E_TIMEOUT       : timer object has not finished countdown before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int tmr_waitNext( tmr_t *tmr, cnt_t delay );

/******************************************************************************
 *
 * Name              : tmr_waitUntil
 *
 * Description       : wait until given timepoint until the timer finishes countdown
 *
 * Parameters
 *   tmr             : pointer to timer object
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : timer object successfully finished countdown
 *   E_FAILURE       : timer has not yet been started
 *   E_STOPPED       : timer object was reseted before the specified timeout expired
 *   E_DELETED       : timer object was deleted before the specified timeout expired
 *   E_TIMEOUT       : timer object has not finished countdown before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int tmr_waitUntil( tmr_t *tmr, cnt_t time );

/******************************************************************************
 *
 * Name              : tmr_wait
 *
 * Description       : wait indefinitely until the timer finishes countdown
 *
 * Parameters
 *   tmr             : pointer to timer object
 *
 * Return
 *   E_SUCCESS       : timer object successfully finished countdown
 *   E_FAILURE       : timer has not yet been started
 *   E_STOPPED       : timer object was reseted before the countdown ended
 *   E_DELETED       : timer object was deleted before the countdown ended
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
int tmr_wait( tmr_t *tmr ) { return tmr_waitFor(tmr, INFINITE); }

/******************************************************************************
 *
 * Name              : tmr_flipISR
 *
 * Description       : change callback procedure for current periodic timer (available in next period)
 *
 * Parameters
 *   proc            : new callback procedure
 *                     NULL: no callback
 *
 * Return            : none
 *
 * Note              : use only in timer's callback procedure
 *
 ******************************************************************************/

__STATIC_INLINE
void tmr_flipISR( fun_t *proc ) { tmr_thisISR()->proc = proc; }

/******************************************************************************
 *
 * Name              : tmr_delayISR
 *
 * Description       : change delay time for current timer
 *
 * Parameters
 *   delay           : duration of time (maximum number of ticks to countdown)
 *                     0:         current timer is stopped even if it was periodic
 *                     otherwise: current timer is restarted even if it was one-shot
 *
 * Return            : none
 *
 * Note              : use only in timer's callback procedure
 *
 ******************************************************************************/

__STATIC_INLINE
void tmr_delayISR( cnt_t delay ) { tmr_thisISR()->delay = delay; }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace stateos {

/******************************************************************************
 *
 * Class             : baseTimer
 *
 * Description       : create and initialize base class for timer objects
 *
 * Constructor parameters
 *   proc            : callback procedure
 *                     nullptr: no callback
 *
 * Note              : for internal use
 *
 ******************************************************************************/

struct baseTimer : public __tmr
{
	baseTimer(): __tmr _TMR_INIT(nullptr) {}
#if __cplusplus >= 201402L
	template<class F>
	baseTimer( F&& _proc ): __tmr _TMR_INIT(fun_), fun{_proc} {}
#else
	baseTimer( fun_t *_proc ): __tmr _TMR_INIT(_proc) {}
#endif

	void reset        ()                                                    {        tmr_reset        (this); }
	void kill         ()                                                    {        tmr_kill         (this); }
	void destroy      ()                                                    {        tmr_destroy      (this); }
	template<typename T>
	void start        ( const T& _delay, const T& _period )                 {        tmr_start        (this, Clock::count(_delay), Clock::count(_period)); }
	template<typename T>
	void startFor     ( const T& _delay )                                   {        tmr_startFor     (this, Clock::count(_delay)); }
	template<typename T>
	void startPeriodic( const T& _period )                                  {        tmr_startPeriodic(this, Clock::count(_period)); }
	template<typename T>
	void startNext    ( const T& _delay )                                   {        tmr_startNext    (this, Clock::count(_delay)); }
	template<typename T>
	void startUntil   ( const T& _time )                                    {        tmr_startUntil   (this, Clock::until(_time)); }
#if __cplusplus >= 201402L
	template<typename T>
	void startFrom    ( const T& _delay, const T& _period, std::nullptr_t ) {        tmr_startFrom    (this, Clock::count(_delay), Clock::count(_period), nullptr); }
	template<typename T, class F>
	void startFrom    ( const T& _delay, const T& _period, F&&     _proc )  {        new (&fun) Fun_t(_proc);
	                                                                                 tmr_startFrom    (this, Clock::count(_delay), Clock::count(_period), fun_); }
#else
	template<typename T>
	void startFrom    ( const T& _delay, const T& _period, fun_t * _proc )  {        tmr_startFrom    (this, Clock::count(_delay), Clock::count(_period), _proc); }
#endif
	void stop         ()                                                    {        tmr_stop         (this); }
	int  take         ()                                                    { return tmr_take         (this); }
	int  tryWait      ()                                                    { return tmr_tryWait      (this); }
	int  takeISR      ()                                                    { return tmr_takeISR      (this); }
	template<typename T>
	int  waitFor      ( const T& _delay )                                   { return tmr_waitFor      (this, Clock::count(_delay)); }
	template<typename T>
	int  waitNext     ( const T& _delay )                                   { return tmr_waitNext     (this, Clock::count(_delay)); }
	template<typename T>
	int  waitUntil    ( const T& _time )                                    { return tmr_waitUntil    (this, Clock::until(_time)); }
	int  wait         ()                                                    { return tmr_wait         (this); }
	explicit
	operator bool     () const                                              { return __tmr::hdr.id != ID_STOPPED; }

	template<class T = baseTimer> static
	T *  current      ()                                                    { return static_cast<T *>(tmr_thisISR()); }

#if __cplusplus >= 201402L
	static
	void fun_         ()                                                    {        current()->fun(); }
	Fun_t fun;
#endif

/******************************************************************************
 *
 * Class             : [base]Timer::Current
 *
 * Description       : provide set of functions for current timer
 *
 ******************************************************************************/

	struct Current
	{
#if __cplusplus >= 201402L
		static
		void flipISR ( std::nullptr_t )            { tmr_flipISR (nullptr); }
		template<class F> static
		void flipISR ( F&& _proc )                 { new (&current()->fun) Fun_t(_proc);
		                                             tmr_flipISR (fun_); }
		template<typename F, typename... A> static
		void flipISR ( F&& _proc, A&&... _args )   { flipISR(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...)); }
#else
		static
		void flipISR ( fun_t  * _proc )            { tmr_flipISR (_proc); }
#endif
		template<typename T> static
		void delayISR( const T& _delay )           { tmr_delayISR(Clock::count(_delay)); }
	};
};

using thisTimer = baseTimer::Current;
using this_timer = baseTimer::Current;

/******************************************************************************
 *
 * Class             : Timer
 *
 * Description       : create and initialize a timer object
 *
 * Constructor parameters
 *   proc            : callback procedure
 *                     none / nullptr: no callback
 *
 ******************************************************************************/

struct Timer : public baseTimer
{
	Timer(): baseTimer{} {}
	template<class F>
	Timer( F&& _proc ): baseTimer{_proc} {}
#if __cplusplus >= 201402L
	Timer( std::nullptr_t ): baseTimer{} {}
	template<typename F, typename... A>
	Timer( F&& _proc, A&&... _args ): baseTimer{std::bind(std::forward<F>(_proc), std::forward<A>(_args)...)} {}
#endif

	~Timer() { assert(__tmr::hdr.id == ID_STOPPED); }

	Timer( Timer&& ) = default;
	Timer( const Timer& ) = delete;
	Timer& operator=( Timer&& ) = delete;
	Timer& operator=( const Timer& ) = delete;

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<Timer>;
#else
	using Ptr = Timer *;
#endif

/******************************************************************************
 *
 * Name              : Timer::Make
 *
 * Description       : create and initialize static timer object
 *
 * Parameters
 *   proc            : callback procedure
 *                     none / nullptr: no callback
 *   args            : arguments for callback procedure
 *
 * Return            : Timer object
 *
 ******************************************************************************/

	static
	Timer Make()
	{
		return {};
	}

	template<class F> static
	Timer Make( F&& _proc )
	{
		return { _proc };
	}

#if __cplusplus >= 201402L
	static
	Timer Make( std::nullptr_t )
	{
		return Make();
	}

	template<typename F, typename... A> static
	Timer Make( F&& _proc, A&&... _args )
	{
		return Make(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif

/******************************************************************************
 *
 * Name              : Timer::Start
 *
 * Description       : create and initialize static timer object
 *                     and start periodic timer for given duration of time
 *                     when the timer has finished the countdown, the callback procedure is launched
 *                     do this periodically
 *
 * Parameters
 *   delay           : duration of time (maximum number of ticks to countdown) for first expiration
 *                     IMMEDIATE: don't countdown
 *                     INFINITE:  countdown indefinitely
 *   period          : duration of time (maximum number of ticks to countdown) for all next expirations
 *                     IMMEDIATE: don't countdown
 *                     INFINITE:  countdown indefinitely
 *   proc            : callback procedure
 *                     none / nullptr: no callback
 *   args            : arguments for callback procedure
 *
 * Return            : Timer object
 *
 ******************************************************************************/

	template<typename T> static
	Timer Start( const T& _delay, const T& _period )
	{
		Timer tmr {};
		tmr.start(_delay, _period);
		return tmr;
	}

	template<typename T, class F> static
	Timer Start( const T& _delay, const T& _period, F&& _proc )
	{
		Timer tmr { _proc };
		tmr.start(_delay, _period);
		return tmr;
	}

#if __cplusplus >= 201402L
	template<typename T> static
	Timer Start( const T& _delay, const T& _period, std::nullptr_t )
	{
		return Start(_delay, _period);
	}

	template<typename T, typename F, typename... A> static
	Timer Start( const T& _delay, const T& _period, F&& _proc, A&&... _args )
	{
		return Start(_delay, _period, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif

/******************************************************************************
 *
 * Name              : Timer::StartFor
 *
 * Description       : create and initialize static timer object
 *                     and start one-shot timer for given duration of time
 *                     when the timer has finished the countdown, the callback procedure is launched
 *
 * Parameters
 *   delay           : duration of time (maximum number of ticks to countdown)
 *                     IMMEDIATE: don't countdown
 *                     INFINITE:  countdown indefinitely
 *   proc            : callback procedure
 *                     none / nullptr: no callback
 *   args            : arguments for callback procedure
 *
 * Return            : Timer object
 *
 ******************************************************************************/

	template<typename T> static
	Timer StartFor( const T& _delay )
	{
		Timer tmr {};
		tmr.startFor(_delay);
		return tmr;
	}

	template<typename T, class F> static
	Timer StartFor( const T& _delay, F&& _proc )
	{
		Timer tmr { _proc };
		tmr.startFor(_delay);
		return tmr;
	}

#if __cplusplus >= 201402L
	template<typename T> static
	Timer StartFor( const T& _delay, std::nullptr_t )
	{
		return StartFor(_delay);
	}

	template<typename T, typename F, typename... A> static
	Timer StartFor( const T& _delay, F&& _proc, A&&... _args )
	{
		return StartFor(_delay, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif

/******************************************************************************
 *
 * Name              : Timer::StartPeriodic
 *
 * Description       : create and initialize static timer object
 *                     and start periodic timer for given duration of time
 *                     when the timer has finished the countdown, the callback procedure is launched
 *                     do this periodically
 *
 * Parameters
 *   period          : duration of time (maximum number of ticks to countdown)
 *                     IMMEDIATE: don't countdown
 *                     INFINITE:  countdown indefinitely
 *   proc            : callback procedure
 *                     none / nullptr: no callback
 *   args            : arguments for callback procedure
 *
 * Return            : Timer object
 *
 ******************************************************************************/

	template<typename T> static
	Timer StartPeriodic( const T& _period )
	{
		Timer tmr {};
		tmr.startPeriodic(_period);
		return tmr;
	}

	template<typename T, class F> static
	Timer StartPeriodic( const T& _period, F&& _proc )
	{
		Timer tmr { _proc };
		tmr.startPeriodic(_period);
		return tmr;
	}

#if __cplusplus >= 201402L
	template<typename T> static
	Timer StartPeriodic( const T& _period, std::nullptr_t )
	{
		return StartPeriodic(_period);
	}

	template<typename T, typename F, typename... A> static
	Timer StartPeriodic( const T& _period, F&& _proc, A&&... _args )
	{
		return StartPeriodic(_period, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif

/******************************************************************************
 *
 * Name              : Timer::StartUntil
 *
 * Description       : create and initialize static timer object
 *                     and start one-shot timer until given timepoint
 *                     when the timer has finished the countdown, the callback procedure is launched
 *
 * Parameters
 *   time            : timepoint value
 *   proc            : callback procedure
 *                     none / nullptr: no callback
 *   args            : arguments for callback procedure
 *
 * Return            : Timer object
 *
 ******************************************************************************/

	template<typename T> static
	Timer StartUntil( const T& _time )
	{
		Timer tmr {};
		tmr.startUntil(_time);
		return tmr;
	}

	template<typename T, class F> static
	Timer StartUntil( const T& _time, F&& _proc )
	{
		Timer tmr { _proc };
		tmr.startUntil(_time);
		return tmr;
	}

#if __cplusplus >= 201402L
	template<typename T> static
	Timer StartUntil( const T& _time, std::nullptr_t )
	{
		return StartUntil(_time);
	}

	template<typename T, typename F, typename... A> static
	Timer StartUntil( const T& _time, F&& _proc, A&&... _args )
	{
		return StartUntil(_time, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif

/******************************************************************************
 *
 * Name              : Timer::Create
 *
 * Description       : create and initialize dynamic timer with manageable resources
 *
 * Parameters
 *   proc            : callback procedure
 *                     none / nullptr: no callback
 *   args            : arguments for callback procedure
 *
 * Return            : std::unique_pointer / pointer to Timer object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create()
	{
		auto tmr = new (std::nothrow) Timer();
		if (tmr != nullptr)
			tmr->__tmr::obj.res = tmr;
		return Ptr(tmr);
	}

	template<class F> static
	Ptr Create( F&& _proc )
	{
		auto tmr = new (std::nothrow) Timer(_proc);
		if (tmr != nullptr)
			tmr->__tmr::obj.res = tmr;
		return Ptr(tmr);
	}

#if __cplusplus >= 201402L
	static
	Ptr Create( std::nullptr_t )
	{
		return Create();
	}

	template<typename F, typename... A> static
	Ptr Create( F&& _proc, A&&... _args )
	{
		return Create(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__STATEOS_TMR_H
