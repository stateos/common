/******************************************************************************

    @file    StateOS: osjobqueue.c
    @author  Rajmund Szymanski
    @date    01.11.2022
    @brief   This file provides set of functions for StateOS.

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

#include "inc/osjobqueue.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_job_init( job_t *job, fun_t **data, size_t bufsize, void *res )
/* -------------------------------------------------------------------------- */
{
	memset(job, 0, sizeof(job_t));

	core_obj_init(&job->obj, res);

	job->limit = bufsize / sizeof(fun_t *);
	job->data  = data;
}

/* -------------------------------------------------------------------------- */
void job_init( job_t *job, fun_t **data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(job);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		priv_job_init(job, data, bufsize, NULL);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
job_t *job_create( unsigned limit )
/* -------------------------------------------------------------------------- */
{
	struct job_T { job_t job; fun_t *buf[]; } *tmp;
	job_t *job = NULL;
	size_t bufsize;

	assert_tsk_context();
	assert(limit);

	sys_lock();
	{
		bufsize = limit * sizeof(fun_t *);
		tmp = malloc(sizeof(struct job_T) + bufsize);
		if (tmp)
			priv_job_init(job = &tmp->job, tmp->buf, bufsize, tmp);
	}
	sys_unlock();

	return job;
}

/* -------------------------------------------------------------------------- */
static
void priv_job_reset( job_t *job, int event )
/* -------------------------------------------------------------------------- */
{
	job->count = 0;
	job->head  = 0;
	job->tail  = 0;

	core_all_wakeup(job->obj.queue, event);
}

/* -------------------------------------------------------------------------- */
void job_reset( job_t *job )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(job);
	assert(job->obj.res!=RELEASED);

	sys_lock();
	{
		priv_job_reset(job, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void job_destroy( job_t *job )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(job);
	assert(job->obj.res!=RELEASED);

	sys_lock();
	{
		priv_job_reset(job, job->obj.res ? E_DELETED : E_STOPPED);
		core_res_free(&job->obj);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
bool priv_job_empty( job_t *job )
/* -------------------------------------------------------------------------- */
{
	return job->count == 0;
}

/* -------------------------------------------------------------------------- */
static
bool priv_job_full( job_t *job )
/* -------------------------------------------------------------------------- */
{
	return job->count == job->limit;
}

/* -------------------------------------------------------------------------- */
static
void priv_job_dec( job_t *job )
/* -------------------------------------------------------------------------- */
{
	job->head = job->head + 1 < job->limit ? job->head + 1 : 0;
	job->count -= 1;
}

/* -------------------------------------------------------------------------- */
static
void priv_job_inc( job_t *job )
/* -------------------------------------------------------------------------- */
{
	job->tail = job->tail + 1 < job->limit ? job->tail + 1 : 0;
	job->count += 1;
}

/* -------------------------------------------------------------------------- */
static
fun_t *priv_job_get( job_t *job )
/* -------------------------------------------------------------------------- */
{
	fun_t *fun = job->data[job->head];

	priv_job_dec(job);

	return fun;
}

/* -------------------------------------------------------------------------- */
static
void priv_job_put( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	job->data[job->tail] = fun;

	priv_job_inc(job);
}

/* -------------------------------------------------------------------------- */
static
fun_t *priv_job_getUpdate( job_t *job )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	fun_t *fun = priv_job_get(job);

	tsk = core_one_wakeup(job->obj.queue, E_SUCCESS);
	if (tsk) priv_job_put(job, tsk->tmp.job.fun);

	return fun;
}

/* -------------------------------------------------------------------------- */
static
void priv_job_putUpdate( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	priv_job_put(job, fun);

	tsk = core_one_wakeup(job->obj.queue, E_SUCCESS);
	if (tsk) tsk->tmp.job.fun = priv_job_get(job);
}

/* -------------------------------------------------------------------------- */
static
void priv_job_skipUpdate( job_t *job )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	while (priv_job_full(job))
	{
		priv_job_dec(job);

		tsk = core_one_wakeup(job->obj.queue, E_SUCCESS);
		if (tsk) priv_job_put(job, tsk->tmp.job.fun);
	}
}

/* -------------------------------------------------------------------------- */
static
int priv_job_take( job_t *job, fun_t **fun )
/* -------------------------------------------------------------------------- */
{
	if (priv_job_empty(job))
		return E_TIMEOUT;

	*fun = priv_job_getUpdate(job);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int job_take( job_t *job )
/* -------------------------------------------------------------------------- */
{
	int result;
	fun_t *fun = NULL;

	assert(job);
	assert(job->obj.res!=RELEASED);
	assert(job->data);
	assert(job->limit);

	sys_lock();
	{
		result = priv_job_take(job, &fun);
	}
	sys_unlock();

	if (fun != NULL)
		fun();

	return result;
}

/* -------------------------------------------------------------------------- */
int job_waitFor( job_t *job, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(job);
	assert(job->obj.res!=RELEASED);
	assert(job->data);
	assert(job->limit);

	sys_lock();
	{
		result = priv_job_take(job, &System.cur->tmp.job.fun);
		if (result == E_TIMEOUT)
			result = core_tsk_waitFor(&job->obj.queue, delay);
	}
	sys_unlock();

	if (result == E_SUCCESS)
		System.cur->tmp.job.fun();

	return result;
}

/* -------------------------------------------------------------------------- */
int job_waitUntil( job_t *job, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(job);
	assert(job->obj.res!=RELEASED);
	assert(job->data);
	assert(job->limit);

	sys_lock();
	{
		result = priv_job_take(job, &System.cur->tmp.job.fun);
		if (result == E_TIMEOUT)
			result = core_tsk_waitUntil(&job->obj.queue, time);
	}
	sys_unlock();

	if (result == E_SUCCESS)
		System.cur->tmp.job.fun();

	return result;
}

/* -------------------------------------------------------------------------- */
static
int priv_job_give( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	if (priv_job_full(job))
		return E_TIMEOUT;

	priv_job_putUpdate(job, fun);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int job_give( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(job);
	assert(job->obj.res!=RELEASED);
	assert(job->data);
	assert(job->limit);
	assert(fun);

	sys_lock();
	{
		result = priv_job_give(job, fun);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int job_sendFor( job_t *job, fun_t *fun, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(job);
	assert(job->obj.res!=RELEASED);
	assert(job->data);
	assert(job->limit);
	assert(fun);

	sys_lock();
	{
		result = priv_job_give(job, fun);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.job.fun = fun;
			result = core_tsk_waitFor(&job->obj.queue, delay);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int job_sendUntil( job_t *job, fun_t *fun, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(job);
	assert(job->obj.res!=RELEASED);
	assert(job->data);
	assert(job->limit);
	assert(fun);

	sys_lock();
	{
		result = priv_job_give(job, fun);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.job.fun = fun;
			result = core_tsk_waitUntil(&job->obj.queue, time);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
void job_push( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	assert(job);
	assert(job->obj.res!=RELEASED);
	assert(job->data);
	assert(job->limit);
	assert(fun);

	sys_lock();
	{
		priv_job_skipUpdate(job);
		priv_job_putUpdate(job, fun);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
unsigned job_count( job_t *job )
/* -------------------------------------------------------------------------- */
{
	unsigned count;

	assert(job);
	assert(job->obj.res!=RELEASED);

	sys_lock();
	{
		count = job->count;
	}
	sys_unlock();

	return count;
}

/* -------------------------------------------------------------------------- */
unsigned job_space( job_t *job )
/* -------------------------------------------------------------------------- */
{
	unsigned space;

	assert(job);
	assert(job->obj.res!=RELEASED);

	sys_lock();
	{
		space = job->limit - job->count;
	}
	sys_unlock();

	return space;
}

/* -------------------------------------------------------------------------- */
unsigned job_limit( job_t *job )
/* -------------------------------------------------------------------------- */
{
	unsigned limit;

	assert(job);
	assert(job->obj.res!=RELEASED);

	sys_lock();
	{
		limit = job->limit;
	}
	sys_unlock();

	return limit;
}

/* -------------------------------------------------------------------------- */

#if OS_ATOMICS

/* -------------------------------------------------------------------------- */
static
bool priv_job_emptyAsync( job_t *job )
/* -------------------------------------------------------------------------- */
{
	return atomic_load((atomic_size_t *)&job->count) == 0;
}

/* -------------------------------------------------------------------------- */
static
bool priv_job_fullAsync( job_t *job )
/* -------------------------------------------------------------------------- */
{
	return atomic_load((atomic_size_t *)&job->count) == job->limit;
}

/* -------------------------------------------------------------------------- */
static
void priv_job_decAsync( job_t *job )
/* -------------------------------------------------------------------------- */
{
	job->head = job->head + 1 < job->limit ? job->head + 1 : 0;
	atomic_fetch_sub((atomic_size_t *)&job->count, 1);
}

/* -------------------------------------------------------------------------- */
static
void priv_job_incAsync( job_t *job )
/* -------------------------------------------------------------------------- */
{
	job->tail = job->tail + 1 < job->limit ? job->tail + 1 : 0;
	atomic_fetch_add((atomic_size_t *)&job->count, 1);
}

/* -------------------------------------------------------------------------- */
static
fun_t *priv_job_getAsync( job_t *job )
/* -------------------------------------------------------------------------- */
{
	fun_t *fun = job->data[job->head];

	priv_job_decAsync(job);

	return fun;
}

/* -------------------------------------------------------------------------- */
static
void priv_job_putAsync( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	job->data[job->tail] = fun;

	priv_job_incAsync(job);
}

/* -------------------------------------------------------------------------- */
static
int priv_job_takeAsync( job_t *job, fun_t **fun )
/* -------------------------------------------------------------------------- */
{
	if (priv_job_emptyAsync(job))
		return E_TIMEOUT;

	*fun = priv_job_getAsync(job);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int job_takeAsync( job_t *job )
/* -------------------------------------------------------------------------- */
{
	int result;
	fun_t *fun = NULL;

	assert(job);
	assert(job->obj.res!=RELEASED);
	assert(job->data);
	assert(job->limit);

	sys_lock();
	{
		result = priv_job_takeAsync(job, &fun);
	}
	sys_unlock();

	if (fun != NULL)
		fun();

	return result;
}

/* -------------------------------------------------------------------------- */
int job_waitAsync( job_t *job )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();

	while (job_takeAsync(job) != E_SUCCESS)
		core_ctx_switchNow();

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
static
int priv_job_giveAsync( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	if (priv_job_fullAsync(job))
		return E_TIMEOUT;

	priv_job_putAsync(job, fun);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int job_giveAsync( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(job);
	assert(job->obj.res!=RELEASED);
	assert(job->data);
	assert(job->limit);
	assert(fun);

	sys_lock();
	{
		result = priv_job_giveAsync(job, fun);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int job_sendAsync( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();

	while (job_giveAsync(job, fun) != E_SUCCESS)
		core_ctx_switchNow();

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */

#endif//OS_ATOMICS
