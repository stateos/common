/******************************************************************************

    @file    DemOS: os.h
    @author  Rajmund Szymański
    @date    24.04.2023
    @brief   This file provides set of functions for DemOS.

 ******************************************************************************

   Copyright (c) 2018-2023 Rajmund Szymanski. All rights reserved.

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

#ifndef __DEMOS_H
#define __DEMOS_H

#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include "osversion.h"
#include "osport.h"

#ifdef __cplusplus
extern "C" {
#endif

/* System =================================================================== */

#ifndef OS_FREQUENCY
#define OS_FREQUENCY  1000
#endif

#if    (OS_FREQUENCY)/1000000 > 0
#define USEC       (cnt_t)((OS_FREQUENCY)/1000000) // time multiplier given in microseconds
#endif
#if    (OS_FREQUENCY)/1000 > 0
#define MSEC       (cnt_t)((OS_FREQUENCY)/1000)    // time multiplier given in milliseconds
#endif
#define SEC        ((cnt_t)(OS_FREQUENCY))         // time multiplier given in seconds
#define MIN        ((cnt_t)(OS_FREQUENCY)*60)      // time multiplier given in minutes

#ifndef INFINITE
#define INFINITE   ((cnt_t)0-1)                    // time value for infinite wait
#endif

/* -------------------------------------------------------------------------- */
// definition of task function
typedef void fun_t( void );

// system functions
void    sys_init ( void );           // port function - initialize the system timer
cnt_t   sys_time ( void );           // port function - get current value of the system timer (in milliseconds)
void    sys_delay( cnt_t );          // core function - blocking delay

#ifdef  USE_GOTO

/* Task ( goto / label ) ==================================================== */

typedef void * tag_t;

#define TSK_CONCATENATE(tag)            Line ## tag
#define TSK_LABEL(tag)                  TSK_CONCATENATE(tag)
#define TSK_STATE(tag)                  sys.current->state = && TSK_LABEL(tag)
#define TSK_BEGIN()                     if (sys.current->state) goto *sys.current->state
#define TSK_END()

#else//!USE_GOTO

/* Task ( switch / case ) =================================================== */

typedef intptr_t tag_t;

#define TSK_LABEL(tag)                  /* falls through */ case tag
#define TSK_STATE(tag)                  sys.current->state = tag
#define TSK_BEGIN()                     switch (sys.current->state) { case 0:
#define TSK_END()                       }

#endif//USE_GOTO

/* Task ( internal ) ======================================================== */

// for internal use; wait while the condition (cnd) is true
#define TSK_WHILE(cnd)                  TSK_STATE(__LINE__); TSK_LABEL(__LINE__): if (cnd) return; (void)0
// for internal use; pass control to the next ready task if the condition (cnd) is true
#define TSK_YIELD(cnd)                  TSK_STATE(__LINE__); if (cnd) return; TSK_LABEL(__LINE__): (void)0

/* Task ===================================================================== */
// definition of timer
typedef struct __tmr { cnt_t start; cnt_t delay; } tmr_t;
// definition of task state
typedef enum   __tid { ID_RIP = 0, ID_RDY, ID_DLY } tid_t;
// definition of task structure
typedef struct __tsk { tmr_t tmr; tid_t id; tag_t state; fun_t *function; struct __tsk *next; } tsk_t;
// definition of system
typedef struct __sys { tsk_t *current; volatile cnt_t counter; bool suspended; } sys_t;

// timer initializer
#define TMR_INIT()                      { 0, 0 }
// task initializer
#define TSK_INIT(fun)                   { TMR_INIT(), ID_RIP, 0, fun, (tsk_t*)NULL }

extern
sys_t   sys;                         // system struct
void    tsk_start( tsk_t* );         // system function - make the task ready to execute
void    sys_start( void );           // system function - start the scheduler

#define sys_suspend()              do { sys.suspended = true;                                                      } while(0)
#define sys_resume()               do { sys.suspended = false;                                                     } while(0)

