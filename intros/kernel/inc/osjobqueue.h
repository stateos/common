/******************************************************************************

    @file    IntrOS: osjobqueue.h
    @author  Rajmund Szymanski
    @date    17.11.2025
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

#ifndef __INTROS_JOB_H
#define __INTROS_JOB_H

#include "oskernel.h"
#include "osclock.h"

/**
 * @defgroup jobqueue Job queue
 * @brief    job queues (procedures executed by the receiving task)
 * @{
 */

/**
 * @brief   job queue
 */

typedef struct __job job_t;

struct __job
{
	obj_t    obj;   ///< object header

	unsigned count; ///< inherited from semaphore
	unsigned limit; ///< inherited from semaphore

	unsigned head;  ///< first element to read from data buffer
	unsigned tail;  ///< first element to write into data buffer
	fun_t ** data;  ///< data buffer
};

typedef struct __job job_id [];

/******************************************************************************
 *
 * Name              : _JOB_INIT
 *
 * Description       : create and initialize a job queue object
 *
 * Parameters
 *   limit           : size of a queue (max number of stored job procedures)
 *   data            : job queue data buffer
 *
 * Return            : job queue object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _JOB_INIT( _limit, _data ) { _OBJ_INIT(), 0, _limit, 0, 0, _data }

/******************************************************************************
 *
 * Name              : _JOB_DATA
 *
 * Description       : create a job queue data buffer
 *
 * Parameters
 *   limit           : size of a queue (max number of stored job procedures)
 *
 * Return            : job queue data buffer
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#ifndef __cplusplus
#define               _JOB_DATA( _limit ) (fun_t *[_limit]){ NULL }
#endif

/**
 * @brief   define job queue data buffer
 *
 * @param   buf        name of the buffer (passed to the init function)
 * @param   size       size of a queue (max number of stored job procedures)
 */

#define             OS_JOB_BUFFER( buf, size ) \
                       fun_t *buf[size]

/**
 * @brief   define and initialize a job queue object
 *
 * @param   job        name of a pointer to job queue object
 * @param   limit      size of a queue (max number of stored job procedures)
 */

