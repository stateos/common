/******************************************************************************

    @file    StateOS: ostask.h
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

#ifndef __STATEOS_TSK_H
#define __STATEOS_TSK_H

#include "oskernel.h"
#include "osclock.h"
#include "osmutex.h"
#include "ostimer.h"
#include "oscriticalsection.h"

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

typedef union __tmp tmp_t;

union __tmp
{
	struct {
	unsigned event;
	}        evt;   // temporary data used by event object

	struct {
	unsigned sigset;
	unsigned signo;
	}        sig;   // temporary data used by signal object

	struct {
	unsigned flags;
	unsigned mode;
	}        flg;   // temporary data used by flag object

	struct {
	void   * data;
	}        lst;   // temporary data used by list / memory pool object

	struct {
	union  {
	const
	char   * out;
	char   * in;
	}        data;
	size_t   size;
	}        raw;   // temporary data used by raw buffer object

	struct {
	union  {
	const
	char   * out;
	char   * in;
	}        data;
	size_t   size;
	}        msg;   // temporary data used by message buffer object

	struct {
	union  {
	const
	void   * out;
	void   * in;
	}        data;
	}        box;   // temporary data used by mailbox queue object

	struct {
	unsigned event;
	}        evq;   // temporary data used by event queue object

	struct {
	fun_t  * fun;
	}        job;   // temporary data used by job queue object
};

struct __tsk
{
	obj_t    obj;   ///< object header
	hdr_t    hdr;   ///< timer / task header

	fun_t  * proc;  ///< task proc (initial task function, doesn't have to be noreturn-type)
	void   * arg;   ///< argument of the task function
	cnt_t    start; ///< inherited from timer
	cnt_t    delay; ///< inherited from timer
	cnt_t    slice;	///< time slice

	stk_t  * stack; ///< base of stack
	size_t   size;  ///< size of stack (in bytes)
	void   * sp;    ///< current stack pointer

	unsigned basic; ///< basic priority
	unsigned prio;  ///< current priority

	tsk_t  * owner; ///< task owner (joinable / detached state)
	tsk_t ** guard; ///< BLOCKED queue for the pending process
	int      event; ///< wakeup event

	struct {
	mtx_t  * list;  ///< list of mutexes held
	mtx_t  * tree;  ///< tree of tasks waiting for mutexes
	}        mtx;

	struct {
	unsigned sigset;///< pending signals
	act_t  * action;///< signal handler
	unsigned run;   ///< number of signal handlers being executed (nested)
	struct {
	void   * sp;    ///< stack pointer of the interrupted context
	int      event; ///< wakeup event of the interrupted context
	cnt_t    start; ///< timer data of the interrupted context
	cnt_t    delay; ///< timer data of the interrupted context
	tmp_t    temp;  ///< temporary data of the interrupted context
	}        backup;
	}        sig;

	tmp_t    temp;  ///< temporary data used by blocking functions

#ifndef _PORT_DATA_INIT
#define _PORT_DATA_INIT()
#else
	    _PORT_DATA();
#endif
};

typedef struct __tsk tsk_id [];

/******************************************************************************
 *
 * Name              : _TSK_INIT
 *
 * Description       : create and initialize a task object
 *
 * Parameters
 *   prio            : initial task priority (any unsigned int value)
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

#define               _TSK_INIT( _prio, _proc, _stack, _size )                                                            \
                       { _OBJ_INIT(), _HDR_INIT(), _proc, NULL, 0, 0, 0, _stack, _size, NULL, _prio, _prio, NULL, NULL, 0, \
                       { NULL, NULL }, { 0, NULL, 0, { NULL, 0, 0, 0, { { 0 } } } }, { { 0 } }, _PORT_DATA_INIT() }

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
 * @param   prio       initial task priority (any unsigned int value)
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   size       size of task private stack (in bytes)
 */

