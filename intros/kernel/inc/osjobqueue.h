/******************************************************************************

    @file    IntrOS: osjobqueue.h
    @author  Rajmund Szymanski
    @date    26.07.2022
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

/******************************************************************************
 *
 * Name              : job queue
 *
 ******************************************************************************/

typedef struct __job job_t;

struct __job
{
	unsigned count; // inherited from semaphore
	unsigned limit; // inherited from semaphore

	unsigned head;  // first element to read from data buffer
	unsigned tail;  // first element to write into data buffer
	fun_t ** data;  // data buffer
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

#define               _JOB_INIT( _limit, _data ) { 0, _limit, 0, 0, _data }

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

/******************************************************************************
 *
 * Name              : OS_JOB_BUFFER
 *
 * Description       : define job queue data buffer
 *
 * Parameters
 *   buf             : name of the buffer (passed to the init function)
 *   size            : size of a queue (max number of stored job procedures)
 *
 ******************************************************************************/

#define             OS_JOB_BUFFER( buf, size ) \
                       fun_t *buf[size]

/******************************************************************************
 *
 * Name              : OS_JOB
 * Static alias      : static_JOB
 *
 * Description       : define and initialize a job queue object
 *
 * Parameters
 *   job             : name of a pointer to job queue object
 *   limit           : size of a queue (max number of stored job procedures)
 *
 ******************************************************************************/

#define             OS_JOB( job, limit )        \
                static fun_t *job##__buf[limit]; \
                       job_t job[] = { _JOB_INIT( limit, job##__buf ) }

#define         static_JOB( job, limit )        \
                static fun_t *job##__buf[limit]; \
                static job_t job[] = { _JOB_INIT( limit, job##__buf ) }

/******************************************************************************
 *
 * Name              : JOB_INIT
 *
 * Description       : create and initialize a job queue object
 *
 * Parameters
 *   limit           : size of a queue (max number of stored job procedures)
 *
 * Return            : job queue object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                JOB_INIT( limit ) \
                      _JOB_INIT( limit, _JOB_DATA( limit ) )
#endif

/******************************************************************************
 *
 * Name              : JOB_CREATE
 * Alias             : JOB_NEW
 *
 * Description       : create and initialize a job queue object
 *
 * Parameters
 *   limit           : size of a queue (max number of stored job procedures)
 *
 * Return            : job queue object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                JOB_CREATE( limit ) \
                     { JOB_INIT  ( limit ) }
#define                JOB_NEW \
                       JOB_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : job_init
 *
 * Description       : initialize a job queue object
 *
 * Parameters
 *   job             : pointer to job queue object
 *   data            : job queue data buffer
 *   bufsize         : size of the data buffer (in bytes)
 *
 * Return            : none
 *
 ******************************************************************************/

void job_init( job_t *job, fun_t **data, size_t bufsize );

/******************************************************************************
 *
 * Name              : job_take
 * Alias             : job_tryWait
 * Async alias       : job_takeAsync
 *
 * Description       : try to transfer job data from the job queue object and execute the job procedure,
 *                     don't wait if the job queue object is empty
 *
 * Parameters
 *   job             : pointer to job queue object
 *
 * Return
 *   SUCCESS         : job data was successfully transferred from the job queue object
 *   FAILURE         : job queue object is empty
 *
 ******************************************************************************/

unsigned job_take( job_t *job );

__STATIC_INLINE
unsigned job_tryWait( job_t *job ) { return job_take(job); }

#if OS_ATOMICS
__STATIC_INLINE
unsigned job_takeAsync( job_t *job ) { return job_take(job); }
#endif

/******************************************************************************
 *
 * Name              : job_wait
 * Async alias       : job_waitAsync
 *
 * Description       : try to transfer job data from the job queue object and execute the job procedure,
 *                     wait indefinitely while the job queue object is empty
 *
 * Parameters
 *   job             : pointer to job queue object
 *
 * Return            : none
 *
 ******************************************************************************/

void job_wait( job_t *job );

#if OS_ATOMICS
__STATIC_INLINE
void job_waitAsync( job_t *job ) { job_wait(job); }
#endif

/******************************************************************************
 *
 * Name              : job_give
 * Async alias       : job_giveAsync
 *
 * Description       : try to transfer job data to the job queue object,
 *                     don't wait if the job queue object is full
 *
 * Parameters
 *   job             : pointer to job queue object
 *   fun             : pointer to job procedure
 *
 * Return
 *   SUCCESS         : job data was successfully transferred to the job queue object
 *   FAILURE         : job queue object is full
 *
 ******************************************************************************/

unsigned job_give( job_t *job, fun_t *fun );

#if OS_ATOMICS
__STATIC_INLINE
unsigned job_giveAsync( job_t *job, fun_t *fun ) { return job_give(job, fun); }
#endif

/******************************************************************************
 *
 * Name              : job_send
 * Async alias       : job_sendAsync
 *
 * Description       : try to transfer job data to the job queue object,
 *                     wait indefinitely while the job queue object is full
 *
 * Parameters
 *   job             : pointer to job queue object
 *   fun             : pointer to job procedure
 *
 * Return            : none
 *
 ******************************************************************************/

void job_send( job_t *job, fun_t *fun );

#if OS_ATOMICS
__STATIC_INLINE
void job_sendAsync( job_t *job, fun_t *fun ) { job_send(job, fun); }
#endif

/******************************************************************************
 *
 * Name              : job_push
 *
 * Description       : transfer job data to the job queue object,
 *                     remove the oldest job data if the job queue object is full
 *
 * Parameters
 *   job             : pointer to job queue object
 *   fun             : pointer to job procedure
 *
 * Return            : none
 *
 ******************************************************************************/

void job_push( job_t *job, fun_t *fun );

/******************************************************************************
 *
 * Name              : job_getCount
 *
 * Description       : return the amount of data contained in the job queue
 *
 * Parameters
 *   job             : pointer to job queue object
 *
 * Return            : amount of data contained in the job queue
 *
 ******************************************************************************/

unsigned job_getCount( job_t *job );

/******************************************************************************
 *
 * Name              : job_getSpace
 *
 * Description       : return the amount of free space in the job queue
 *
 * Parameters
 *   job             : pointer to job queue object
 *
 * Return            : amount of free space in the job queue
 *
 ******************************************************************************/

unsigned job_getSpace( job_t *job );

/******************************************************************************
 *
 * Name              : job_getLimit
 *
 * Description       : return the size of the job queue
 *
 * Parameters
 *   job             : pointer to job queue object
 *
 * Return            : size of the job queue
 *
 ******************************************************************************/

unsigned job_getLimit( job_t *job );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
namespace intros {

/******************************************************************************
 *
 * Class             : JobQueueT<>
 *
 * Description       : create and initialize a job queue object
 *
 * Constructor parameters
 *   limit           : size of a queue (max number of stored job procedures)
 *
 ******************************************************************************/

template<unsigned limit_>
struct JobQueueT : public __job
{
	constexpr
	JobQueueT(): __job _JOB_INIT(limit_, data_) {}

	JobQueueT( JobQueueT&& ) = default;
	JobQueueT( const JobQueueT& ) = delete;
	JobQueueT& operator=( JobQueueT&& ) = delete;
	JobQueueT& operator=( const JobQueueT& ) = delete;

	unsigned take     ()              { return job_take     (this); }
	unsigned tryWait  ()              { return job_tryWait  (this); }
	void     wait     ()              {        job_wait     (this); }
	unsigned give     ( fun_t *_fun ) { return job_give     (this, _fun); }
	void     send     ( fun_t *_fun ) {        job_send     (this, _fun); }
	void     push     ( fun_t *_fun ) {        job_push     (this, _fun); }
	unsigned getCount ()              { return job_getCount (this); }
	unsigned getSpace ()              { return job_getSpace (this); }
	unsigned getLimit ()              { return job_getLimit (this); }
#if OS_ATOMICS
	unsigned takeAsync()              { return job_takeAsync(this); }
	void     waitAsync()              {        job_waitAsync(this); }
	unsigned giveAsync( fun_t *_fun ) { return job_giveAsync(this, _fun); }
	void     sendAsync( fun_t *_fun ) {        job_sendAsync(this, _fun); }
#endif

	private:
	fun_t *data_[limit_];
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_JOB_H