/* -------------------------------------------------------------------------- */
// define and initialize the task (tsk) with function (fun)
#define OS_TSK(tsk, fun)                tsk_t tsk[] = { TSK_INIT(fun) }
// define and initialize the task (tsk); function body must be defined immediately below
#define OS_TSK_DEF(tsk)                 void tsk ## __fun( void ); \
                                        OS_TSK(tsk, tsk ## __fun); \
                                        void tsk ## __fun( void )
// define, initialize and start the task (tsk); function body must be defined immediately below
#define OS_TSK_START(tsk)               void tsk ## __fun( void ); \
                                        OS_TSK(tsk, tsk ## __fun); \
           __attribute__((constructor)) void tsk ## __run( void ) { tsk_start(tsk); } \
                                        void tsk ## __fun( void )
/* -------------------------------------------------------------------------- */
// return the current task
#define tsk_this()                    ( sys.current )
//alias
#define SELF                          ( sys.current )
// check whether the task (tsk) is the current task
#define tsk_self(tsk)                 ( sys.current == (tsk) )
// check whether the task (tsk) is ready to execute
#define tsk_ready(tsk)                ( (tsk)->id == ID_RDY )
// check whether the task (tsk) has been stopped
#define tsk_dead(tsk)                 ( (tsk)->id == ID_RIP )
// check whether the task (tsk) has been suspended
#define tsk_suspended(tsk)            ( (tsk)->id == ID_DLY )
// necessary prologue of the task
#define tsk_begin()                     TSK_BEGIN(); do {                                                          } while(0)
#ifdef  USE_EXIT
// necessary epilogue of the task - task is killed at the end of the task procedure
#define tsk_end()                       TSK_END(); do { tsk_exit();                                                } while(0)
#else//!USE_EXIT
// necessary epilogue of the task - task procedure is executed in an infinite loop
#define tsk_end()                       TSK_END(); do { tsk_again();                                               } while(0)
#endif//USE_EXIT
// wait while the condition (cnd) is true
#define tsk_waitWhile(cnd)         do { TSK_WHILE(cnd);                                                            } while(0)
// wait while the condition (cnd) is false
#define tsk_waitUntil(cnd)         do { tsk_waitWhile(!(cnd));                                                     } while(0)
// start new or restart previously stopped task (tsk) with function (fun)
#define tsk_startFrom(tsk, fun)    do { if ((tsk)->id == ID_RIP) { (tsk)->function = (fun); tsk_start(tsk); }      } while(0)
// wait while the task (tsk) is working
#define tsk_join(tsk)              do { tsk_waitUntil((tsk)->id == ID_RIP);                                        } while(0)
// start task (tsk) and wait for the end of execution of (tsk)
#define tsk_call(tsk)              do { tsk_start(tsk); tsk_join(tsk);                                             } while(0)
// start new or restart previously stopped task (tsk) with function (fun) and wait for the end of execution of (tsk)
#define tsk_callFrom(tsk, fun)     do { tsk_startFrom(tsk, fun); tsk_join(tsk);                                    } while(0)
// restart the current task from the initial state
#define tsk_again()                do { sys.current->state = (tag_t)0; return;                                     } while(0)
// stop the current task; it will no longer be executed
#define tsk_stop()                 do { sys.current->id = ID_RIP; return;                                          } while(0)
// stop the current task; it will no longer be executed
#define tsk_exit()                 do { sys.current->id = ID_RIP; return;                                          } while(0)
// stop the task (tsk); it will no longer be executed
#define tsk_kill(tsk)              do { (tsk)->id = ID_RIP; if (tsk_self(tsk)) return;                             } while(0)
// restart the task (tsk) from the initial state
#define tsk_restart(tsk)           do { if (tsk_self(tsk)) tsk_again(); tsk_kill(tsk); tsk_start(tsk);             } while(0)
// restart the task (tsk) with function (fun)
#define tsk_restartFrom(tsk, fun)  do { if (tsk_self(tsk)) tsk_flip(fun); tsk_kill(tsk); tsk_startFrom(tsk, fun);  } while(0)
// pass control to the next ready task
#define tsk_yield()                do { TSK_YIELD(true);                                                           } while(0)
// restart the current task with function (fun)
#define tsk_flip(fun)              do { sys.current->function = (fun); tsk_again();                                } while(0)
// delay execution of current task for given duration of time (dly)
#define tsk_sleepFor(dly)          do { tmr_startFor(&sys.current->tmr, dly); tsk_yield();                         } while(0)
// delay execution of current task for given duration of time (dly) from the end of the previous countdown
#define tsk_sleepNext(dly)         do { tmr_startNext(&sys.current->tmr, dly); tsk_yield();                        } while(0)
// delay execution of current task until given timepoint (tim)
#define tsk_sleepUntil(tim)        do { tmr_startUntil(&sys.current->tmr, tim); tsk_yield();                       } while(0)
// delay indefinitely execution of current task
#define tsk_sleep()                do { tsk_sleepFor(INFINITE);                                                    } while(0)
// delay execution of current task for given duration of time (dly)
#define tsk_delay(dly)             do { tsk_sleepFor(dly);                                                         } while(0)
// suspend execution of the ready task (tsk)
#define tsk_suspend(tsk)           do { if ((tsk)->id == ID_RDY) { (tsk)->id = ID_DLY; TSK_YIELD(tsk_self(tsk)); } } while(0)
// resume execution of the suspended task (tsk)
#define tsk_resume(tsk)            do { if ((tsk)->id == ID_DLY) { (tsk)->id = ID_RDY; }                           } while(0)

/* Timer ==================================================================== */
// define and initialize the timer (tmr)
#define OS_TMR(tmr)                     tmr_t tmr[] = { TMR_INIT() }
/* -------------------------------------------------------------------------- */
// check if the timer (tmr) finishes countdown
#define tmr_expired(tmr)              ( sys_time() - (tmr)->start + 1 > (tmr)->delay )
// check if the timer (tmr) finishes countdown from the end of the previous countdown
#define tmr_expiredNext(tmr)          ( tmr_expired(tmr) ? (((tmr)->start += (tmr)->delay), true) : false )
// start/restart the timer (tmr) for given duration of time (dly)
#define tmr_startFor(tmr, dly)     do { (tmr)->start = sys_time(); (tmr)->delay = (dly);                           } while(0)
// start/restart the timer (tmr) for given duration of time (dly) from the end of the previous countdown
#define tmr_startNext(tmr, dly)    do {                            (tmr)->delay = (dly);                           } while(0)
// start/restart the timer (tmr) until given timepoint (tim)
#define tmr_startUntil(tmr, tim)   do { (tmr)->start = sys_time(); (tmr)->delay = (tim) - (tmr)->start;            } while(0)
// start/restart the timer (tmr) and wait until the timer (tmr) finishes countdown for given duration of time (dly)
#define tmr_waitFor(tmr, dly)      do { tmr_startFor(tmr, dly);   tsk_waitUntil(tmr_expiredNext(tmr));             } while(0)
// wait until the timer (tmr) finishes countdown for given duration of time (dly) from the end of the previous countdown
#define tmr_waitNext(tmr, dly)     do { tmr_startNext(tmr, dly);  tsk_waitUntil(tmr_expiredNext(tmr));             } while(0)
// start/restart the timer (tmr) and wait until the timer (tmr) finishes countdown until given timepoint (tim)
#define tmr_waitUntil(tmr, tim)    do { tmr_startUntil(tmr, tim); tsk_waitUntil(tmr_expiredNext(tmr));             } while(0)

/* Mutex ==================================================================== */
// definition of mutex
typedef tsk_t *mtx_t;

/* -------------------------------------------------------------------------- */
// define and initialize the mutex (mtx)
#define OS_MTX(mtx)                     mtx_t mtx[] = { (tsk_t*)NULL }
/* -------------------------------------------------------------------------- */
// try to lock the mutex (mtx); return true if the mutex has been successfully locked
#define mtx_take(mtx)                 ( *(mtx) ? false : ((*(mtx) = sys.current), true) )
// alias
#define mtx_tryLock(mtx)                mtx_take(mtx)
// wait for the released mutex (mtx) and lock it
#define mtx_wait(mtx)                   tsk_waitUntil(mtx_take(mtx))
// alias
#define mtx_lock(mtx)                   mtx_wait(mtx)
// release previously owned mutex (mtx); return true if the mutex has been successfully released
#define mtx_give(mtx)                 ( tsk_self(*(mtx)) ? ((*(mtx) = (tsk_t*)NULL), true) : false )
// alias
#define mtx_unlock(mtx)                 mtx_give(mtx)

/* Binary semaphore ========================================================= */
// definition of binary semaphore
typedef uint_fast8_t sem_t;

/* -------------------------------------------------------------------------- */
// define and initialize the binary semaphore (sem) with initial value (ini)
#define OS_SEM(sem, ini)                sem_t sem[] = { ini }
/* -------------------------------------------------------------------------- */
// return true if the semaphore (sem) is released
#define sem_given(sem)                ( *(sem) != 0 )
// alias
#define sem_released(sem)               sem_giveen(sem)
// try to lock the semaphore (sem); return true if the semaphore has been successfully locked
#define sem_take(sem)                 ( sem_given(sem) ? ((*(sem) = 0), true) : false )
// alias
#define sem_tryWait(sem)                sem_take(sem)
// wait for the released semaphore (sem) and lock it
#define sem_wait(sem)                   tsk_waitUntil(sem_take(sem))
// return true if the semaphore (sem) is locked
#define sem_taken(sem)                ( *(sem) == 0 )
// try to release the semaphore (sem); return true if the semaphore has been successfully released
#define sem_give(sem)                 ( sem_taken(sem) ? ((*(sem) = 1), true) : false )
// alias
#define sem_post(sem)                   sem_give(sem)
// alias
#define sem_release(sem)                sem_give(sem)

/* Protected signal ========================================================= */
// definition of protected signal
typedef unsigned sig_t;

#define SIG_LIMIT     (sizeof(sig_t) * CHAR_BIT)

#define SIGSET(signo) (((signo) < SIG_LIMIT) ? 1U << (signo) : 0U) // signal mask from the given signal number
#define sigAll        (sig_t)(-1)                                  // signal mask for all signals

/* -------------------------------------------------------------------------- */
// define and initialize the protected signal (sig)
#define OS_SIG(sig)                     sig_t sig[] = { 0 }
/* -------------------------------------------------------------------------- */
// return true if the signal number (num) of signal object (sig) is set
#define sig_take(sig, sigset)         ( (*(sig) & (sigset)) != 0 )
// alias
#define sig_tryWait(sig, sigset)        sig_take(sig, sigset)
// wait for the signal (sig) to be set
#define sig_wait(sig, sigset)           tsk_waitUntil(sig_take(sig, sigset))
// try to reset the signal (sig); return true if the signal has been successfully reset
#define sig_clear(sig, signo)         ( sig_take(sig, SIGSET(signo)) ? ((*(sig) &= ~SIGSET(signo)), true) : false )
// try to set the signal (sig); return true if the signal has been successfully set
#define sig_give(sig, signo)          ( sig_take(sig, SIGSET(signo)) ? false : ((*(sig) |= SIGSET(signo)), true) )
// alias
#define sig_set(sig, signo)             sig_give(sig, signo)

/* Event ==================================================================== */
// definition of event
typedef uintptr_t evt_t;

/* -------------------------------------------------------------------------- */
// define and initialize the event (evt)
#define OS_EVT(evt)                     evt_t evt[] = { 0 }
/* -------------------------------------------------------------------------- */
// get the event (evt) value
#define evt_take(evt)                 ( *(evt) )
// wait for a the new value of the event (evt)
#define evt_wait(evt)              do { *(evt) = 0; tsk_waitUntil(*(evt)); } while(0)
// set a new value (val) of the event (evt)
#define evt_give(evt, val)         do { *(evt) = (val);                    } while(0)

/* Job ====================================================================== */
// definition of job
typedef fun_t *job_t;

/* -------------------------------------------------------------------------- */
// define and initialize the job (job)
#define OS_JOB(job)                     job_t job[] = { (fun_t*)NULL }
/* -------------------------------------------------------------------------- */
// try to take a function from the job (job); return true if the function has been successfully taken and executed
#define job_take(job)                 ( *(job) ? ((*(job))(), *(job) = (fun_t*)NULL, true) : false )
// alias
#define job_tryWait(job)                job_take(job)
// wait for a the new job (job) function, execute it and release the job
#define job_wait(job)                   tsk_waitUntil(job_take(job))
// try to give the new function (fun) to the job (job); return true if the function has been successfully given
#define job_give(job, fun)            ( *(job) ? false : (*(job) = (fun), true) )
// wait for the released job (job) and give it a new function (fun)
#define job_send(job, fun)              tsk_waitUntil(job_give(job, fun))

/* Once flag ================================================================ */
// definition of once flag
typedef uint_fast8_t one_t;

/* -------------------------------------------------------------------------- */
// define and initialize the once flag (one)
#define OS_ONE(one)                     one_t one[] = { 0 }
/* -------------------------------------------------------------------------- */
// try to execute function (fun); if the once flag (one) has been previously set, the function is not executed
#define one_call(one, fun)         do { if (*(one) == 0) { (fun)(); *(one) = 1; } } while(0)

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif//__DEMOS_H
