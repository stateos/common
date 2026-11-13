/******************************************************************************

    @file    IntrOS: ostimer.h
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

#ifndef __INTROS_TMR_H
#define __INTROS_TMR_H

#include "oskernel.h"
#include "osclock.h"

/**
 * @defgroup timer Timer
 * @brief    timers (one-shot, periodic)
 * @{
 */

/**
 * @brief   timer
 */

struct __tmr
{
	obj_t    obj;   ///< object header
	hdr_t    hdr;   ///< timer / task header

	fun_t *  proc;  ///< callback procedure
	void *   arg;   ///< reserved for internal use
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

/**
 * @brief   define and initialize a timer object
 *
 * @param   tmr        name of a pointer to timer object
 * @param   proc       callback procedure
 *                     - NULL: no callback
 */

#define             OS_TMR( tmr, proc ) \
                       tmr_t tmr[] = { _TMR_INIT( proc ) }

/** static alias of OS_TMR() */
#define         static_TMR( tmr, proc ) \
                static tmr_t tmr[] = { _TMR_INIT( proc ) }

/**
 * @brief   define and initialize complete timer object
 *
 *          timer callback procedure (function body) must be defined immediately below
 *
 * @param   tmr        name of a pointer to timer object
 */

#define             OS_TMR_DEF( tmr )          \
                static void tmr##__fun( void ); \
                    OS_TMR( tmr, tmr##__fun );   \
                static void tmr##__fun( void )

/** static alias of OS_TMR_DEF() */
#define         static_TMR_DEF( tmr )          \
                static void tmr##__fun( void ); \
                static_TMR( tmr, tmr##__fun );   \
                static void tmr##__fun( void )

/**
 * @brief   define, initialize and start complete timer object
 *
 *          timer callback procedure (function body) must be defined immediately below
 *
 * @param   tmr        name of a pointer to timer object
 * @param   delay      duration of time (maximum number of ticks to countdown) for first expiration
 *                     - IMMEDIATE: don't countdown
 *                     - INFINITE:  countdown indefinitely
 * @param   period     duration of time (maximum number of ticks to countdown) for all next expirations
 *                     - IMMEDIATE: don't countdown
 *                     - INFINITE:  countdown indefinitely
 *
 * @note    only available for compilers supporting the "constructor" function attribute or its equivalent
 */

#ifdef __CONSTRUCTOR
#define             OS_TMR_START( tmr, delay, period )                         \
                static void tmr##__fun( void );                                 \
                    OS_TMR( tmr, tmr##__fun );                                   \
  __CONSTRUCTOR static void tmr##__run( void ) { tmr_start(tmr, delay, period); } \
                static void tmr##__fun( void )
#endif

#ifdef __CONSTRUCTOR
/** static alias of OS_TMR_START() */
#define         static_TMR_START( tmr, delay, period )                         \
                static void tmr##__fun( void );                                 \
                static_TMR( tmr, tmr##__fun );                                   \
  __CONSTRUCTOR static void tmr##__run( void ) { tmr_start(tmr, delay, period); } \
                static void tmr##__fun( void )
#endif

/**
 * @brief   define, initialize and start complete timer object
 *
 *          timer callback procedure (function body) must be defined immediately below
 *
 * @param   tmr        name of a pointer to timer object
 * @param   time       timepoint value
 *
 * @note    only available for compilers supporting the "constructor" function attribute or its equivalent
 */

#ifdef __CONSTRUCTOR
#define             OS_TMR_UNTIL( tmr, time )                              \
                static void tmr##__fun( void );                             \
                    OS_TMR( tmr, tmr##__fun );                               \
  __CONSTRUCTOR static void tmr##__run( void ) { tmr_startUntil(tmr, time); } \
                static void tmr##__fun( void )
#endif

#ifdef __CONSTRUCTOR
/** static alias of OS_TMR_UNTIL() */
#define         static_TMR_UNTIL( tmr, time )                              \
                static void tmr##__fun( void );                             \
                static_TMR( tmr, tmr##__fun );                               \
  __CONSTRUCTOR static void tmr##__run( void ) { tmr_startUntil(tmr, time); } \
                static void tmr##__fun( void )
#endif

/**
 * @brief   create and initialize a timer object
 *
 * @param   proc       callback procedure
 *                     - NULL: no callback
 *
 * @return  timer object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                TMR_INIT( proc ) \
                      _TMR_INIT( proc )
#endif

/**
 * @brief   create and initialize a timer object
 *
 * @param   proc       callback procedure
 *                     - NULL: no callback
 *
 * @return  timer object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                TMR_CREATE( proc ) \
                     { TMR_INIT  ( proc ) }
/** alias of TMR_CREATE() */
#define                TMR_NEW \
                       TMR_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   return current timer object
 *
 * @return  current timer object
 *
 * @note    use only in timer's callback procedure
 */

__STATIC_INLINE
tmr_t *tmr_this( void ) { return (tmr_t *) System.cur; }

/**
 * @brief   initialize a timer object
 *
 * @param   tmr        pointer to timer object
 * @param   proc       callback procedure
 *                     - NULL: no callback
 */

void tmr_init( tmr_t *tmr, fun_t *proc );

/**
 * @brief   reset the timer object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * @param   tmr        pointer to timer object
 *
 * @note    use only in thread mode
 */

void tmr_reset( tmr_t *tmr );

/** alias of tmr_reset() */
__STATIC_INLINE
void tmr_kill( tmr_t *tmr ) { tmr_reset(tmr); }

/**
 * @brief   start/restart periodic timer for given duration of time
 *
 *          when the timer has finished the countdown, the callback procedure is launched
 *
 *          do this periodically if period > 0
 *
 * @param   tmr        pointer to timer object
 * @param   delay      duration of time (maximum number of ticks to countdown) for first expiration
 *                     - IMMEDIATE: don't countdown
 *                     - INFINITE:  countdown indefinitely
 * @param   period     duration of time (maximum number of ticks to countdown) for all next expirations
 *                     - IMMEDIATE: don't countdown
 *                     - INFINITE:  countdown indefinitely
 */

void tmr_start( tmr_t *tmr, cnt_t delay, cnt_t period );

/**
 * @brief   start/restart one-shot timer for given duration of time
 *
 *          when the timer has finished the countdown, the callback procedure is launched
 *
 * @param   tmr        pointer to timer object
 * @param   delay      duration of time (maximum number of ticks to countdown)
 *                     - IMMEDIATE: don't countdown
 *                     - INFINITE:  countdown indefinitely
 */

__STATIC_INLINE
void tmr_startFor( tmr_t *tmr, cnt_t delay ) { tmr_start(tmr, delay, 0); }

/**
 * @brief   start/restart periodic timer for given duration of time
 *
 *          when the timer has finished the countdown, the callback procedure is launched
 *
 *          do this periodically
 *
 * @param   tmr        pointer to timer object
 * @param   period     duration of time (maximum number of ticks to countdown)
 *                     - IMMEDIATE: don't countdown
 *                     - INFINITE:  countdown indefinitely
 */

__STATIC_INLINE
void tmr_startPeriodic( tmr_t *tmr, cnt_t period ) { tmr_start(tmr, period, period); }

/**
 * @brief   start/restart periodic timer for given duration of time
 *
 *          when the timer has finished the countdown, the callback procedure is launched
 *
 *          do this periodically if period > 0
 *
 * @param   tmr        pointer to timer object
 * @param   delay      duration of time (maximum number of ticks to countdown) for first expiration
 *                     - IMMEDIATE: don't countdown
 *                     - INFINITE:  countdown indefinitely
 * @param   period     duration of time (maximum number of ticks to countdown) for all next expirations
 *                     - IMMEDIATE: don't countdown
 *                     - INFINITE:  countdown indefinitely
 * @param   proc       callback procedure
 *                     - NULL: no callback
 */

void tmr_startFrom( tmr_t *tmr, cnt_t delay, cnt_t period, fun_t *proc );

/**
 * @brief   restart the timer for given duration of time from the end of the previous countdown
 *
 *          when the timer has finished the countdown, the callback procedure is launched
 *
 * @param   tmr        pointer to timer object
 * @param   delay      duration of time (maximum number of ticks to countdown)
 *                     - IMMEDIATE: don't countdown
 *                     - INFINITE:  countdown indefinitely
 */

void tmr_startNext( tmr_t *tmr, cnt_t delay );

/**
 * @brief   start/restart one-shot timer until given timepoint
 *
 *          when the timer has finished the countdown, the callback procedure is launched
 *
 * @param   tmr        pointer to timer object
 * @param   time       timepoint value
 */

void tmr_startUntil( tmr_t *tmr, cnt_t time );

/**
 * @brief   stop timer
 *
 * @param   tmr        pointer to timer object
 */

__STATIC_INLINE
void tmr_stop( tmr_t *tmr ) { tmr_reset(tmr); }

/**
 * @brief   check if the timer finishes countdown
 *
 * @param   tmr        pointer to timer object
 *
 * @retval  E_SUCCESS  timer object successfully finished countdown
 * @retval  E_FAILURE  timer has not yet been started
 * @retval  E_TIMEOUT  timer object has not yet completed counting, try again
 */

int tmr_take( tmr_t *tmr );

/** alias of tmr_take() */
__STATIC_INLINE
int tmr_expired( tmr_t *tmr ) { return tmr_take(tmr); }

/** alias of tmr_take() */
__STATIC_INLINE
int tmr_tryWait( tmr_t *tmr ) { return tmr_take(tmr); }

/**
 * @brief   wait for given duration of time until the timer finishes countdown
 *
 * @param   tmr        pointer to timer object
 * @param   delay      duration of time (maximum number of ticks to wait for the timer finishes countdown)
 *                     - IMMEDIATE: don't wait for the timer finishes countdown
 *                     - INFINITE:  wait indefinitely until the timer finishes countdown
 *
 * @retval  E_SUCCESS  timer object successfully finished countdown
 * @retval  E_FAILURE  timer has not yet been started
 * @retval  E_STOPPED  timer object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  timer object has not finished countdown before the specified timeout expired
 */

int tmr_waitFor( tmr_t *tmr, cnt_t delay );

/**
 * @brief   wait for given duration of time from the end of the previous countdown
 *
 *          until the timer finishes countdown
 *
 * @param   tmr        pointer to timer object
 * @param   delay      duration of time (maximum number of ticks to wait for the timer finishes countdown)
 *                     - IMMEDIATE: don't wait for the timer finishes countdown
 *                     - INFINITE:  wait indefinitely until the timer finishes countdown
 *
 * @retval  E_SUCCESS  timer object successfully finished countdown
 * @retval  E_FAILURE  timer has not yet been started
 * @retval  E_STOPPED  timer object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  timer object has not finished countdown before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int tmr_waitNext( tmr_t *tmr, cnt_t delay );

/**
 * @brief   wait until given timepoint until the timer finishes countdown
 *
 * @param   tmr        pointer to timer object
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  timer object successfully finished countdown
 * @retval  E_FAILURE  timer has not yet been started
 * @retval  E_STOPPED  timer object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  timer object has not finished countdown before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int tmr_waitUntil( tmr_t *tmr, cnt_t time );

/**
 * @brief   wait indefinitely until the timer finishes countdown
 *
 * @param   tmr        pointer to timer object
 *
 * @retval  E_SUCCESS  timer object successfully finished countdown
 * @retval  E_FAILURE  timer has not yet been started
 * @retval  E_STOPPED  timer object was reseted before the countdown ended
 */

__STATIC_INLINE
int tmr_wait( tmr_t *tmr ) { return tmr_waitFor(tmr, INFINITE); }

/**
 * @brief   change callback procedure for current periodic timer (available in next period)
 *
 * @param   proc       new callback procedure
 *                     - NULL: no callback
 *
 * @note    use only in timer's callback procedure
 */

__STATIC_INLINE
void tmr_flip( fun_t *proc ) { tmr_this()->proc = proc; }

/**
 * @brief   change delay time for current timer
 *
 * @param   delay      duration of time (maximum number of ticks to countdown)
 *                     - 0:         current timer is stopped even if it was periodic
 *                     - otherwise: current timer is restarted even if it was one-shot
 *
 * @note    use only in timer's callback procedure
 */

__STATIC_INLINE
void tmr_delay( cnt_t delay ) { tmr_this()->delay = delay; }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus)
namespace intros {

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
	void startFrom    ( const T& _delay, const T& _period, F&&     _proc )  {        fun = Fun_t(_proc);
	                                                                                 tmr_startFrom    (this, Clock::count(_delay), Clock::count(_period), fun_); }
#else
	template<typename T>
	void startFrom    ( const T& _delay, const T& _period, fun_t * _proc )  {        tmr_startFrom    (this, Clock::count(_delay), Clock::count(_period), _proc); }
#endif
	void stop         ()                                                    {        tmr_stop         (this); }
	int  take         ()                                                    { return tmr_take         (this); }
	int  expired      ()                                                    { return tmr_expired      (this); }
	int  tryWait      ()                                                    { return tmr_tryWait      (this); }
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
	T *  current      ()                                                    { return static_cast<T *>(tmr_this()); }

#if __cplusplus >= 201402L
	static
	void fun_         ()
	{
		auto  tmr = current();
		Fun_t run;
		run.swap(tmr->fun);       // the running function must live until the callback returns,
		run();                    // the callback can replace it (flip, startFrom)
		if (!tmr->fun)
			tmr->fun.swap(run);
	}
	Fun_t fun;
#endif

/**
 * @brief   provide set of functions for current timer
 */

	struct Current
	{
#if __cplusplus >= 201402L
		static
		void flip ( std::nullptr_t )            { tmr_flip (nullptr); }
		template<class F> static
		void flip ( F&& _proc )                 { current()->fun = Fun_t(_proc);
		                                          tmr_flip (fun_); }
		template<typename F, typename... A> static
		void flip ( F&& _proc, A&&... _args )   { flip(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...)); }
#else
		static
		void flip ( fun_t  * _proc )            { tmr_flip (_proc); }
#endif
		template<typename T> static
		void delay( const T& _delay )           { tmr_delay(Clock::count(_delay)); }
	};
};

using thisTimer = baseTimer::Current;
using this_timer = baseTimer::Current;

/**
 * @brief   create and initialize a timer object
 *
 * @par     Constructor parameters
 *          - proc: callback procedure
 *            - none / nullptr: no callback
 */

struct Timer : public baseTimer
{
	/// constructor, see the constructor parameters of the class
	Timer(): baseTimer{} {}
	/// constructor, see the constructor parameters of the class
	template<class F>
	Timer( F&& _proc ): baseTimer{_proc} {}
#if __cplusplus >= 201402L
	/// constructor, see the constructor parameters of the class
	Timer( std::nullptr_t ): baseTimer{} {}
	/// constructor, see the constructor parameters of the class
	template<typename F, typename... A>
	Timer( F&& _proc, A&&... _args ): baseTimer{std::bind(std::forward<F>(_proc), std::forward<A>(_args)...)} {}
#endif

	~Timer() { assert(__tmr::hdr.id == ID_STOPPED); }

	/// move constructor
	Timer( Timer&& _src ): baseTimer(static_cast<baseTimer&&>(_src))
	{
		assert(_src.hdr.id == ID_STOPPED && _src.obj.queue == nullptr);
	}

	Timer( const Timer& ) = delete;
	Timer& operator=( Timer&& ) = delete;
	Timer& operator=( const Timer& ) = delete;

private: // the Start functions return the object started in place: a started object must not be moved
	template<class S, typename... A>
	Timer( Held_t, S&& _start, A&&... _args ): Timer{static_cast<A&&>(_args)...} { _start(*this); }
public:

/**
 * @brief   create and initialize static timer object
 *
 * @par     Parameters
 *          - proc: callback procedure
 *            - none / nullptr: no callback
 *          - args: arguments for callback procedure
 *
 * @return  Timer object
 */

	static
	Timer Make()
	{
		return {};
	}

	/** @overload */
	template<class F> static
	Timer Make( F&& _proc )
	{
		return { _proc };
	}

#if __cplusplus >= 201402L
	/** @overload */
	static
	Timer Make( std::nullptr_t )
	{
		return Make();
	}

	/** @overload */
	template<typename F, typename... A> static
	Timer Make( F&& _proc, A&&... _args )
	{
		return Make(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif

/**
 * @brief   create and initialize static timer object
 *          and start periodic timer for given duration of time
 *
 *          when the timer has finished the countdown, the callback procedure is launched
 *
 *          do this periodically
 *
 * @par     Parameters
 *          - delay: duration of time (maximum number of ticks to countdown) for first expiration
 *            - IMMEDIATE: don't countdown
 *            - INFINITE:  countdown indefinitely
 *          - period: duration of time (maximum number of ticks to countdown) for all next expirations
 *            - IMMEDIATE: don't countdown
 *            - INFINITE:  countdown indefinitely
 *          - proc: callback procedure
 *            - none / nullptr: no callback
 *          - args: arguments for callback procedure
 *
 * @return  Timer object
 */

	template<typename T> static
	Timer Start( const T& _delay, const T& _period )
	{
		return { Held_t{}, [&]( Timer& _tmr ){ _tmr.start(_delay, _period); } };
	}

	/** @overload */
	template<typename T, class F> static
	Timer Start( const T& _delay, const T& _period, F&& _proc )
	{
		return { Held_t{}, [&]( Timer& _tmr ){ _tmr.start(_delay, _period); }, _proc };
	}

#if __cplusplus >= 201402L
	/** @overload */
	template<typename T> static
	Timer Start( const T& _delay, const T& _period, std::nullptr_t )
	{
		return Start(_delay, _period);
	}

	/** @overload */
	template<typename T, typename F, typename... A> static
	Timer Start( const T& _delay, const T& _period, F&& _proc, A&&... _args )
	{
		return Start(_delay, _period, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif

/**
 * @brief   create and initialize static timer object
 *          and start one-shot timer for given duration of time
 *
 *          when the timer has finished the countdown, the callback procedure is launched
 *
 * @par     Parameters
 *          - delay: duration of time (maximum number of ticks to countdown)
 *            - IMMEDIATE: don't countdown
 *            - INFINITE:  countdown indefinitely
 *          - proc: callback procedure
 *            - none / nullptr: no callback
 *          - args: arguments for callback procedure
 *
 * @return  Timer object
 */

	template<typename T> static
	Timer StartFor( const T& _delay )
	{
		return { Held_t{}, [&]( Timer& _tmr ){ _tmr.startFor(_delay); } };
	}

	/** @overload */
	template<typename T, class F> static
	Timer StartFor( const T& _delay, F&& _proc )
	{
		return { Held_t{}, [&]( Timer& _tmr ){ _tmr.startFor(_delay); }, _proc };
	}

#if __cplusplus >= 201402L
	/** @overload */
	template<typename T> static
	Timer StartFor( const T& _delay, std::nullptr_t )
	{
		return StartFor(_delay);
	}

	/** @overload */
	template<typename T, typename F, typename... A> static
	Timer StartFor( const T& _delay, F&& _proc, A&&... _args )
	{
		return StartFor(_delay, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif

/**
 * @brief   create and initialize static timer object
 *          and start periodic timer for given duration of time
 *
 *          when the timer has finished the countdown, the callback procedure is launched
 *
 *          do this periodically
 *
 * @par     Parameters
 *          - period: duration of time (maximum number of ticks to countdown)
 *            - IMMEDIATE: don't countdown
 *            - INFINITE:  countdown indefinitely
 *          - proc: callback procedure
 *            - none / nullptr: no callback
 *          - args: arguments for callback procedure
 *
 * @return  Timer object
 */

	template<typename T> static
	Timer StartPeriodic( const T& _period )
	{
		return { Held_t{}, [&]( Timer& _tmr ){ _tmr.startPeriodic(_period); } };
	}

	/** @overload */
	template<typename T, class F> static
	Timer StartPeriodic( const T& _period, F&& _proc )
	{
		return { Held_t{}, [&]( Timer& _tmr ){ _tmr.startPeriodic(_period); }, _proc };
	}

#if __cplusplus >= 201402L
	/** @overload */
	template<typename T> static
	Timer StartPeriodic( const T& _period, std::nullptr_t )
	{
		return StartPeriodic(_period);
	}

	/** @overload */
	template<typename T, typename F, typename... A> static
	Timer StartPeriodic( const T& _period, F&& _proc, A&&... _args )
	{
		return StartPeriodic(_period, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif

/**
 * @brief   create and initialize static timer object
 *          and start one-shot timer until given timepoint
 *
 *          when the timer has finished the countdown, the callback procedure is launched
 *
 * @par     Parameters
 *          - time: timepoint value
 *          - proc: callback procedure
 *            - none / nullptr: no callback
 *          - args: arguments for callback procedure
 *
 * @return  Timer object
 */

	template<typename T> static
	Timer StartUntil( const T& _time )
	{
		return { Held_t{}, [&]( Timer& _tmr ){ _tmr.startUntil(_time); } };
	}

	/** @overload */
	template<typename T, class F> static
	Timer StartUntil( const T& _time, F&& _proc )
	{
		return { Held_t{}, [&]( Timer& _tmr ){ _tmr.startUntil(_time); }, _proc };
	}

#if __cplusplus >= 201402L
	/** @overload */
	template<typename T> static
	Timer StartUntil( const T& _time, std::nullptr_t )
	{
		return StartUntil(_time);
	}

	/** @overload */
	template<typename T, typename F, typename... A> static
	Timer StartUntil( const T& _time, F&& _proc, A&&... _args )
	{
		return StartUntil(_time, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

/** @} */

#endif//__INTROS_TMR_H
