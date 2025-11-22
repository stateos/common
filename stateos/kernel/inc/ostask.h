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
	obj_t    obj;   // object header
	hdr_t    hdr;   // timer / task header

	fun_t  * proc;  // task proc (initial task function, doesn't have to be noreturn-type)
	void   * arg;   // reserved for internal use
	cnt_t    start; // inherited from timer
	cnt_t    delay; // inherited from timer
	cnt_t    slice;	// time slice

	stk_t  * stack; // base of stack
	size_t   size;  // size of stack (in bytes)
	void   * sp;    // current stack pointer

	unsigned basic; // basic priority
	unsigned prio;  // current priority

	tsk_t  * owner; // task owner (joinable / detached state)
	tsk_t ** guard; // BLOCKED queue for the pending process
	int      event; // wakeup event

	struct {
	mtx_t  * list;  // list of mutexes held
	mtx_t  * tree;  // tree of tasks waiting for mutexes
	}        mtx;

	struct {
	unsigned sigset;// pending signals
	act_t  * action;// signal handler
	struct {
	void   * sp;
	tsk_t ** guard;
	}        backup;
	}        sig;

	union  {

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

	}        tmp;

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
                       { NULL, NULL }, { 0, NULL, { NULL, NULL } }, { { 0 } }, _PORT_DATA_INIT() }

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
 *   prio            : initial task priority (any unsigned int value)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   size            : size of task private stack (in bytes)
 *
 ******************************************************************************/