#define             OS_WRK( tsk, prio, proc, size )                       \
                static stk_t tsk##__stk[STK_SIZE( size )] __STKALIGN; \
                       tsk_t tsk[] = { _TSK_INIT( prio, proc, tsk##__stk, STK_OVER( size ) ) }

/** static alias of OS_WRK() */
#define         static_WRK( tsk, prio, proc, size )                       \
                static stk_t tsk##__stk[STK_SIZE( size )] __STKALIGN; \
                static tsk_t tsk[] = { _TSK_INIT( prio, proc, tsk##__stk, STK_OVER( size ) ) }

/**
 * @brief   define and initialize complete work area for task object with default stack size
 *
 * @param   tsk        name of a pointer to task object
 * @param   prio       initial task priority (any unsigned int value)
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   ...        size: (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 */

#define             OS_TSK( tsk, prio, proc, ... ) \
                    OS_WRK( tsk, prio, proc, _VA_STK(__VA_ARGS__) )

/** static alias of OS_TSK() */
#define         static_TSK( tsk, prio, proc, ... ) \
                static_WRK( tsk, prio, proc, _VA_STK(__VA_ARGS__) )

/**
 * @brief   define and initialize complete work area for task object
 *
 *          task proc (function body) must be defined immediately below
 *
 * @param   tsk        name of a pointer to task object
 * @param   prio       initial task priority (any unsigned int value)
 * @param   size       size of task private stack (in bytes)
 */

#define             OS_WRK_DEF( tsk, prio, size )        \
                static void tsk##__fun( void );           \
                    OS_WRK( tsk, prio, tsk##__fun, size ); \
                static void tsk##__fun( void )

/** static alias of OS_WRK_DEF() */
#define         static_WRK_DEF( tsk, prio, size )        \
                static void tsk##__fun( void );           \
                static_WRK( tsk, prio, tsk##__fun, size ); \
                static void tsk##__fun( void )

/**
 * @brief   define and initialize complete work area for task object with default stack size
 *
 *          task proc (function body) must be defined immediately below
 *
 * @param   tsk        name of a pointer to task object
 * @param   prio       initial task priority (any unsigned int value)
 * @param   ...        size: (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 */

#define             OS_TSK_DEF( tsk, prio, ... ) \
                    OS_WRK_DEF( tsk, prio, _VA_STK(__VA_ARGS__) )

/** static alias of OS_TSK_DEF() */
#define         static_TSK_DEF( tsk, prio, ... ) \
                static_WRK_DEF( tsk, prio, _VA_STK(__VA_ARGS__) )

/**
 * @brief   define, initialize and start complete work area for task object
 *
 *          task proc (function body) must be defined immediately below
 *
 * @param   tsk        name of a pointer to task object
 * @param   prio       initial task priority (any unsigned int value)
 * @param   size       size of task private stack (in bytes)
 *
 * @note    only available for compilers supporting the "constructor" function attribute or its equivalent
 */

#ifdef __CONSTRUCTOR
#define             OS_WRK_START( tsk, prio, size )                         \
                static void tsk##__fun( void );                              \
                    OS_WRK( tsk, prio, tsk##__fun, size );                    \
  __CONSTRUCTOR static void tsk##__run( void ) { sys_init(); tsk_start(tsk); } \
                static void tsk##__fun( void )
#endif

#ifdef __CONSTRUCTOR
/** static alias of OS_WRK_START() */
#define         static_WRK_START( tsk, prio, size )                         \
                static void tsk##__fun( void );                              \
                static_WRK( tsk, prio, tsk##__fun, size );                    \
  __CONSTRUCTOR static void tsk##__run( void ) { sys_init(); tsk_start(tsk); } \
                static void tsk##__fun( void )
#endif

/**
 * @brief   define, initialize and start complete work area for task object with default stack size
 *
 *          task proc (function body) must be defined immediately below
 *
 * @param   tsk        name of a pointer to task object
 * @param   prio       initial task priority (any unsigned int value)
 * @param   ...        size: (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 *
 * @note    only available for compilers supporting the "constructor" function attribute or its equivalent
 */

#ifdef __CONSTRUCTOR
#define             OS_TSK_START( tsk, prio, ... ) \
                    OS_WRK_START( tsk, prio, _VA_STK(__VA_ARGS__) )
#endif

#ifdef __CONSTRUCTOR
/** static alias of OS_TSK_START() */
#define         static_TSK_START( tsk, prio, ... ) \
                static_WRK_START( tsk, prio, _VA_STK(__VA_ARGS__) )
#endif

/**
 * @brief   create and initialize complete work area for task object
 *
 * @param   prio       initial task priority (any unsigned int value)
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   size       size of task private stack (in bytes)
 *
 * @return  task object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                WRK_INIT( prio, proc, size ) \
                      _TSK_INIT( prio, proc, _TSK_STACK( size ), STK_OVER( size ) )
#endif

/**
 * @brief   create and initialize complete work area for task object
 *
 * @param   prio       initial task priority (any unsigned int value)
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   size       size of task private stack (in bytes)
 *
 * @return  task object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                WRK_CREATE( prio, proc, size ) \
                     { WRK_INIT  ( prio, proc, size ) }
/** alias of WRK_CREATE() */
#define                WRK_NEW \
                       WRK_CREATE
#endif

/**
 * @brief   create and initialize complete work area for task object with default stack size
 *
 * @param   prio       initial task priority (any unsigned int value)
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   ...        size: (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 *
 * @return  task object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                TSK_INIT( prio, proc, ... ) \
                       WRK_INIT( prio, proc, _VA_STK(__VA_ARGS__) )
#endif

/**
 * @brief   create and initialize complete work area for task object with default stack size
 *
 * @param   prio       initial task priority (any unsigned int value)
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   ...        size: (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 *
 * @return  task object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                TSK_CREATE( prio, proc, ... ) \
                       WRK_CREATE( prio, proc, _VA_STK(__VA_ARGS__) )
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
 *
 * @note    use only in thread mode
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
 * @param   prio       initial task priority (any unsigned int value)
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   stack      base of task's private stack storage
 * @param   size       size of task private stack (in bytes)
 *
 * @note    use only in thread mode
 */

void wrk_init( tsk_t *tsk, unsigned prio, fun_t *proc, stk_t *stack, size_t size );

/**
 * @brief   initialize complete work area for task object
 *          and start the task, if proc != NULL
 *
 * @param   tsk        pointer to task object
 * @param   prio       initial task priority (any unsigned int value)
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   stack      base of task's private stack storage
 * @param   size       size of task private stack (in bytes)
 *
 * @note    use only in thread mode
 */

void tsk_init( tsk_t *tsk, unsigned prio, fun_t *proc, stk_t *stack, size_t size );

/**
 * @brief   create and initialize complete work area for task object
 *
 * @param   prio       initial task priority (any unsigned int value)
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 * @param   size       size of task private stack (in bytes)
 * @param   detached   create as detached?
 * @param   autostart  specifies whether the task runs immediately after initialization
 *
 * @return  pointer to task object
 * @retval  NULL       object not created (not enough free memory)
 *
 * @note    use only in thread mode
 */

tsk_t *wrk_create( unsigned prio, fun_t *proc, size_t size, bool detached, bool autostart );

/** alias of wrk_create() */
__STATIC_INLINE
tsk_t *wrk_new( unsigned prio, fun_t *proc, size_t size, bool detached, bool autostart )
{
	return wrk_create(prio, proc, size, detached, autostart);
}

/**
 * @brief   create and initialize complete work area for task object
 *          and start the task, if proc != NULL
 *
 * @param   prio       initial task priority (any unsigned int value)
 * @param   proc       task initial procedure (initial task function with argument)
 * @param   arg        task initial procedure argument
 * @param   size       size of task private stack (in bytes)
 *
 * @return  pointer to task object
 * @retval  NULL       object not created (not enough free memory)
 *
 * @note    use only in thread mode
 * @note    tsk_startFrom and tsk_flip clear the argument
 */

tsk_t *tsk_setup( unsigned prio, fun_a *proc, void *arg, size_t size );

/**
 * @brief   create and initialize a new task object with default stack size
 *          and start the task, if proc != NULL
 *
 * @param   prio       initial task priority (any unsigned int value)
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *
 * @return  pointer to task object
 * @retval  NULL       object not created (not enough free memory)
 *
 * @note    use only in thread mode
 */

__STATIC_INLINE
tsk_t *tsk_create( unsigned prio, fun_t *proc )
{
	return wrk_create(prio, proc, OS_STACK_SIZE, false, !!proc);
}

/** alias of tsk_create() */
__STATIC_INLINE
tsk_t *tsk_new( unsigned prio, fun_t *proc )
{
	return wrk_create(prio, proc, OS_STACK_SIZE, false, !!proc);
}

/**
 * @brief   create and initialize a new detached task object with default stack size
 *          and start the task, if proc != NULL
 *
 * @param   prio       initial task priority (any unsigned int value)
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *
 * @return  pointer to task object
 * @retval  NULL       object not created (not enough free memory)
 *
 * @note    use only in thread mode
 */

__STATIC_INLINE
tsk_t *tsk_detached( unsigned prio, fun_t *proc )
{
	return wrk_create(prio, proc, OS_STACK_SIZE, true, !!proc);
}

/**
 * @brief   start previously defined/created/stopped task object
 *
 * @param   tsk        pointer to task object
 *
 * @note    use only in thread mode
 */

void tsk_start( tsk_t *tsk );

/**
 * @brief   start previously defined/created/stopped task object
 *
 * @param   tsk        pointer to task object
 * @param   proc       task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *
 * @note    use only in thread mode
 */

void tsk_startFrom( tsk_t *tsk, fun_t *proc );

/**
 * @brief   start previously defined/created/stopped task object
 *
 * @param   tsk        pointer to task object
 * @param   proc       task initial procedure (initial task function with argument)
 * @param   arg        task initial procedure argument
 *
 * @note    use only in thread mode
 * @note    tsk_startFrom and tsk_flip clear the argument
 */

void tsk_startWith( tsk_t *tsk, fun_a *proc, void *arg );

/**
 * @brief   detach given task
 *
 * @param   tsk        pointer to task object
 *
 * @retval  E_SUCCESS  given task was successfully detached
 * @retval  E_FAILURE  given task cannot be detached
 *
 * @note    use only in thread mode
 */

int tsk_detach( tsk_t *tsk );

/**
 * @brief   delay execution of current task until termination of given task
 *
 * @param   tsk        pointer to task object
 *
 * @retval  E_SUCCESS  given task has stopped (stop / exit)
 * @retval  E_STOPPED  given task was reseted (reset / kill)
 * @retval  E_DELETED  given task was deleted (delete / destroy)
 * @retval  E_FAILURE  given task cannot be joined
 *
 * @note    use only in thread mode
 */

int tsk_join( tsk_t *tsk );

/**
 * @brief   stop the current task and remove it from READY queue
 *
 *          all mutexes held by the task are released (waiting tasks get E_STOPPED, robust mutex: next owner gets OWNERDEAD)
 *
 *          detached task is destroyed, otherwise
 *          function doesn't destroy the stack storage,
 *          and all allocated resources will remain intact until
 *          joining, destroying or restarting the task
 *
 * @note    use only in thread mode
 */

__NO_RETURN
void tsk_stop( void );

/** alias of tsk_stop() */
__STATIC_INLINE
void tsk_exit( void ) { tsk_stop(); }

/**
 * @brief   reset the task object and remove it from READY/BLOCKED queue
 *
 *          all mutexes held by the task are released (waiting tasks get E_STOPPED, robust mutex: next owner gets OWNERDEAD)
 *
 *          other objects held by the task (e.g. semaphores, read/write locks) are not released
 *
 * @param   tsk        pointer to task object
 *
 * @retval  E_SUCCESS  given task was already inactive or has been reseted
 * @retval  E_FAILURE  given task cannot be reseted
 *
 * @note    use only in thread mode
 * @note    readers waiting behind a writer stopped while waiting for a read/write lock
 *          are not released until the lock is given
 */

int tsk_reset( tsk_t *tsk );

/** alias of tsk_reset() */
__STATIC_INLINE
int tsk_kill( tsk_t *tsk ) { return tsk_reset(tsk); }

/**
 * @brief   reset the task object and free allocated resources
 *
 *          all mutexes held by the task are released (waiting tasks get E_STOPPED, robust mutex: next owner gets OWNERDEAD)
 *
 * @param   tsk        pointer to task object
 *
 * @retval  E_SUCCESS  given task has been destroyed
 * @retval  E_FAILURE  given task cannot be destroyed
 *
 * @note    use only in thread mode
 */

int tsk_destroy( tsk_t *tsk );

/** alias of tsk_destroy() */
__STATIC_INLINE
int tsk_delete( tsk_t *tsk ) { return tsk_destroy(tsk); }

/**
 * @brief   yield system control to the next task with the same priority in READY queue
 *
 *          the current task goes to the end of its priority group (also when a higher priority task is ready)
 *
 * @note    use only in thread mode
 */

void tsk_yield( void );

/** alias of tsk_yield() */
__STATIC_INLINE
void tsk_pass ( void ) { tsk_yield(); }

/**
 * @brief   restart current task with the new proc (task function)
 *
 *          all mutexes held by the task are released (waiting tasks get E_STOPPED, robust mutex: next owner gets OWNERDEAD)
 *
 * @param   proc       new task proc (task function)
 *
 * @note    use only in thread mode
 */

__NO_RETURN
void tsk_flip( fun_t *proc );

/**
 * @brief   set current task priority
 *
 * @param   prio       new task priority value
 *
 * @note    use only in thread mode
 */

void tsk_setPrio( unsigned prio );

/** alias of tsk_setPrio() */
__STATIC_INLINE
void tsk_prio( unsigned prio ) { tsk_setPrio(prio); }

/**
 * @brief   get current task priority
 *
 * @return  current task priority value
 *
 * @note    use only in thread mode
 */

unsigned tsk_getPrio( void );

/**
 * @brief   delay execution of current task for given duration of time
 *
 * @param   delay      duration of time (maximum number of ticks to delay execution of current task)
 *                     - IMMEDIATE: don't delay execution of current task (cooperative mode: only give up the processor once)
 *                     - INFINITE:  delay indefinitely execution of current task
 *
 * @note    use only in thread mode
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
 *                     - IMMEDIATE: don't delay execution of current task (cooperative mode: only give up the processor once)
 *                     - INFINITE:  delay indefinitely execution of current task
 *
 * @note    use only in thread mode
 */

void tsk_sleepNext( cnt_t delay );

/**
 * @brief   delay execution of current task until given timepoint
 *
 * @param   time       timepoint value
 *                     - past timepoint: don't delay execution of current task (cooperative mode: only give up the processor once)
 *
 * @note    use only in thread mode
 */

void tsk_sleepUntil( cnt_t time );

/**
 * @brief   delay indefinitely execution of current task
 *
 *          execution of the task can be resumed
 *
 * @note    use only in thread mode
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
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

int tsk_resume( tsk_t *tsk );

/** ISR alias of tsk_resume() */
__STATIC_INLINE
int tsk_resumeISR( tsk_t *tsk ) { return tsk_resume(tsk); }

/**
 * @brief   send given signal to the task
 *
 * @param   tsk        pointer to the task object
 * @param   signo      signal number
 *
 * @note    use only in thread mode
 * @note    the signal handler of a blocked task is called after the task is woken up
 *          (the task does not leave the blocked queue, so no wakeup can be lost)
 * @note    the signal handler is called on the stack of the task; the stack must have
 *          room for an additional task context (and another one, if the handler
 *          calls blocking functions)
 * @note    signals sent by the task to itself are handled immediately, also inside
 *          a critical section of the caller (the handler is called with unlocked
 *          interrupts); if such a handler is preempted and the task receives another
 *          signal, the handler is called again (nested), so it should be reentrant
 */

void tsk_give( tsk_t *tsk, unsigned signo );

/** alias of tsk_give() */
__STATIC_INLINE
void tsk_signal( tsk_t *tsk, unsigned signo ) { tsk_give(tsk, signo); }

/**
 * @brief   set given function as a signal handler
 *
 * @param   tsk        pointer to the task object
 * @param   action     signal handler
 *
 * @note    use only in thread mode
 * @note    the signal handler of a blocked task is called after the task is woken up
 *          (the task does not leave the blocked queue, so no wakeup can be lost)
 * @note    the signal handler is called on the stack of the task; the stack must have
 *          room for an additional task context (and another one, if the handler
 *          calls blocking functions)
 * @note    signals sent by the task to itself are handled immediately, also inside
 *          a critical section of the caller (the handler is called with unlocked
 *          interrupts); if such a handler is preempted and the task receives another
 *          signal, the handler is called again (nested), so it should be reentrant
 */

void tsk_action( tsk_t *tsk, act_t *action );

/**
 * @brief   chack water mark of the stack of the current task
 *
 * @return  high water mark of the stack of the current task
 * @retval  0          DEBUG not defined
 */

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

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(__STATEOS_STDCXX)
namespace stateos {

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
	static_assert(size_>sizeof(ctx_t), "incorrect stack size");
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
 *   prio            : initial task priority (any unsigned int value)
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
	baseTask( const unsigned _prio, F&&     _proc, stk_t * const _stack, const size_t _size ): __tsk _TSK_INIT(_prio, fun_, _stack, _size), fun{_proc} {}
#else
	baseTask( const unsigned _prio, fun_t * _proc, stk_t * const _stack, const size_t _size ): __tsk _TSK_INIT(_prio, _proc, _stack, _size) {}
#endif

	void     start    ()                   {        tsk_start    (this); }
#if __cplusplus >= 201402L
	template<class F>
	void     startFrom( F&&      _proc )
	{
		Fun_t tmp(_proc);
		{
			CriticalSection cs;
			if (__tsk::hdr.id == ID_STOPPED)  // active tasks cannot be started
			{
				fun.swap(tmp);
				tsk_startFrom(this, fun_);
			}
		}                                     // the replaced function is destroyed outside the critical section
	}
#else
	void     startFrom( fun_t  * _proc )   {        tsk_startFrom(this, _proc); }
#endif
	int      detach   ()                   { return tsk_detach   (this); }
	int      join     ()                   { return tsk_join     (this); }
	int      reset    ()                   { return tsk_reset    (this); }
	int      kill     ()                   { return tsk_kill     (this); }
	int      destroy  ()                   { return tsk_destroy  (this); }
	unsigned prio     ()                   { return __tsk::basic; }
	unsigned getPrio  ()                   { return __tsk::basic; }
	int      suspend  ()                   { return tsk_suspend  (this); }
	int      resume   ()                   { return tsk_resume   (this); }
	int      resumeISR()                   { return tsk_resumeISR(this); }
	void     give     ( unsigned _signo )  {        tsk_give     (this, _signo); }
	void     signal   ( unsigned _signo )  {        tsk_signal   (this, _signo); }
#if __cplusplus >= 201402L
	template<class F>
	void     action   ( F&&      _action )
	{
		Act_t tmp(_action);
		Act_t old;
		{
			CriticalSection cs;
			if (__tsk::sig.run == 0)          // the action is not being executed:
			{
				act.swap(tmp);                // replace it now
				nxt.swap(old);                // and drop the pending one
			}
			else                              // the action is being executed (it can replace itself):
				nxt.swap(tmp);                // replace it after the execution
			tsk_action(this, act_);
		}                                     // the replaced actions are destroyed outside the critical section
	}
#else
	void     action   ( act_t *  _action ) {        tsk_action   (this, _action); }
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
	void     act_     ( unsigned _signo )
	{
		auto tsk  = current();
		auto next = [tsk]                     // use the pending action
		{
			Act_t old;
			{
				CriticalSection cs;
				if (tsk->nxt && tsk->sig.run == 1) // no execution of the action is in progress (except the caller, outside of the action)
				{
					old.swap(tsk->act);
					tsk->act.swap(tsk->nxt);
				}
			}                                 // the replaced action is destroyed outside the critical section
		};
		next();                               // replaced during the previous execution (ended by tsk_flip, tsk_stop or tsk_reset)
		if (tsk->act)
			tsk->act(_signo);
		next();                               // replaced during this execution
	}
	Act_t    act;
	Act_t    nxt;                             // the action replaced during its execution
#endif

/**
 * @brief   provide set of functions for current task
 */

	struct Current
	{
		static
		int      detach    ()                   { return tsk_detach    (current()); }
		static
		void     stop      ()                   {        tsk_stop      (); }
		static
		void     exit      ()                   {        tsk_exit      (); }
		static
		int      reset     ()                   { return tsk_reset     (current()); }
		static
		int      kill      ()                   { return tsk_kill      (current()); }
		static
		int      destroy   ()                   { return tsk_destroy   (current()); }
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
		static
		void     setPrio   ( unsigned _prio )   {        tsk_setPrio   (_prio); }
		static
		void     prio      ( unsigned _prio )   {        tsk_prio      (_prio); }
		static
		unsigned getPrio   ()                   { return tsk_getPrio   (); }
		static
		unsigned prio      ()                   { return tsk_getPrio   (); }
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
		static
		void     give      ( unsigned _signo )  {        tsk_give      (current(), _signo); }
		static
		void     signal    ( unsigned _signo )  {        tsk_signal    (current(), _signo); }
#if __cplusplus >= 201402L
		template<class F> static
		void     action    ( F&&      _action ) {        current()->action(std::forward<F>(_action)); }
#else
		static
		void     action    ( act_t  * _action ) {        tsk_action    (current(), _action); }
#endif
	};
};

using thisTask = baseTask::Current;
using this_task = baseTask::Current;

/**
 * @brief   create and initialize complete work area for task object
 *
 * @par     Constructor parameters
 *          - size: size of task private stack (in bytes)
 *          - prio: initial task priority (any unsigned int value)
 *          - proc: task proc (initial task function) doesn't have to be noreturn-type
 *            it will be executed into an infinite system-implemented loop
 */

template<size_t size_>
struct TaskT : public baseTask, public baseStack<size_>
{
	/// constructor, see the constructor parameters of the class
	template<class F>
	TaskT( const unsigned _prio, F&& _proc ):
	baseTask{_prio, _proc, baseStack<size_>::stack_, sizeof(baseStack<size_>::stack_)} {}

	/// constructor, see the constructor parameters of the class
	template<class F>
	TaskT( F&& _proc ):
	TaskT<size_>{OS_MAIN_PRIO, _proc} {}

#if __cplusplus >= 201402L
	/// constructor, see the constructor parameters of the class
	template<typename F, typename... A>
	TaskT( const unsigned _prio, F&& _proc, A&&... _args ):
	TaskT<size_>{_prio, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...)} {}

#if __cplusplus >= 201703L && !defined(__ICCARM__)
	/// constructor, see the constructor parameters of the class
	template<typename F, typename... A, typename = std::enable_if_t<std::is_invocable_v<F, A...>>>
	TaskT( F&& _proc, A&&... _args ):
	TaskT<size_>{std::bind(std::forward<F>(_proc), std::forward<A>(_args)...)} {}
#endif
#endif

	~TaskT() { assert(__tsk::hdr.id == ID_STOPPED); }

#if __cplusplus >= 201402L
	/// move constructor
	TaskT( TaskT<size_>&& _src ):
	baseTask{_src.basic, _src.fun, baseStack<size_>::stack_, sizeof(baseStack<size_>::stack_)}
	{
		assert(_src.hdr.id == ID_STOPPED);
		act = _src.act;                      // the signal action set before the task was stopped
		nxt = _src.nxt;
		__tsk::sig.action = _src.sig.action;
	}
#else
	/// move constructor
	TaskT( TaskT<size_>&& _src ):
	baseTask{_src.basic, _src.proc, baseStack<size_>::stack_, sizeof(baseStack<size_>::stack_)}
	{
		assert(_src.hdr.id == ID_STOPPED);
		__tsk::sig.action = _src.sig.action; // the signal action set before the task was stopped
	}
#endif

	TaskT( const TaskT<size_>& ) = delete;
	TaskT<size_>& operator=( TaskT<size_>&& ) = delete;
	TaskT<size_>& operator=( const TaskT<size_>& ) = delete;

private:
	struct Starter {}; // Start functions return the task started in place: a started task must not be moved
	template<typename... A>
	TaskT( Starter, A&&... _args ): TaskT<size_>{static_cast<A&&>(_args)...} { this->start(); }
public:

/**
 * @brief   create and initialize static undetachable task
 *
 * @par     Parameters
 *          - size: size of task private stack (in bytes)
 *          - prio: initial task priority (any unsigned int value)
 *          - proc: task proc (initial task function) doesn't have to be noreturn-type
 *            it will be executed into an infinite system-implemented loop
 *          - args: arguments for task proc
 *
 * @return  TaskT<> object
 */

	template<class F> static
	TaskT<size_> Make( const unsigned _prio, F&& _proc )
	{
		return { _prio, _proc };
	}

	/** @overload */
	template<class F> static
	TaskT<size_> Make( F&& _proc )
	{
		return Make(OS_MAIN_PRIO, _proc);
	}

#if __cplusplus >= 201402L
	/** @overload */
	template<typename F, typename... A> static
	TaskT<size_> Make( const unsigned _prio, F&& _proc, A&&... _args )
	{
		return Make(_prio, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}

#if __cplusplus >= 201703L && !defined(__ICCARM__)
	/** @overload */
	template<typename F, typename... A, typename = std::enable_if_t<std::is_invocable_v<F, A...>>> static
	TaskT<size_> Make( F&& _proc, A&&... _args )
	{
		return Make(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif
#endif

/**
 * @brief   create, initialize and start static undetachable task
 *
 * @par     Parameters
 *          - size: size of task private stack (in bytes)
 *          - prio: initial task priority (any unsigned int value)
 *          - proc: task proc (initial task function) doesn't have to be noreturn-type
 *            it will be executed into an infinite system-implemented loop
 *          - args: arguments for task proc
 *
 * @return  TaskT<> object
 */

	template<class F> static
	TaskT<size_> Start( const unsigned _prio, F&& _proc )
	{
		return { Starter{}, _prio, _proc };
	}

	/** @overload */
	template<class F> static
	TaskT<size_> Start( F&& _proc )
	{
		return Start(OS_MAIN_PRIO, _proc);
	}

#if __cplusplus >= 201402L
	/** @overload */
	template<typename F, typename... A> static
	TaskT<size_> Start( const unsigned _prio, F&& _proc, A&&... _args )
	{
		return Start(_prio, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}

#if __cplusplus >= 201703L && !defined(__ICCARM__)
	/** @overload */
	template<typename F, typename... A, typename = std::enable_if_t<std::is_invocable_v<F, A...>>> static
	TaskT<size_> Start( F&& _proc, A&&... _args )
	{
		return Start(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif
#endif

#if __cplusplus >= 201402L
	using Deleter = struct _Deleter { void operator()( void* ) const {} }; ///< deleter of Ptr (does not delete the task object)
	using Ptr = std::unique_ptr<TaskT<size_>, Deleter>; ///< result type of Create()
#else
	using Ptr = TaskT<size_> *; ///< result type of Create()
#endif

/**
 * @brief   create, initialize and start dynamic joinable task
 *
 *          with manageable resources
 *
 * @par     Parameters
 *          - size: size of task private stack (in bytes)
 *          - prio: initial task priority (any unsigned int value)
 *          - proc: task proc (initial task function) doesn't have to be noreturn-type
 *            it will be executed into an infinite system-implemented loop
 *          - args: arguments for task proc
 *
 * @return  std::unique_pointer / pointer to dynamic joinable TaskT<> object
 *
 * @note    use only in thread mode
 */

	template<class F> static
	Ptr Create( const unsigned _prio, F&& _proc )
	{
		auto tsk = new (std::nothrow) TaskT<size_>(_prio, _proc);
		if (tsk != nullptr)
		{
			tsk->__tsk::obj.res = tsk;
			tsk->start();
		}
		return Ptr(tsk);
	}

	/** @overload */
	template<class F> static
	Ptr Create( F&& _proc )
	{
		return Create(OS_MAIN_PRIO, _proc);
	}

#if __cplusplus >= 201402L
	/** @overload */
	template<typename F, typename... A> static
	Ptr Create( const unsigned _prio, F&& _proc, A&&... _args )
	{
		return Create(_prio, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}

#if __cplusplus >= 201703L && !defined(__ICCARM__)
	/** @overload */
	template<typename F, typename... A, typename = std::enable_if_t<std::is_invocable_v<F, A...>>> static
	Ptr Create( F&& _proc, A&&... _args )
	{
		return Create(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif
#endif

/**
 * @brief   create, initialize and start dynamic detached task
 *
 *          with manageable resources
 *
 * @par     Parameters
 *          - size: size of task private stack (in bytes)
 *          - prio: initial task priority (any unsigned int value)
 *          - proc: task proc (initial task function) doesn't have to be noreturn-type
 *            it will be executed into an infinite system-implemented loop
 *          - args: arguments for task proc
 *
 * @return  std::unique_pointer / pointer to dynamic detached TaskT<> object
 *
 * @note    use only in thread mode
 */

	template<class F> static
	Ptr Detached( const unsigned _prio, F&& _proc )
	{
		auto tsk = new (std::nothrow) TaskT<size_>(_prio, _proc);
		if (tsk != nullptr)
		{
			tsk->__tsk::obj.res = tsk;
			tsk->__tsk::owner = tsk;
			tsk->start();
		}
		return Ptr(tsk);
	}

	/** @overload */
	template<class F> static
	Ptr Detached( F&& _proc )
	{
		return Detached(OS_MAIN_PRIO, _proc);
	}

#if __cplusplus >= 201402L
	/** @overload */
	template<typename F, typename... A> static
	Ptr Detached( const unsigned _prio, F&& _proc, A&&... _args )
	{
		return Detached(_prio, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}

#if __cplusplus >= 201703L && !defined(__ICCARM__)
	/** @overload */
	template<typename F, typename... A, typename = std::enable_if_t<std::is_invocable_v<F, A...>>> static
	Ptr Detached( F&& _proc, A&&... _args )
	{
		return Detached(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif
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

#endif//__STATEOS_TSK_H
