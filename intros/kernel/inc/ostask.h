/******************************************************************************

    @file    IntrOS: ostask.h
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

#ifndef __INTROS_TSK_H
#define __INTROS_TSK_H

#include "oskernel.h"
#include "osclock.h"

/**
 * @defgroup task Task
 * @brief    tasks (creation, control, delays)
 * @{
 */

/* -------------------------------------------------------------------------- */

/** size of the task stack in stk_t units: the size in bytes (rounded up) with the guard area */
#define STK_SIZE( size ) \
    ALIGNED_SIZE((size_t)( size ) + (OS_GUARD_SIZE), sizeof(stk_t))

#define STK_OVER( size ) \
         ALIGNED((size_t)( size ) + (OS_GUARD_SIZE), sizeof(stk_t))

#define STK_CROP( base, size ) \
         LIMITED((size_t)( base ) + (size_t)( size ), sizeof(stk_t))

/**
 * @brief   task (thread)
 */

struct __tsk
{
	obj_t    obj;   ///< object header
	hdr_t    hdr;   ///< timer / task header

	fun_t  * proc;  ///< task proc (initial task function, doesn't have to be noreturn-type)
	void   * arg;   ///< argument of the task function
	cnt_t    start; ///< inherited from timer
	cnt_t    delay; ///< inherited from timer
	cnt_t    period;///< inherited from timer

	stk_t  * stack; ///< base of stack
	size_t   size;  ///< size of stack (in bytes)

	tsk_t ** guard; ///< BLOCKED queue for the pending process
	int      event; ///< wakeup event

	struct {
	mtx_t  * list;  ///< list of mutexes held
	}        mtx;

	union  {

	struct {
	unsigned event;
	}        evt;   ///< temporary data used by event object

	struct {
	unsigned sigset;
	unsigned signo;
	}        sig;   ///< temporary data used by signal object

	struct {
	unsigned flags;
	unsigned mode;
	}        flg;   ///< temporary data used by flag object

	struct {
	void   * data;
	}        lst;   ///< temporary data used by list / memory pool object

	struct {
	union  {
	const
	char   * out;
	char   * in;
	}        data;
	size_t   size;
	}        raw;   ///< temporary data used by raw buffer object

	struct {
	union  {
	const
	char   * out;
	char   * in;
	}        data;
	size_t   size;
	}        msg;   ///< temporary data used by message buffer object

	struct {
	union  {
	const
	void   * out;
	void   * in;
	}        data;
	}        box;   ///< temporary data used by mailbox queue object

	struct {
	unsigned event;
	}        evq;   ///< temporary data used by event queue object

	struct {
	fun_t  * fun;
	}        job;   ///< temporary data used by job queue object

	}        tmp;