#define             OS_WRK( tsk, prio, proc, size )                       \
                static stk_t tsk##__stk[STK_SIZE( size )] __STKALIGN; \
                       tsk_t tsk[] = { _TSK_INIT( prio, proc, tsk##__stk, STK_OVER( size ) ) }

#define         static_WRK( tsk, prio, proc, size )                       \
                static stk_t tsk##__stk[STK_SIZE( size )] __STKALIGN; \
                static tsk_t tsk[] = { _TSK_INIT( prio, proc, tsk##__stk, STK_OVER( size ) ) }

/******************************************************************************
 *
 * Name              : OS_TSK
 * Static alias      : static_TSK
 *
 * Description       : define and initialize complete work area for task object with default stack size
 *
 * Parameters
 *   tsk             : name of a pointer to task object
 *   prio            : initial task priority (any unsigned int value)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   size            : (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 *
 ******************************************************************************/

#define             OS_TSK( tsk, prio, proc, ... ) \
                    OS_WRK( tsk, prio, proc, _VA_STK(__VA_ARGS__) )

#define         static_TSK( tsk, prio, proc, ... ) \
                static_WRK( tsk, prio, proc, _VA_STK(__VA_ARGS__) )

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
 *   prio            : initial task priority (any unsigned int value)
 *   size            : size of task private stack (in bytes)
 *
 ******************************************************************************/

#define             OS_WRK_DEF( tsk, prio, size )        \
                static void tsk##__fun( void );           \
                    OS_WRK( tsk, prio, tsk##__fun, size ); \
                static void tsk##__fun( void )

#define         static_WRK_DEF( tsk, prio, size )        \
                static void tsk##__fun( void );           \
                static_WRK( tsk, prio, tsk##__fun, size ); \
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
 *   prio            : initial task priority (any unsigned int value)
 *   size            : (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 *
 ******************************************************************************/

#define             OS_TSK_DEF( tsk, prio, ... ) \
                    OS_WRK_DEF( tsk, prio, _VA_STK(__VA_ARGS__) )

#define         static_TSK_DEF( tsk, prio, ... ) \
                static_WRK_DEF( tsk, prio, _VA_STK(__VA_ARGS__) )

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
 *   prio            : initial task priority (any unsigned int value)
 *   size            : size of task private stack (in bytes)
 *
 * Note              : only available for compilers supporting the "constructor" function attribute or its equivalent
 *
 ******************************************************************************/

#ifdef __CONSTRUCTOR
#define             OS_WRK_START( tsk, prio, size )                         \
                static void tsk##__fun( void );                              \
                    OS_WRK( tsk, prio, tsk##__fun, size );                    \
  __CONSTRUCTOR static void tsk##__run( void ) { sys_init(); tsk_start(tsk); } \
                static void tsk##__fun( void )
#endif

#ifdef __CONSTRUCTOR
#define         static_WRK_START( tsk, prio, size )                         \
                static void tsk##__fun( void );                              \
                static_WRK( tsk, prio, tsk##__fun, size );                    \
  __CONSTRUCTOR static void tsk##__run( void ) { sys_init(); tsk_start(tsk); } \
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
 *   prio            : initial task priority (any unsigned int value)
 *   size            : (optional) size of task private stack (in bytes); default: OS_STACK_SIZE
 *
 * Note              : only available for compilers supporting the "constructor" function attribute or its equivalent
 *
 ******************************************************************************/

#ifdef __CONSTRUCTOR
#define             OS_TSK_START( tsk, prio, ... ) \
                    OS_WRK_START( tsk, prio, _VA_STK(__VA_ARGS__) )
#endif

#ifdef __CONSTRUCTOR
#define         static_TSK_START( tsk, prio, ... ) \
                static_WRK_START( tsk, prio, _VA_STK(__VA_ARGS__) )
#endif

/******************************************************************************
 *
 * Name              : WRK_INIT
 *
 * Description       : create and initialize complete work area for task object
 *
 * Parameters
 *   prio            : initial task priority (any unsigned int value)
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
#define                WRK_INIT( prio, proc, size ) \
                      _TSK_INIT( prio, proc, _TSK_STACK( size ), STK_OVER( size ) )
#endif

/******************************************************************************
 *
 * Name              : WRK_CREATE
 * Alias             : WRK_NEW
 *
 * Description       : create and initialize complete work area for task object
 *
 * Parameters
 *   prio            : initial task priority (any unsigned int value)
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
#define                WRK_CREATE( prio, proc, size ) \
                     { WRK_INIT  ( prio, proc, size ) }
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
 *   prio            : initial task priority (any unsigned int value)
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
#define                TSK_INIT( prio, proc, ... ) \
                       WRK_INIT( prio, proc, _VA_STK(__VA_ARGS__) )
#endif

/******************************************************************************
 *
 * Name              : TSK_CREATE
 * Alias             : TSK_NEW
 *
 * Description       : create and initialize complete work area for task object with default stack size
 *
 * Parameters
 *   prio            : initial task priority (any unsigned int value)
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
#define                TSK_CREATE( prio, proc, ... ) \
                       WRK_CREATE( prio, proc, _VA_STK(__VA_ARGS__) )
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
 * Note              : use only in thread mode
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
 *   prio            : initial task priority (any unsigned int value)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   stack           : base of task's private stack storage
 *   size            : size of task private stack (in bytes)
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void wrk_init( tsk_t *tsk, unsigned prio, fun_t *proc, stk_t *stack, size_t size );

/******************************************************************************
 *
 * Name              : tsk_init
 *
 * Description       : initialize complete work area for task object
 *                     and start the task, if proc != NULL
 *
 * Parameters
 *   tsk             : pointer to task object
 *   prio            : initial task priority (any unsigned int value)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   stack           : base of task's private stack storage
 *   size            : size of task private stack (in bytes)
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void tsk_init( tsk_t *tsk, unsigned prio, fun_t *proc, stk_t *stack, size_t size );

/******************************************************************************
 *
 * Name              : wrk_create
 * Alias             : wrk_new
 *
 * Description       : create and initialize complete work area for task object
 *
 * Parameters
 *   prio            : initial task priority (any unsigned int value)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   size            : size of task private stack (in bytes)
 *   detached        : create as detached?
 *   autostart       : specifies whether the task runs immediately after initialization
 *
 * Return            : pointer to task object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

tsk_t *wrk_create( unsigned prio, fun_t *proc, size_t size, bool detached, bool autostart );

__STATIC_INLINE
tsk_t *wrk_new( unsigned prio, fun_t *proc, size_t size, bool detached, bool autostart )
{
	return wrk_create(prio, proc, size, detached, autostart);
}

/******************************************************************************
 *
 * Name              : tsk_setup
 *
 * Description       : create and initialize complete work area for task object
 *                     and start the task, if proc != NULL
 *
 * Parameters
 *   prio            : initial task priority (any unsigned int value)
 *   proc            : task initial procedure (initial task function)
 *   arg             : task initial procedure argument (for internal use)
 *   size            : size of task private stack (in bytes)
 *
 * Return            : pointer to task object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : for internal use
 *
 ******************************************************************************/

tsk_t *tsk_setup( unsigned prio, fun_a *proc, void *arg, size_t size );

/******************************************************************************
 *
 * Name              : tsk_create
 * Alias             : tsk_new
 *
 * Description       : create and initialize a new task object with default stack size
 *                     and start the task, if proc != NULL
 *
 * Parameters
 *   prio            : initial task priority (any unsigned int value)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *
 * Return            : pointer to task object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
tsk_t *tsk_create( unsigned prio, fun_t *proc )
{
	return wrk_create(prio, proc, OS_STACK_SIZE, false, !!proc);
}

__STATIC_INLINE
tsk_t *tsk_new( unsigned prio, fun_t *proc )
{
	return wrk_create(prio, proc, OS_STACK_SIZE, false, !!proc);
}

/******************************************************************************
 *
 * Name              : tsk_detached
 *
 * Description       : create and initialize a new detached task object with default stack size
 *                     and start the task, if proc != NULL
 *
 * Parameters
 *   prio            : initial task priority (any unsigned int value)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *
 * Return            : pointer to task object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
tsk_t *tsk_detached( unsigned prio, fun_t *proc )
{
	return wrk_create(prio, proc, OS_STACK_SIZE, true, !!proc);
}

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
 * Note              : use only in thread mode
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
 * Note              : use only in thread mode
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
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void tsk_startWith( tsk_t *tsk, fun_a *proc, void *arg );

/******************************************************************************
 *
 * Name              : tsk_detach
 *
 * Description       : detach given task
 *
 * Parameters
 *   tsk             : pointer to task object
 *
 * Return
 *   E_SUCCESS       : given task was successfully detached
 *   E_FAILURE       : given task cannot be detached
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int tsk_detach( tsk_t *tsk );

/******************************************************************************
 *
 * Name              : tsk_join
 *
 * Description       : delay execution of current task until termination of given task
 *
 * Parameters
 *   tsk             : pointer to task object
 *
 * Return
 *   E_SUCCESS       : given task has stopped (stop / exit)
 *   E_STOPPED       : given task was reseted (reset / kill)
 *   E_DELETED       : given task was deleted (delete / destroy)
 *   E_FAILURE       : given task cannot be joined
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int tsk_join( tsk_t *tsk );

/******************************************************************************
 *
 * Name              : tsk_stop
 * Alias             : tsk_exit
 *
 * Description       : stop the current task and remove it from READY queue
 *                     detached task is destroyed, otherwise
 *                     function doesn't destroy the stack storage,
 *                     and all allocated resources will remain intact until
 *                     joining, destroying or restarting the task
 *
 * Parameters        : none
 *
 * Return            : none
 *
 * Note              : use only in thread mode
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
 * Description       : reset the task object and remove it from READY/BLOCKED queue
 *
 * Parameters
 *   tsk             : pointer to task object
 *
 * Return
 *   E_SUCCESS       : given task was already inactive or has been reseted
 *   E_FAILURE       : given task cannot be reseted
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int tsk_reset( tsk_t *tsk );

__STATIC_INLINE
int tsk_kill( tsk_t *tsk ) { return tsk_reset(tsk); }

/******************************************************************************
 *
 * Name              : tsk_destroy
 * Alias             : tsk_delete
 *
 * Description       : reset the task object and free allocated resources
 *
 * Parameters
 *   tsk             : pointer to task object
 *
 * Return
 *   E_SUCCESS       : given task has been destroyed
 *   E_FAILURE       : given task cannot be destroyed
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int tsk_destroy( tsk_t *tsk );

__STATIC_INLINE
int tsk_delete( tsk_t *tsk ) { return tsk_destroy(tsk); }

/******************************************************************************
 *
 * Name              : tsk_yield
 * Alias             : tsk_pass
 *
 * Description       : yield system control to the next task with the same priority in READY queue
 *
 * Parameters        : none
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void tsk_yield( void );

__STATIC_INLINE
void tsk_pass ( void ) { tsk_yield(); }

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
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__NO_RETURN
void tsk_flip( fun_t *proc );

/******************************************************************************
 *
 * Name              : tsk_setPrio
 * Alias             : tsk_prio
 *
 * Description       : set current task priority
 *
 * Parameters
 *   prio            : new task priority value
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void tsk_setPrio( unsigned prio );

__STATIC_INLINE
void tsk_prio( unsigned prio ) { tsk_setPrio(prio); }

/******************************************************************************
 *
 * Name              : tsk_getPrio
 *
 * Description       : get current task priority
 *
 * Parameters        : none
 *
 * Return            : current task priority value
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

unsigned tsk_getPrio( void );

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
 * Note              : use only in thread mode
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
 * Note              : use only in thread mode
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
 * Note              : use only in thread mode
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
 * Note              : use only in thread mode
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
 *   E_SUCCESS       : task was successfully suspended
 *   E_FAILURE       : task cannot be suspended
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int tsk_suspend( tsk_t *tsk );

/******************************************************************************
 *
 * Name              : tsk_resume
 * ISR alias         : tsk_resumeISR
 *
 * Description       : resume execution of given suspended task
 *                     only suspended or indefinitely blocked tasks can be resumed
 *
 * Parameters
 *   tsk             : pointer to suspended task object
 *
 * Return
 *   E_SUCCESS       : task was successfully resumed
 *   E_FAILURE       : task cannot be resumed
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

int tsk_resume( tsk_t *tsk );

__STATIC_INLINE
int tsk_resumeISR( tsk_t *tsk ) { return tsk_resume(tsk); }

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
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void tsk_give( tsk_t *tsk, unsigned signo );

__STATIC_INLINE
void tsk_signal( tsk_t *tsk, unsigned signo ) { tsk_give(tsk, signo); }

/******************************************************************************
 *
 * Name              : tsk_action
 *
 * Description       : set given function as a signal handler
 *
 * Parameters
 *   tsk             : pointer to the task object
 *   action          : signal handler
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void tsk_action( tsk_t *tsk, act_t *action );

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

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
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
	void     startFrom( F&&      _proc )   {        new (&fun) Fun_t(_proc);
	                                                tsk_startFrom(this, fun_); }
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
	void     action   ( F&&      _action ) {        new (&act) Act_t(_action);
	                                                tsk_action   (this, act_); }
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
		void     flip      ( F&&      _proc )   {        new (&current()->fun) Fun_t(_proc);
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
		void     action    ( F&&      _action ) {        new (&current()->act) Act_t(_action);
		                                                 tsk_action    (current(), act_); }
#else
		static
		void     action    ( act_t  * _action ) {        tsk_action    (current(), _action); }
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
 *   prio            : initial task priority (any unsigned int value)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *
 ******************************************************************************/

template<size_t size_>
struct TaskT : public baseTask, public baseStack<size_>
{
	template<class F>
	TaskT( const unsigned _prio, F&& _proc ):
	baseTask{_prio, _proc, baseStack<size_>::stack_, sizeof(baseStack<size_>::stack_)} {}

	template<class F>
	TaskT( F&& _proc ):
	TaskT<size_>{OS_MAIN_PRIO, _proc} {}

#if __cplusplus >= 201402L
	template<typename F, typename... A>
	TaskT( const unsigned _prio, F&& _proc, A&&... _args ):
	TaskT<size_>{_prio, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...)} {}

#if __cplusplus >= 201703L && !defined(__ICCARM__)
	template<typename F, typename... A, typename = std::enable_if_t<std::is_invocable_v<F, A...>>>
	TaskT( F&& _proc, A&&... _args ):
	TaskT<size_>{std::bind(std::forward<F>(_proc), std::forward<A>(_args)...)} {}
#endif
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
 * Description       : create and initialize static undetachable task
 *
 * Parameters
 *   size            : size of task private stack (in bytes)
 *   prio            : initial task priority (any unsigned int value)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   args            : arguments for task proc
 *
 * Return            : TaskT<> object
 *
 ******************************************************************************/

	template<class F> static
	TaskT<size_> Make( const unsigned _prio, F&& _proc )
	{
		return { _prio, _proc };
	}

	template<class F> static
	TaskT<size_> Make( F&& _proc )
	{
		return Make(OS_MAIN_PRIO, _proc);
	}

#if __cplusplus >= 201402L
	template<typename F, typename... A> static
	TaskT<size_> Make( const unsigned _prio, F&& _proc, A&&... _args )
	{
		return Make(_prio, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}

#if __cplusplus >= 201703L && !defined(__ICCARM__)
	template<typename F, typename... A, typename = std::enable_if_t<std::is_invocable_v<F, A...>>> static
	TaskT<size_> Make( F&& _proc, A&&... _args )
	{
		return Make(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif
#endif

/******************************************************************************
 *
 * Name              : TaskT<>::Start
 *
 * Description       : create, initialize and start static undetachable task
 *
 * Parameters
 *   size            : size of task private stack (in bytes)
 *   prio            : initial task priority (any unsigned int value)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   args            : arguments for task proc
 *
 * Return            : TaskT<> object
 *
 ******************************************************************************/

	template<class F> static
	TaskT<size_> Start( const unsigned _prio, F&& _proc )
	{
		TaskT<size_> tsk { _prio, _proc };
		tsk.start();
		return tsk;
	}

	template<class F> static
	TaskT<size_> Start( F&& _proc )
	{
		return Start(OS_MAIN_PRIO, _proc);
	}

#if __cplusplus >= 201402L
	template<typename F, typename... A> static
	TaskT<size_> Start( const unsigned _prio, F&& _proc, A&&... _args )
	{
		return Start(_prio, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}

#if __cplusplus >= 201703L && !defined(__ICCARM__)
	template<typename F, typename... A, typename = std::enable_if_t<std::is_invocable_v<F, A...>>> static
	TaskT<size_> Start( F&& _proc, A&&... _args )
	{
		return Start(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif
#endif

#if __cplusplus >= 201402L
	using Deleter = struct _Deleter { void operator()( void* ) const {} };
	using Ptr = std::unique_ptr<TaskT<size_>, Deleter>;
#else
	using Ptr = TaskT<size_> *;
#endif

/******************************************************************************
 *
 * Name              : TaskT<>::Create
 *
 * Description       : create, initialize and start dynamic joinable task
 *                     with manageable resources
 *
 * Parameters
 *   size            : size of task private stack (in bytes)
 *   prio            : initial task priority (any unsigned int value)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   args            : arguments for task proc
 *
 * Return            : std::unique_pointer / pointer to dynamic joinable TaskT<> object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

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

	template<class F> static
	Ptr Create( F&& _proc )
	{
		return Create(OS_MAIN_PRIO, _proc);
	}

#if __cplusplus >= 201402L
	template<typename F, typename... A> static
	Ptr Create( const unsigned _prio, F&& _proc, A&&... _args )
	{
		return Create(_prio, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}

#if __cplusplus >= 201703L && !defined(__ICCARM__)
	template<typename F, typename... A, typename = std::enable_if_t<std::is_invocable_v<F, A...>>> static
	Ptr Create( F&& _proc, A&&... _args )
	{
		return Create(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif
#endif

/******************************************************************************
 *
 * Name              : TaskT<>::Detached
 *
 * Description       : create, initialize and start dynamic detached task
 *                     with manageable resources
 *
 * Parameters
 *   size            : size of task private stack (in bytes)
 *   prio            : initial task priority (any unsigned int value)
 *   proc            : task proc (initial task function) doesn't have to be noreturn-type
 *                     it will be executed into an infinite system-implemented loop
 *   args            : arguments for task proc
 *
 * Return            : std::unique_pointer / pointer to dynamic detached TaskT<> object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

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

	template<class F> static
	Ptr Detached( F&& _proc )
	{
		return Detached(OS_MAIN_PRIO, _proc);
	}

#if __cplusplus >= 201402L
	template<typename F, typename... A> static
	Ptr Detached( const unsigned _prio, F&& _proc, A&&... _args )
	{
		return Detached(_prio, std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}

#if __cplusplus >= 201703L && !defined(__ICCARM__)
	template<typename F, typename... A, typename = std::enable_if_t<std::is_invocable_v<F, A...>>> static
	Ptr Detached( F&& _proc, A&&... _args )
	{
		return Detached(std::bind(std::forward<F>(_proc), std::forward<A>(_args)...));
	}
#endif
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

#endif//__STATEOS_TSK_H