#define             OS_JOB( job, limit )        \
                static fun_t *job##__buf[limit]; \
                       job_t job[] = { _JOB_INIT( limit, job##__buf ) }

/** static alias of OS_JOB() */
#define         static_JOB( job, limit )        \
                static fun_t *job##__buf[limit]; \
                static job_t job[] = { _JOB_INIT( limit, job##__buf ) }

/**
 * @brief   create and initialize a job queue object
 *
 * @param   limit      size of a queue (max number of stored job procedures)
 *
 * @return  job queue object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                JOB_INIT( limit ) \
                      _JOB_INIT( limit, _JOB_DATA( limit ) )
#endif

/**
 * @brief   create and initialize a job queue object
 *
 * @param   limit      size of a queue (max number of stored job procedures)
 *
 * @return  job queue object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                JOB_CREATE( limit ) \
                     { JOB_INIT  ( limit ) }
/** alias of JOB_CREATE() */
#define                JOB_NEW \
                       JOB_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize a job queue object
 *
 * @param   job        pointer to job queue object
 * @param   data       job queue data buffer
 * @param   bufsize    size of the data buffer (in bytes)
 */

void job_init( job_t *job, fun_t **data, size_t bufsize );

/**
 * @brief   reset the job queue object and wake up all waiting tasks with 'E_STOPPED' event
 *
 * @param   job        pointer to job queue object
 *
 * @note    use only in thread mode
 */

void job_reset( job_t *job );

/** alias of job_reset() */
__STATIC_INLINE
void job_kill( job_t *job ) { job_reset(job); }

/**
 * @brief   try to transfer job data from the job queue object and execute the job procedure,
 *          don't wait if the job queue object is empty
 *
 * @param   job        pointer to job queue object
 *
 * @retval  E_SUCCESS  job data was successfully transferred from the job queue object
 * @retval  E_TIMEOUT  job queue object is empty, try again
 *
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

int job_take( job_t *job );

/** alias of job_take() */
__STATIC_INLINE
int job_tryWait( job_t *job ) { return job_take(job); }

#if OS_ATOMICS
/** Async alias of job_take() */
int job_takeAsync( job_t *job );
#endif

/**
 * @brief   try to transfer job data from the job queue object and execute the job procedure,
 *          wait for given duration of time while the job queue object is empty
 *
 * @param   job        pointer to job queue object
 * @param   delay      duration of time (maximum number of ticks to wait while the job queue object is empty)
 *                     - IMMEDIATE: don't wait if the job queue object is empty
 *                     - INFINITE:  wait indefinitely while the job queue object is empty
 *
 * @retval  E_SUCCESS  job data was successfully transferred from the job queue object
 * @retval  E_STOPPED  job queue object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  job queue object is empty and was not received data before the specified timeout expired
 */

int job_waitFor( job_t *job, cnt_t delay );

/**
 * @brief   try to transfer job data from the job queue object and execute the job procedure,
 *          wait until given timepoint while the job queue object is empty
 *
 * @param   job        pointer to job queue object
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  job data was successfully transferred from the job queue object
 * @retval  E_STOPPED  job queue object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  job queue object is empty and was not received data before the specified timeout expired
 */

int job_waitUntil( job_t *job, cnt_t time );

/**
 * @brief   try to transfer job data from the job queue object and execute the job procedure,
 *          wait indefinitely while the job queue object is empty
 *
 * @param   job        pointer to job queue object
 *
 * @retval  E_SUCCESS  job data was successfully transferred from the job queue object
 * @retval  E_STOPPED  job queue object was reseted (unavailable for async version)
 *
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

__STATIC_INLINE
int job_wait( job_t *job ) { return job_waitFor(job, INFINITE); }

#if OS_ATOMICS
/** Async alias of job_wait() */
int job_waitAsync( job_t *job );
#endif

/**
 * @brief   try to transfer job data to the job queue object,
 *          don't wait if the job queue object is full
 *
 * @param   job        pointer to job queue object
 * @param   fun        pointer to job procedure
 *
 * @retval  E_SUCCESS  job data was successfully transferred to the job queue object
 * @retval  E_TIMEOUT  job queue object is full, try again
 *
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

int job_give( job_t *job, fun_t *fun );

#if OS_ATOMICS
/** Async alias of job_give() */
int job_giveAsync( job_t *job, fun_t *fun );
#endif

/**
 * @brief   try to transfer job data to the job queue object,
 *          wait for given duration of time while the job queue object is full
 *
 * @param   job        pointer to job queue object
 * @param   fun        pointer to job procedure
 * @param   delay      duration of time (maximum number of ticks to wait while the job queue object is full)
 *                     - IMMEDIATE: don't wait if the job queue object is full
 *                     - INFINITE:  wait indefinitely while the job queue object is full
 *
 * @retval  E_SUCCESS  job data was successfully transferred to the job queue object
 * @retval  E_STOPPED  job queue object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  job queue object is full and was not issued data before the specified timeout expired
 */

int job_sendFor( job_t *job, fun_t *fun, cnt_t delay );

/**
 * @brief   try to transfer job data to the job queue object,
 *          wait until given timepoint while the job queue object is full
 *
 * @param   job        pointer to job queue object
 * @param   fun        pointer to job procedure
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  job data was successfully transferred to the job queue object
 * @retval  E_STOPPED  job queue object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  job queue object is full and was not issued data before the specified timeout expired
 */

int job_sendUntil( job_t *job, fun_t *fun, cnt_t time );

/**
 * @brief   try to transfer job data to the job queue object,
 *          wait indefinitely while the job queue object is full
 *
 * @param   job        pointer to job queue object
 * @param   fun        pointer to job procedure
 *
 * @retval  E_SUCCESS  job data was successfully transferred to the job queue object
 * @retval  E_STOPPED  job queue object was reseted (unavailable for async version)
 *
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

__STATIC_INLINE
int job_send( job_t *job, fun_t *fun ) { return job_sendFor(job, fun, INFINITE); }

#if OS_ATOMICS
/** Async alias of job_send() */
int job_sendAsync( job_t *job, fun_t *fun );
#endif

/**
 * @brief   transfer job data to the job queue object,
 *          remove the oldest job data if the job queue object is full
 *
 * @param   job        pointer to job queue object
 * @param   fun        pointer to job procedure
 */

void job_push( job_t *job, fun_t *fun );

/**
 * @brief   return the amount of data contained in the job queue
 *
 * @param   job        pointer to job queue object
 *
 * @return  amount of data contained in the job queue
 */

unsigned job_count( job_t *job );

/**
 * @brief   return the amount of free space in the job queue
 *
 * @param   job        pointer to job queue object
 *
 * @return  amount of free space in the job queue
 */

unsigned job_space( job_t *job );

/**
 * @brief   return the size of the job queue
 *
 * @param   job        pointer to job queue object
 *
 * @return  size of the job queue
 */

unsigned job_limit( job_t *job );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus)
namespace intros {

/**
 * @brief   create and initialize a job queue object
 *
 * @par     Constructor parameters
 *          - limit: size of a queue (max number of stored job procedures)
 */

template<unsigned limit_>
struct JobQueueT : public __job
{
	/// constructor, see the constructor parameters of the class
	constexpr
	JobQueueT(): __job _JOB_INIT(limit_, data_) {}

	~JobQueueT() { assert(__job::obj.queue == nullptr); }

	/// move constructor
	JobQueueT( JobQueueT&& _src ): __job _JOB_INIT(limit_, data_)
	{
		assert(_src.__job::count == 0 && _src.obj.queue == nullptr);
		(void) _src;
	}

	JobQueueT( const JobQueueT& ) = delete;
	JobQueueT& operator=( JobQueueT&& ) = delete;
	JobQueueT& operator=( const JobQueueT& ) = delete;

	void     reset    ()                               {        job_reset    (this); }                             ///< job_reset()
	void     kill     ()                               {        job_kill     (this); }                             ///< job_kill()
	int      take     ()                               { return job_take     (this); }                             ///< job_take()
	int      tryWait  ()                               { return job_tryWait  (this); }                             ///< job_tryWait()
	template<typename T>
	int      waitFor  ( const T& _delay )              { return job_waitFor  (this, Clock::count(_delay)); }       ///< job_waitFor()
	template<typename T>
	int      waitUntil( const T& _time )               { return job_waitUntil(this, Clock::until(_time)); }        ///< job_waitUntil()
	int      wait     ()                               { return job_wait     (this); }                             ///< job_wait()
	int      give     ( fun_t *_fun )                  { return job_give     (this, _fun); }                       ///< job_give()
	template<typename T>
	int      sendFor  ( fun_t *_fun, const T& _delay ) { return job_sendFor  (this, _fun, Clock::count(_delay)); } ///< job_sendFor()
	template<typename T>
	int      sendUntil( fun_t *_fun, const T& _time )  { return job_sendUntil(this, _fun, Clock::until(_time)); }  ///< job_sendUntil()
	int      send     ( fun_t *_fun )                  { return job_send     (this, _fun); }                       ///< job_send()
	void     push     ( fun_t *_fun )                  {        job_push     (this, _fun); }                       ///< job_push()
	unsigned count    ()                               { return job_count    (this); }                             ///< job_count()
	unsigned space    ()                               { return job_space    (this); }                             ///< job_space()
	unsigned limit    ()                               { return job_limit    (this); }                             ///< job_limit()
#if OS_ATOMICS
	int      takeAsync()                               { return job_takeAsync(this); }                             ///< job_takeAsync()
	int      waitAsync()                               { return job_waitAsync(this); }                             ///< job_waitAsync()
	int      giveAsync( fun_t *_fun )                  { return job_giveAsync(this, _fun); }                       ///< job_giveAsync()
	int      sendAsync( fun_t *_fun )                  { return job_sendAsync(this, _fun); }                       ///< job_sendAsync()
#endif

	private:
	fun_t *data_[limit_];
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

/** @} */

#endif//__INTROS_JOB_H
