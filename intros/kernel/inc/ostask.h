/******************************************************************************

    @file    IntrOS: ostask.h
    @author  Rajmund Szymanski
    @date    10.04.2023
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

#ifndef __INTROS_TSK_H
#define __INTROS_TSK_H

#include "oskernel.h"
#include "osclock.h"

/* -------------------------------------------------------------------------- */

#define STK_SIZE( size ) \
    ALIGNED_SIZE((size_t)( size ) + (OS_GUARD_SIZE), sizeof(stk_t))

#define STK_OVER( size ) \
         ALIGNED((size_t)( size ) + (OS_GUARD_SIZE), sizeof(stk_t))

#define STK_CROP( base, size ) \
         LIMITED((size_t)( base ) + (size_t)( size ), sizeof(stk_t))

/******************************************************************************
 *
 * Name              : task (thread)
 *
 ******************************************************************************/

struct __tsk
{
	hdr_t    hdr;   // timer / task header

	fun_t *  proc;  // task proc (initial task function, doesn't have to be noreturn-type)
	void *   arg;   // reserved for internal use
	cnt_t    start; // inherited from timer
	cnt_t    delay; // inherited from timer
	cnt_t    period;// inherited from timer

	stk_t *  stack; // base of stack
	size_t   size;  // size of stack (in bytes)

	struct {
	unsigned sigset;// pending signals
	act_t *  action;// signal handler
	struct {
	fun_t *  pc;
	cnt_t    delay;
	}        backup;
	}        sig;

	union  {
	ctx_t    reg;   // task context
	jmp_buf  buf;   // setjmp/longjmp buffer
	}        ctx;
};

typedef struct __tsk tsk_id [];

/******************************************************************************
 *
 * Name              : _TSK_INIT
 *
 * Description       : create and initialize a task object
 *
 * Parameters
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   stack           : base of task's private stack storage
 *   size            : size of task private stack (in bytes)
 *
 * Return            : task object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _TSK_INIT( _proc, _stack, _size ) \
                    { _HDR_INIT(), _proc, NULL, 0, 0, 0, _stack, _size, { 0, NULL, { NULL, 0 } }, { _CTX_INIT() } }

/******************************************************************************
 *
 * Name              : _TSK_STACK
 *
 * Description       : create task's private stack storage
 *
 * Parameters
 *   size            : size of task's private stack storage (in bytes)
 *
 * Return            : base of task's private stack storage
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#ifndef __cplusplus
#define               _TSK_STACK( _size ) (stk_t [STK_SIZE( _size )]) { 0 }
#endif

/******************************************************************************
 *
 * Name              : _VA_STK
 *
 * Description       : calculate stack size from optional parameter
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _VA_STK( _size ) ((_size + 0) ? (_size + 0) : (OS_STACK_SIZE))

/******************************************************************************
 *
 * Name              : OS_TSK_STACK
 * Static alias      : static_TSK_STACK
 *
 * Description       : define task stack
 *
 * Parameters
 *   stk             : name of the stack (passed to the init function)
 *   size            : (optional) size of task stack (in bytes); default: OS_STACK_SIZE
 *
 ******************************************************************************/

#define             OS_TSK_STACK( stk, ... ) \
                       stk_t stk[ STK_SIZE( _VA_STK(__VA_ARGS__) ) ] __STKALIGN

#define         static_TSK_STACK( stk, ... ) \
                static stk_t stk[ STK_SIZE( _VA_STK(__VA_ARGS__) ) ] __STKALIGN

/******************************************************************************
 *
 * Name              : OS_WRK
 * Static alias      : static_WRK
 *
 * Description       : define and initialize complete work area for task object
 *
 * Parameters
 *   tsk             : name of a pointer to task object
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   size            : size of task private stack (in bytes)
 *
 ******************************************************************************/