	union  {
	ctx_t    reg;   ///< task context
	jmp_buf  buf;   ///< setjmp/longjmp buffer
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
                    { _OBJ_INIT(), _HDR_INIT(), _proc, NULL, 0, 0, 0, _stack, _size, \
                      NULL, 0, { NULL }, { { 0 } }, { _CTX_INIT() } }

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

/**
 * @brief   define task stack
 *
 * @param   stk        name of the stack (passed to the init function)
 * @param   ...        size: (optional) size of task stack (in bytes); default: OS_STACK_SIZE
 */

#define             OS_TSK_STACK( stk, ... ) \
                       stk_t stk[ STK_SIZE( _VA_STK(__VA_ARGS__) ) ] __STKALIGN

/** static alias of OS_TSK_STACK() */
#define         static_TSK_STACK( stk, ... ) \
                static stk_t stk[ STK_SIZE( _VA_STK(__VA_ARGS__) ) ] __STKALIGN

/**
 * @brief   define and initialize complete work area for task object
 *
 * @param   tsk        name of a pointer to task object
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   size       size of task private stack (in bytes)
 */

#define             OS_WRK( tsk, proc, size )                        \
                static stk_t tsk##__stk[STK_SIZE( size )] __STKALIGN; \
                       tsk_t tsk[] = { _TSK_INIT( proc, tsk##__stk, STK_OVER( size ) ) }

/** static alias of OS_WRK() */
#define         static_WRK( tsk, proc, size )                        \
                static stk_t tsk##__stk[STK_SIZE( size )] __STKALIGN; \
                static tsk_t tsk[] = { _TSK_INIT( proc, tsk##__stk, STK_OVER( size ) ) }

/**
 * @brief   define and initialize complete work area for task object with default stack size
 *
 * @param   tsk        name of a pointer to task object
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   ...        size: (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 */

#define             OS_TSK( tsk, proc, ... ) \
                    OS_WRK( tsk, proc, _VA_STK(__VA_ARGS__) )

/** static alias of OS_TSK() */
#define         static_TSK( tsk, proc, ... ) \
                static_WRK( tsk, proc, _VA_STK(__VA_ARGS__) )

/**
 * @brief   define and initialize complete work area for task object
 *
 *          task proc (function body) must be defined immediately below
 *
 * @param   tsk        name of a pointer to task object
 * @param   size       size of task private stack (in bytes)
 */

#define             OS_WRK_DEF( tsk, size )        \
                static void tsk##__fun( void );     \
                    OS_WRK( tsk, tsk##__fun, size ); \
                       void tsk##__fun( void )

/** static alias of OS_WRK_DEF() */
#define         static_WRK_DEF( tsk, size )        \
                static void tsk##__fun( void );     \
                static_WRK( tsk, tsk##__fun, size ); \
                static void tsk##__fun( void )

/**
 * @brief   define and initialize complete work area for task object with default stack size
 *
 *          task proc (function body) must be defined immediately below
 *
 * @param   tsk        name of a pointer to task object
 * @param   ...        size: (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 */

#define             OS_TSK_DEF( tsk, ... ) \
                    OS_WRK_DEF( tsk, _VA_STK(__VA_ARGS__) )

/** static alias of OS_TSK_DEF() */
#define         static_TSK_DEF( tsk, ... ) \
                static_WRK_DEF( tsk, _VA_STK(__VA_ARGS__) )

/**
 * @brief   define, initialize and start complete work area for task object
 *
 *          task proc (function body) must be defined immediately below
 *
 * @param   tsk        name of a pointer to task object
 * @param   size       size of task private stack (in bytes)
 *
 * @note    only available for compilers supporting the "constructor" function attribute or its equivalent
 */

#ifdef __CONSTRUCTOR
#define             OS_WRK_START( tsk, size )                   \
                static void tsk##__fun( void );                  \
                    OS_WRK( tsk, tsk##__fun, size );              \
  __CONSTRUCTOR static void tsk##__run( void ) { tsk_start(tsk); } \
                static void tsk##__fun( void )
#endif

#ifdef __CONSTRUCTOR
/** static alias of OS_WRK_START() */
#define         static_WRK_START( tsk, size )                   \
                static void tsk##__fun( void );                  \
                static_WRK( tsk, tsk##__fun, size );              \
  __CONSTRUCTOR static void tsk##__run( void ) { tsk_start(tsk); } \
                static void tsk##__fun( void )
#endif

/**
 * @brief   define, initialize and start complete work area for task object with default stack size
 *
 *          task proc (function body) must be defined immediately below
 *
 * @param   tsk        name of a pointer to task object
 * @param   ...        size: (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 *
 * @note    only available for compilers supporting the "constructor" function attribute or its equivalent
 */

#ifdef __CONSTRUCTOR
#define             OS_TSK_START( tsk, ... ) \
                    OS_WRK_START( tsk, _VA_STK(__VA_ARGS__) )
#endif

#ifdef __CONSTRUCTOR
/** static alias of OS_TSK_START() */
#define         static_TSK_START( tsk, ... ) \
                static_WRK_START( tsk, _VA_STK(__VA_ARGS__) )
#endif

/**
 * @brief   create and initialize complete work area for task object
 *
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   size       size of task private stack (in bytes)
 *
 * @return  task object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                WRK_INIT( proc, size ) \
                      _TSK_INIT( proc, _TSK_STACK( size ), STK_OVER( size ) )
#endif

/**
 * @brief   create and initialize complete work area for task object
 *
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   size       size of task private stack (in bytes)
 *
 * @return  task object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                WRK_CREATE( proc, size ) \
                     { WRK_INIT  ( proc, size ) }
/** alias of WRK_CREATE() */
#define                WRK_NEW \
                       WRK_CREATE
#endif

/**
 * @brief   create and initialize complete work area for task object with default stack size
 *
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   ...        size: (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 *
 * @return  task object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                TSK_INIT( proc, ... ) \
                       WRK_INIT( proc, _VA_STK(__VA_ARGS__) )
#endif

/**
 * @brief   create and initialize complete work area for task object with default stack size
 *
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   ...        size: (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 *
 * @return  task object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                TSK_CREATE( proc, ... ) \
                       WRK_CREATE( proc, _VA_STK(__VA_ARGS__) )
/** alias of TSK_CREATE() */
#define                TSK_NEW \
                       TSK_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   return current task object
 *
 * @return  current task object
 */

__STATIC_INLINE
tsk_t *tsk_this( void ) { return System.cur; }

/** alias of tsk_this() */
__STATIC_INLINE
tsk_t *cur_task( void ) { return System.cur; }

/** alias of tsk_this() */
#define SELF                   ( System.cur )

/**
 * @brief   initialize complete work area for task object
 *
 *          don't start the task
 *
 * @param   tsk        pointer to task object
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   stack      base of task's private stack storage
 * @param   size       size of task private stack (in bytes)
 */

void wrk_init( tsk_t *tsk, fun_t *proc, stk_t *stack, size_t size );

/**
 * @brief   initialize complete work area for task object
 *          and start the task, if proc != NULL
 *
 * @param   tsk        pointer to task object
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   stack      base of task's private stack storage
 * @param   size       size of task private stack (in bytes)
 */

void tsk_init( tsk_t *tsk, fun_t *proc, stk_t *stack, size_t size );

/**
 * @brief   start previously defined/created/stopped task object
 *
 * @param   tsk        pointer to task object
 */

void tsk_start( tsk_t *tsk );

/**
 * @brief   start previously defined/created/stopped task object
 *
 * @param   tsk        pointer to task object
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 */

void tsk_startFrom( tsk_t *tsk, fun_t *proc );

/**
 * @brief   start previously defined/created/stopped task object
 *
 * @param   tsk        pointer to task object
 * @param   proc       task initial procedure (initial task function)
 * @param   arg        task initial procedure argument (for internal use)
 */

void tsk_startWith( tsk_t *tsk, fun_a *proc, void *arg );

/**
 * @brief   stop execution of current task
 *
 *          all mutexes held by the task are released (waiting tasks get E_STOPPED)
 */

__NO_RETURN
void tsk_stop( void );

/** alias of tsk_stop() */
__STATIC_INLINE
void tsk_exit( void ) { tsk_stop(); }

/**
 * @brief   stop execution of given task
 *
 *          all mutexes held by the task are released (waiting tasks get E_STOPPED)
 *
 *          other objects held by the task (e.g. semaphores, read/write locks) are not released
 *
 * @param   tsk        pointer to task object
 *
 * @note    use only in thread mode
 * @note    readers waiting behind a writer stopped while waiting for a read/write lock
 *          are not released until the lock is given
 */

void tsk_reset( tsk_t *tsk );

/** alias of tsk_reset() */
__STATIC_INLINE
void tsk_kill( tsk_t *tsk ) { tsk_reset(tsk); }

/**
 * @brief   yield system control to the next task
 */

__STATIC_INLINE
void tsk_yield( void ) { core_ctx_switch(); }

/** alias of tsk_yield() */
__STATIC_INLINE
void tsk_pass ( void ) { core_ctx_switch(); }

/**
 * @brief   restart current task with the new proc (task function)
 *
 *          all mutexes held by the task are released (waiting tasks get E_STOPPED)
 *
 * @param   proc       new task proc (task function)
 */

__NO_RETURN
void tsk_flip( fun_t *proc );

/**
 * @brief   delay execution of current task for given duration of time
 *
 * @param   delay      duration of time (maximum number of ticks to delay execution of current task)
 *                     - IMMEDIATE: don't delay execution of current task (only give up the processor once)
 *                     - INFINITE:  delay indefinitely execution of current task
 */

void tsk_sleepFor( cnt_t delay );

/** alias of tsk_sleepFor() */
__STATIC_INLINE
void tsk_delay( cnt_t delay ) { tsk_sleepFor(delay); }

/**
 * @brief   delay execution of current task for given duration of time
 *
 *          from the end of the previous countdown
 *
 * @param   delay      duration of time (maximum number of ticks to delay execution of current task)
 *                     - IMMEDIATE: don't delay execution of current task (only give up the processor once)
 *                     - INFINITE:  delay indefinitely execution of current task
 */

void tsk_sleepNext( cnt_t delay );

/**
 * @brief   delay execution of current task until given timepoint
 *
 * @param   time       timepoint value
 *                     - past timepoint: only give up the processor once
 */

void tsk_sleepUntil( cnt_t time );

/**
 * @brief   delay indefinitely execution of current task
 *
 *          execution of the task can be resumed
 */

__STATIC_INLINE
void tsk_sleep( void ) { tsk_sleepFor(INFINITE); }

/**
 * @brief   delay indefinitely execution of given task
 *
 *          execution of the task can be resumed
 *
 * @param   tsk        pointer to task object
 *
 * @retval  E_SUCCESS  task was successfully suspended
 * @retval  E_FAILURE  task cannot be suspended
 *
 * @note    use only in thread mode
 */

int tsk_suspend( tsk_t *tsk );

/**
 * @brief   resume execution of given suspended task
 *
 *          only suspended or indefinitely blocked tasks can be resumed
 *
 * @param   tsk        pointer to suspended task object
 *
 * @retval  E_SUCCESS  task was successfully resumed
 * @retval  E_FAILURE  task cannot be resumed
 */

int tsk_resume( tsk_t *tsk );

/**
 * @brief   chack water mark of the stack of the current task
 *
 * @return  high water mark of the stack of the current task
 * @retval  0          DEBUG not defined or the current task is the main task (its stack size is unknown)
 */

__STATIC_INLINE
size_t tsk_stackSpace( void )
{
#ifdef DEBUG
	assert_tsk_context();
	return core_stk_space(System.cur);
#else
	return 0;
#endif
}

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus)
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
	void     startFrom( F&&      _proc )
	{
		if (__tsk::hdr.id == ID_STOPPED)  // active tasks cannot be started
		{
			fun = Fun_t(_proc);
			tsk_startFrom(this, fun_);
		}
	}
#else
	void     startFrom( fun_t  * _proc )   {        tsk_startFrom(this, _proc); }
#endif
	void     reset    ()                   {        tsk_reset    (this); }
	void     kill     ()                   {        tsk_kill     (this); }
	int      suspend  ()                   { return tsk_suspend  (this); }
	int      resume   ()                   { return tsk_resume   (this); }
	explicit
	operator bool     () const             { return __tsk::hdr.id != ID_STOPPED; }

	template<class T = baseTask> static
	T *      current  ()                   { return static_cast<T *>(tsk_this()); }

#if __cplusplus >= 201402L
	static
	void     fun_     ()                   {        current()->fun(); }
	Fun_t    fun;
#endif

/**
 * @brief   provide set of functions for current task
 */

	struct Current
	{
		static
		void     stop      ()                   {        tsk_stop      (); }
		static
		void     exit      ()                   {        tsk_exit      (); }
		static
		void     reset     ()                   {        tsk_reset     (current()); }
		static
		void     kill      ()                   {        tsk_kill      (current()); }
		static
		void     yield     ()                   {        tsk_yield     (); }
		static
		void     pass      ()                   {        tsk_pass      (); }
#if __cplusplus >= 201402L
		template<class F> static
		void     flip      ( F&&      _proc )   {        current()->fun = Fun_t(_proc);
		                                                 tsk_flip      (fun_); }
#else
		static
		void     flip      ( fun_t  * _proc )   {        tsk_flip      (_proc); }
#endif
		template<typename T> static
		void     sleepFor  ( const T& _delay )  {        tsk_sleepFor  (Clock::count(_delay)); }
		template<typename T> static
		void     sleepNext ( const T& _delay )  {        tsk_sleepNext (Clock::count(_delay)); }
		template<typename T> static
		void     sleepUntil( const T& _time )   {        tsk_sleepUntil(Clock::until(_time)); }
		static
		void     sleep     ()                   {        tsk_sleep     (); }
		template<typename T> static
		void     delay     ( const T& _delay )  {        tsk_delay     (Clock::count(_delay)); }
		static
		void     suspend   ()                   {        tsk_suspend   (current()); }
	};
};

using thisTask = baseTask::Current;
using this_task = baseTask::Current;

/**
 * @brief   create and initialize complete work area for task object
 *
 * @par     Constructor parameters
 *          - size: size of task private stack (in bytes)
 *          - proc: task proc (initial task function) doesn't have to be noreturn-type
 *            it will be executed into an infinite system-implemented loop
 */

template<size_t size_>
struct TaskT : public baseTask, public baseStack<size_>
{
	/// constructor, see the constructor parameters of the class
	template<class F>
	TaskT( F&& _proc ):
	baseTask{_proc, baseStack<size_>::stack_, sizeof(baseStack<size_>::stack_)} {}

#if __cplusplus >= 202002L
	/// constructor, see the constructor parameters of the class
	template<typename F, typename... A>
	TaskT( F&& _proc, A&&... _args ):
	TaskT<size_>{[proc = std::forward<F>(_proc), ...args = std::forward<A>(_args)]() mutable
		{ std::invoke(std::move(proc), std::unwrap_reference_t<A>(std::move(args))...); }} {}
#else
#if __cplusplus >= 201402L
	/// constructor, see the constructor parameters of the class
	template<typename F, typename... A>
	TaskT( F&& _proc, A&&... _args ):
	TaskT<size_>{std::bind(std::forward<F>(_proc), std::forward<A>(_args)...)} {}
#endif
#endif

	~TaskT() { assert(__tsk::hdr.id == ID_STOPPED); }

#if __cplusplus >= 201402L
	/// move constructor
	TaskT( TaskT<size_>&& _src ):
	baseTask{_src.fun, baseStack<size_>::stack_, sizeof(baseStack<size_>::stack_)}
#else
	/// move constructor
	TaskT( TaskT<size_>&& _src ):
	baseTask{_src.proc, baseStack<size_>::stack_, sizeof(baseStack<size_>::stack_)}
#endif
	{
		assert(_src.hdr.id == ID_STOPPED); // obj.queue of a task is only the link in the queue it waited in
		(void) _src;
	}

	TaskT( const TaskT<size_>& ) = delete;
	TaskT<size_>& operator=( TaskT<size_>&& ) = delete;
	TaskT<size_>& operator=( const TaskT<size_>& ) = delete;

private:
	struct Starter {}; // Start functions return the task started in place: a started task must not be moved
	template<typename... A>
	TaskT( Starter, A&&... _args ): TaskT<size_>{static_cast<A&&>(_args)...} { this->start(); }
public:

/**
 * @brief   create and initialize static task
 *
 * @par     Parameters
 *          - size: size of task private stack (in bytes)
 *          - proc: task proc (initial task function) doesn't have to be noreturn-type
 *            it will be executed into an infinite system-implemented loop
 *          - args: arguments for task proc
 *
 * @return  TaskT<> object
 */

	template<class F> static
	TaskT<size_> Make( F&& _proc )
	{
		return { _proc };
	}

#if __cplusplus >= 201402L
	/** @overload */
	template<typename F, typename... A> static
	TaskT<size_> Make( F&& _proc, A&&... _args )
	{
		return { std::forward<F>(_proc), std::forward<A>(_args)... };
	}
#endif

/**
 * @brief   create, initialize and start static task
 *
 * @par     Parameters
 *          - size: size of task private stack (in bytes)
 *          - proc: task proc (initial task function) doesn't have to be noreturn-type
 *            it will be executed into an infinite system-implemented loop
 *          - args: arguments for task proc
 *
 * @return  TaskT<> object
 */

	template<class F> static
	TaskT<size_> Start( F&& _proc )
	{
		return { Starter{}, _proc };
	}

#if __cplusplus >= 201402L
	/** @overload */
	template<typename F, typename... A> static
	TaskT<size_> Start( F&& _proc, A&&... _args )
	{
		return { Starter{}, std::forward<F>(_proc), std::forward<A>(_args)... };
	}
#endif
};

/**
 * @brief   create and initialize complete work area for task object
 *
 *          with default stack size
 */

using Task = TaskT<OS_STACK_SIZE>;

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

/** @} */

#endif//__INTROS_TSK_H