#define             OS_WRK( tsk, proc, size )                        \
                static stk_t tsk##__stk[STK_SIZE( size )] __STKALIGN; \
                       tsk_t tsk[] = { _TSK_INIT( proc, tsk##__stk, STK_OVER( size ) ) }

#define         static_WRK( tsk, proc, size )                        \
                static stk_t tsk##__stk[STK_SIZE( size )] __STKALIGN; \
                static tsk_t tsk[] = { _TSK_INIT( proc, tsk##__stk, STK_OVER( size ) ) }

/******************************************************************************
 *
 * Name              : OS_TSK
 * Static alias      : static_TSK
 *
 * Description       : define and initialize complete work area for task object with default stack size
 *
 * Parameters
 *   tsk             : name of a pointer to task object
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   size            : (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 *
 ******************************************************************************/

#define             OS_TSK( tsk, proc, ... ) \
                    OS_WRK( tsk, proc, _VA_STK(__VA_ARGS__) )

#define         static_TSK( tsk, proc, ... ) \
                static_WRK( tsk, proc, _VA_STK(__VA_ARGS__) )

/******************************************************************************
 *
 * Name              : OS_WRK_DEF
 * Static alias      : static_WRK_DEF
 *
 * Description       : define and initialize complete work area for task object
 *                     task proc (function body) must be defined immediately below
 *
 * Parameters
 *   tsk             : name of a pointer to task object
 *   size            : size of task private stack (in bytes)
 *
 ******************************************************************************/

#define             OS_WRK_DEF( tsk, size )        \
                static void tsk##__fun( void );     \
                    OS_WRK( tsk, tsk##__fun, size ); \
                       void tsk##__fun( void )

#define         static_WRK_DEF( tsk, size )        \
                static void tsk##__fun( void );     \
                static_WRK( tsk, tsk##__fun, size ); \
                static void tsk##__fun( void )

/******************************************************************************
 *
 * Name              : OS_TSK_DEF
 * Static alias      : static_TSK_DEF
 *
 * Description       : define and initialize complete work area for task object with default stack size
 *                     task proc (function body) must be defined immediately below
 *
 * Parameters
 *   tsk             : name of a pointer to task object
 *   size            : (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 *
 ******************************************************************************/

#define             OS_TSK_DEF( tsk, ... ) \
                    OS_WRK_DEF( tsk, _VA_STK(__VA_ARGS__) )

#define         static_TSK_DEF( tsk, ... ) \
                static_WRK_DEF( tsk, _VA_STK(__VA_ARGS__) )

/******************************************************************************
 *
 * Name              : OS_WRK_START
 * Static alias      : static_WRK_START
 *
 * Description       : define, initialize and start complete work area for task object
 *                     task proc (function body) must be defined immediately below
 *
 * Parameters
 *   tsk             : name of a pointer to task object
 *   size            : size of task private stack (in bytes)
 *
 * Note              : only available for compilers supporting the "constructor" function attribute or its equivalent
 *
 ******************************************************************************/

#ifdef __CONSTRUCTOR
#define             OS_WRK_START( tsk, size )                   \
                static void tsk##__fun( void );                  \
                    OS_WRK( tsk, tsk##__fun, size );              \
  __CONSTRUCTOR static void tsk##__run( void ) { tsk_start(tsk); } \
                static void tsk##__fun( void )
#endif

#ifdef __CONSTRUCTOR
#define         static_WRK_START( tsk, size )                   \
                static void tsk##__fun( void );                  \
                static_WRK( tsk, tsk##__fun, size );              \
  __CONSTRUCTOR static void tsk##__run( void ) { tsk_start(tsk); } \
                static void tsk##__fun( void )
#endif

/******************************************************************************
 *
 * Name              : OS_TSK_START
 * Static alias      : static_TSK_START
 *
 * Description       : define, initialize and start complete work area for task object with default stack size
 *                     task proc (function body) must be defined immediately below
 *
 * Parameters
 *   tsk             : name of a pointer to task object
 *   size            : (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 *
 * Note              : only available for compilers supporting the "constructor" function attribute or its equivalent
 *
 ******************************************************************************/

#ifdef __CONSTRUCTOR
#define             OS_TSK_START( tsk, ... ) \
                    OS_WRK_START( tsk, _VA_STK(__VA_ARGS__) )
#endif

#ifdef __CONSTRUCTOR
#define         static_TSK_START( tsk, ... ) \
                static_WRK_START( tsk, _VA_STK(__VA_ARGS__) )
#endif

/******************************************************************************
 *
 * Name              : WRK_INIT
 *
 * Description       : create and initialize complete work area for task object
 *
 * Parameters
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   size            : size of task private stack (in bytes)
 *
 * Return            : task object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                WRK_INIT( proc, size ) \
                      _TSK_INIT( proc, _TSK_STACK( size ), STK_OVER( size ) )
#endif

/******************************************************************************
 *
 * Name              : WRK_CREATE
 * Alias             : WRK_NEW
 *
 * Description       : create and initialize complete work area for task object
 *
 * Parameters
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   size            : size of task private stack (in bytes)
 *
 * Return            : task object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                WRK_CREATE( proc, size ) \
                     { WRK_INIT  ( proc, size ) }
#define                WRK_NEW \
                       WRK_CREATE
#endif

/******************************************************************************
 *
 * Name              : TSK_INIT
 *
 * Description       : create and initialize complete work area for task object with default stack size
 *
 * Parameters
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   size            : (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 *
 * Return            : task object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                TSK_INIT( proc, ... ) \
                       WRK_INIT( proc, _VA_STK(__VA_ARGS__) )
#endif

/******************************************************************************
 *
 * Name              : TSK_CREATE
 * Alias             : TSK_NEW
 *
 * Description       : create and initialize complete work area for task object with default stack size
 *
 * Parameters
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   size            : (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 *
 * Return            : task object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                TSK_CREATE( proc, ... ) \
                       WRK_CREATE( proc, _VA_STK(__VA_ARGS__) )
#define                TSK_NEW \
                       TSK_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : tsk_this
 * Alias             : cur_task
 * Alias             : SELF
 *
 * Description       : return current task object
 *
 * Parameters        : none
 *
 * Return            : current task object
 *
 ******************************************************************************/

__STATIC_INLINE
tsk_t *tsk_this( void ) { return System.cur; }

__STATIC_INLINE
tsk_t *cur_task( void ) { return System.cur; }

#define SELF                   ( System.cur )

/******************************************************************************
 *
 * Name              : wrk_init
 *
 * Description       : initialize complete work area for task object
 *                     don't start the task
 *
 * Parameters
 *   tsk             : pointer to task object
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   stack           : base of task's private stack storage
 *   size            : size of task private stack (in bytes)
 *
 * Return            : none
 *
 ******************************************************************************/

void wrk_init( tsk_t *tsk, fun_t *proc, stk_t *stack, size_t size );

/******************************************************************************
 *
 * Name              : tsk_init
 *
 * Description       : initialize complete work area for task object
 *                     and start the task, if proc != NULL
 *
 * Parameters
 *   tsk             : pointer to task object
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   stack           : base of task's private stack storage
 *   size            : size of task private stack (in bytes)
 *
 * Return            : none
 *
 ******************************************************************************/

void tsk_init( tsk_t *tsk, fun_t *proc, stk_t *stack, size_t size );

/******************************************************************************
 *
 * Name              : tsk_start
 *
 * Description       : start previously defined/created/stopped task object
 *
 * Parameters
 *   tsk             : pointer to task object
 *
 * Return            : none
 *
 ******************************************************************************/

void tsk_start( tsk_t *tsk );

/******************************************************************************
 *
 * Name              : tsk_startFrom
 *
 * Description       : start previously defined/created/stopped task object
 *
 * Parameters
 *   tsk             : pointer to task object
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *
 * Return            : none
 *
 ******************************************************************************/

void tsk_startFrom( tsk_t *tsk, fun_t *proc );

/******************************************************************************
 *
 * Name              : tsk_startWith
 *
 * Description       : start previously defined/created/stopped task object
 *
 * Parameters
 *   tsk             : pointer to task object
 *   proc            : task initial procedure (initial task function)
 *   arg             : task initial procedure argument (for internal use)
 *
 * Return            : none
 *
 ******************************************************************************/

void tsk_startWith( tsk_t *tsk, fun_a *proc, void *arg );

/******************************************************************************
 *
 * Name              : tsk_stop
 * Alias             : tsk_exit
 *
 * Description       : stop execution of current task
 *
 * Parameters        : none
 *
 * Return            : none
 *
 ******************************************************************************/

__NO_RETURN
void tsk_stop( void );

__STATIC_INLINE
void tsk_exit( void ) { tsk_stop(); }

/******************************************************************************
 *
 * Name              : tsk_reset
 * Alias             : tsk_kill
 *
 * Description       : stop execution of given task
 *
 * Parameters
 *   tsk             : pointer to task object
 *
 * Return            : none
 *
 ******************************************************************************/

void tsk_reset( tsk_t *tsk );

__STATIC_INLINE
void tsk_kill( tsk_t *tsk ) { tsk_reset(tsk); }

/******************************************************************************
 *
 * Name              : tsk_join
 *
 * Description       : delay execution of current task until termination of given task
 *
 * Parameters
 *   tsk             : pointer to task object
 *
 * Return            : none
 *
 ******************************************************************************/

void tsk_join( tsk_t *tsk );

/******************************************************************************
 *
 * Name              : tsk_yield
 * Alias             : tsk_pass
 *
 * Description       : yield system control to the next task
 *
 * Parameters        : none
 *
 * Return            : none
 *
 ******************************************************************************/

__STATIC_INLINE
void tsk_yield( void ) { core_ctx_switch(); }

__STATIC_INLINE
void tsk_pass ( void ) { core_ctx_switch(); }

/******************************************************************************
 *
 * Name              : tsk_flip
 *
 * Description       : restart current task with the new proc (task function)
 *
 * Parameters
 *   proc            : new task proc (task function)
 *
 * Return            : none
 *
 ******************************************************************************/

__NO_RETURN
void tsk_flip( fun_t *proc );

/******************************************************************************
 *
 * Name              : tsk_sleepFor
 * Alias             : tsk_delay
 *
 * Description       : delay execution of current task for given duration of time
 *
 * Parameters
 *   delay           : duration of time (maximum number of ticks to delay execution of current task)
 *                     IMMEDIATE: don't delay execution of current task
 *                     INFINITE:  delay indefinitely execution of current task
 *
 * Return            : none
 *
 ******************************************************************************/

void tsk_sleepFor( cnt_t delay );

__STATIC_INLINE
void tsk_delay( cnt_t delay ) { tsk_sleepFor(delay); }

/******************************************************************************
 *
 * Name              : tsk_sleepNext
 *
 * Description       : delay execution of current task for given duration of time
 *                     from the end of the previous countdown
 *
 * Parameters
 *   delay           : duration of time (maximum number of ticks to delay execution of current task)
 *                     IMMEDIATE: don't delay execution of current task
 *                     INFINITE:  delay indefinitely execution of current task
 *
 * Return            : none
 *
 ******************************************************************************/

void tsk_sleepNext( cnt_t delay );

/******************************************************************************
 *
 * Name              : tsk_sleepUntil
 *
 * Description       : delay execution of current task until given timepoint
 *
 * Parameters
 *   time            : timepoint value
 *
 * Return            : none
 *
 ******************************************************************************/

void tsk_sleepUntil( cnt_t time );

/******************************************************************************
 *
 * Name              : tsk_sleep
 *
 * Description       : delay indefinitely execution of current task
 *                     execution of the task can be resumed
 *
 * Parameters        : none
 *
 * Return            : none
 *
 ******************************************************************************/

__STATIC_INLINE
void tsk_sleep( void ) { tsk_sleepFor(INFINITE); }

/******************************************************************************
 *
 * Name              : tsk_suspend
 *
 * Description       : delay indefinitely execution of given task
 *                     execution of the task can be resumed
 *
 * Parameters
 *   tsk             : pointer to task object
 *
 * Return
 *   SUCCESS         : task was successfully suspended
 *   FAILURE         : task cannot be suspended
 *
 ******************************************************************************/

unsigned tsk_suspend( tsk_t *tsk );

/******************************************************************************
 *
 * Name              : tsk_resume
 *
 * Description       : resume execution of given suspended task
 *                     only suspended or indefinitely delayed tasks can be resumed
 *
 * Parameters
 *   tsk             : pointer to delayed task object
 *
 * Return
 *   SUCCESS         : task was successfully resumed
 *   FAILURE         : task cannot be resumed
 *
 ******************************************************************************/

unsigned tsk_resume( tsk_t *tsk );

/******************************************************************************
 *
 * Name              : tsk_give
 * Alias             : tsk_signal
 *
 * Description       : send given signal to the task
 *
 * Parameters
 *   tsk             : pointer to the task object
 *   signo           : signal number
 *
 * Return            : none
 *
 ******************************************************************************/

void tsk_give( tsk_t *tsk, unsigned signo );

__STATIC_INLINE
void tsk_signal( tsk_t *tsk, unsigned signo ) { tsk_give(tsk, signo); }

/******************************************************************************
 *
 * Name              : tsk_setAction
 *
 * Description       : set given function as a signal handler
 *
 * Parameters
 *   tsk             : pointer to the task object
 *   action          : signal handler
 *
 * Return            : none
 *
 ******************************************************************************/

void tsk_setAction( tsk_t *tsk, act_t *action );

/******************************************************************************
 *
 * Name              : tsk_stackSpace
 *
 * Description       : chack water mark of the stack of the current task
 *
 * Parameters        : none
 *
 * Return            : high water mark of the stack of the current task
 *   0               : DEBUG not defined
 *
 ******************************************************************************/

__STATIC_INLINE
size_t tsk_stackSpace( void )
{
#ifdef DEBUG
	return core_stk_space(System.cur);
#else
	return 0;
#endif
}

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
namespace intros {

/******************************************************************************
 *
 * Class             : baseStack
 *
 * Description       : create base class for stack storage object
 *
 * Constructor parameters
 *   size            : size of stack (in bytes)
 *
 * Note              : for internal use
 *
 ******************************************************************************/

template<size_t size_>
struct baseStack
{
	static_assert(size_>0, "incorrect stack size");
#if __cplusplus >= 201703L && !defined(__ICCARM__)
	stk_t stack_[STK_SIZE(size_)] __STKALIGN;
#else
	stk_t stack_[STK_SIZE(size_)];
#endif
};

/******************************************************************************
 *
 * Class             : baseTask
 *
 * Description       : create and initialize base class for task objects
 *
 * Constructor parameters
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   stack           : base of task's private stack storage
 *   size            : size of task private stack (in bytes)
 *
 * Note              : for internal use
 *
 ******************************************************************************/

struct baseTask : public __tsk
{
#if __cplusplus >= 201402L
	template<class F>
	baseTask( F&& _proc, stk_t * const _stack, const size_t _size ): __tsk _TSK_INIT(fun_, _stack, _size), fun{_proc} {}
#else
	baseTask( fun_t *_proc, stk_t * const _stack, const size_t _size ): __tsk _TSK_INIT(_proc, _stack, _size) {}
#endif

	void     start    ()                   {        tsk_start    (this); }
#if __cplusplus >= 201402L
	template<class F>
	void     startFrom( F&&      _proc )   {        new (&fun) Fun_t(_proc);
	                                                tsk_startFrom(this, fun_); }
#else
	void     startFrom( fun_t  * _proc )   {        tsk_startFrom(this, _proc); }
#endif
	void     join     ()                   {        tsk_join     (this); }
	void     reset    ()                   {        tsk_reset    (this); }
	void     kill     ()                   {        tsk_kill     (this); }
	unsigned suspend  ()                   { return tsk_suspend  (this); }
	unsigned resume   ()                   { return tsk_resume   (this); }
	void     give     ( unsigned _signo )  {        tsk_give     (this, _signo); }
	void     signal   ( unsigned _signo )  {        tsk_signal   (this, _signo); }
#if __cplusplus >= 201402L
	template<class F>
	void     setAction( F&&      _action ) {        new (&act) Act_t(_action);
	                                                tsk_setAction(this, act_); }
#else
	void     setAction( act_t  * _action ) {        tsk_setAction(this, _action); }
#endif
	explicit
	operator bool     () const             { return __tsk::hdr.id != ID_STOPPED; }

	template<class T = baseTask> static
	T *      current  ()                   { return static_cast<T *>(tsk_this()); }

#if __cplusplus >= 201402L
	static
	void     fun_     ()                   {        current()->fun(); }
	Fun_t    fun;
	static
	void     act_     ( unsigned _signo )  {        current()->act(_signo); }
	Act_t    act;
#endif

/******************************************************************************
 *
 * Class             : [base]Task::Current
 *
 * Description       : provide set of functions for current task
 *
 ******************************************************************************/

	struct Current
	{
		static
		void stop      ()                   {        tsk_stop      (); }
		static
		void exit      ()                   {        tsk_exit      (); }
		static
		void reset     ()                   {        tsk_reset     (current()); }
		static
		void kill      ()                   {        tsk_kill      (current()); }
		static
		void yield     ()                   {        tsk_yield     (); }
		static
		void pass      ()                   {        tsk_pass      (); }
#if __cplusplus >= 201402L
		template<class F> static
		void flip      ( F&&      _proc )   {        new (&current()->fun) Fun_t(_proc);
		                                             tsk_flip      (fun_); }
#else
		static
		void flip      ( fun_t  * _proc )   {        tsk_flip      (_proc); }
#endif
		template<typename T> static
		void sleepFor  ( const T& _delay )  {        tsk_sleepFor  (Clock::count(_delay)); }
		template<typename T> static
		void sleepNext ( const T& _delay )  {        tsk_sleepNext (Clock::count(_delay)); }
		template<typename T> static
		void sleepUntil( const T& _time )   {        tsk_sleepUntil(Clock::until(_time)); }
		static
		void sleep     ()                   {        tsk_sleep     (); }
		template<typename T> static
		void delay     ( const T& _delay )  {        tsk_delay     (Clock::count(_delay)); }
		static
		void suspend   ()                   {        tsk_suspend   (current()); }
		static
		void give      ( unsigned _signo )  {        tsk_give      (current(), _signo); }
		static
		void signal    ( unsigned _signo )  {        tsk_signal    (current(), _signo); }
#if __cplusplus >= 201402L
		template<class F> static
		void setAction ( F&&      _action ) {        new (&current()->act) Act_t(_action);
		                                             tsk_setAction (current(), act_); }
#else
		static
		void setAction ( act_t  * _action ) {        tsk_setAction (current(), _action); }
#endif
	};
};

using thisTask = baseTask::Current;
using this_task = baseTask::Current;

/******************************************************************************
 *
 * Class             : TaskT<>
 *
 * Description       : create and initialize complete work area for task object
 *
 * Constructor parameters
 *   size            : size of task private stack (in bytes)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *
 ******************************************************************************/

template<size_t size_>
struct TaskT : public baseTask, public baseStack<size_>
{
	template<class F>
	TaskT( F&& _proc ):
	baseTask{_proc, baseStack<size_>::stack_, sizeof(baseStack<size_>::stack_)} {}
#if __cplusplus >= 201402L
	template<typename F, typename... A>
	TaskT( F&& _proc, A&&... _args ):
	baseTask{std::bind(std::forward<F>(_proc), std::forward<A>(_args)...), baseStack<size_>::stack_, sizeof(baseStack<size_>::stack_)} {}
#endif

	~TaskT() { assert(__tsk::hdr.id == ID_STOPPED); }

	TaskT( TaskT<size_>&& ) = default;
	TaskT( const TaskT<size_>& ) = delete;
	TaskT<size_>& operator=( TaskT<size_>&& ) = delete;
	TaskT<size_>& operator=( const TaskT<size_>& ) = delete;

/******************************************************************************
 *
 * Name              : TaskT<>::Make
 *
 * Description       : create and initialize task object
 *
 * Parameters
 *   size            : size of task private stack (in bytes)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   args            : arguments for task proc
 *
 * Return            : TaskT<> object
 *
 ******************************************************************************/

	template<class F> static
	TaskT<size_> Make( F&& _proc )
	{
		return { _proc };
	}

#if __cplusplus >= 201402L
	template<typename F, typename... A> static
	TaskT<size_> Make( F&& _proc, A&&... _args )
	{
		return Make(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif

/******************************************************************************
 *
 * Name              : TaskT<>::Start
 *
 * Description       : create, initialize and start task object
 *
 * Parameters
 *   size            : size of task private stack (in bytes)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   args            : arguments for task proc
 *
 * Return            : TaskT<> object
 *
 ******************************************************************************/

	template<class F> static
	TaskT<size_> Start( F&& _proc )
	{
		TaskT<size_> tsk { _proc };
		tsk.start();
		return tsk;
	}

#if __cplusplus >= 201402L
	template<typename F, typename... A> static
	TaskT<size_> Start( F&& _proc, A&&... _args )
	{
		return Start(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif
};

/******************************************************************************
 *
 * Class             : Task
 *
 * Description       : create and initialize complete work area for task object
 *                     with default stack size
 *
 ******************************************************************************/

using Task = TaskT<OS_STACK_SIZE>;

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_TSK_H
