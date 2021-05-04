/******************************************************************************

    @file    IntrOS: osjobqueue.c
    @author  Rajmund Szymanski
    @date    04.05.2021
    @brief   This file provides set of functions for IntrOS.

 ******************************************************************************

   Copyright (c) 2018-2021 Rajmund Szymanski. All rights reserved.

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
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
void job_init( job_t *job, fun_t **data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert(job);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		memset(job, 0, sizeof(job_t));

		job->limit = bufsize / sizeof(fun_t *);
		job->data  = data;
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
fun_t *priv_job_get( job_t *job )
/* -------------------------------------------------------------------------- */
{
	unsigned head = job->head;

	fun_t *fun = job->data[head++];

	job->head = head < job->limit ? head : 0;
	job->count--;

	return fun;
}

/* -------------------------------------------------------------------------- */
static
void priv_job_put( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	unsigned tail = job->tail;

	job->data[tail++] = fun;

	job->tail = tail < job->limit ? tail : 0;
	job->count++;
}

/* -------------------------------------------------------------------------- */
static
void priv_job_skip( job_t *job )
/* -------------------------------------------------------------------------- */
{
	unsigned head = job->head + 1;

	job->head = head < job->limit ? head : 0;
	job->count--;
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_job_take( job_t *job, fun_t **fun )
/* -------------------------------------------------------------------------- */
{
	if (job->count == 0)
		return FAILURE;

	*fun = priv_job_get(job);

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned job_take( job_t *job )
/* -------------------------------------------------------------------------- */
{
	unsigned result;
	fun_t *fun = NULL;

	assert(job);
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
void job_wait( job_t *job )
/* -------------------------------------------------------------------------- */
{
	while (job_take(job) != SUCCESS)
		core_ctx_switch();
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_job_give( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	if (job->count == job->limit)
		return FAILURE;

	priv_job_put(job, fun);

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned job_give( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	assert(job);
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
void job_send( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	while (job_give(job, fun) != SUCCESS)
		core_ctx_switch();
}

/* -------------------------------------------------------------------------- */
void job_push( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	assert(job);
	assert(job->data);
	assert(job->limit);
	assert(fun);

	sys_lock();
	{
		if (job->count == job->limit)
			priv_job_skip(job);
		priv_job_put(job, fun);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
unsigned job_count( job_t *job )
/* -------------------------------------------------------------------------- */
{
	unsigned count;

	assert(job);

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
fun_t *priv_job_getAsync( job_t *job )
/* -------------------------------------------------------------------------- */
{
	unsigned head = job->head;

	fun_t *fun = job->data[head++];

	job->head = head < job->limit ? head : 0;
	atomic_fetch_sub((atomic_uint *)&job->count, 1);

	return fun;
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_job_takeAsync( job_t *job, fun_t **fun )
/* -------------------------------------------------------------------------- */
{
	if (atomic_load((atomic_uint *)&job->count) == 0)
		return FAILURE;

	*fun = priv_job_getAsync(job);

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned job_takeAsync( job_t *job )
/* -------------------------------------------------------------------------- */
{
	unsigned result;
	fun_t *fun = NULL;

	assert(job);
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
void job_waitAsync( job_t *job )
/* -------------------------------------------------------------------------- */
{
	while (job_takeAsync(job) != SUCCESS)
		core_ctx_switch();
}

/* -------------------------------------------------------------------------- */
static
void priv_job_putAsync( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	unsigned tail = job->tail;

	job->data[tail++] = fun;

	job->tail = tail < job->limit ? tail : 0;
	atomic_fetch_add((atomic_uint *)&job->count, 1);
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_job_giveAsync( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	if (atomic_load((atomic_uint *)&job->count) == job->limit)
		return FAILURE;

	priv_job_putAsync(job, fun);

	return SUCCESS;
}

/* -------------------------------------------------------------------------- */
unsigned job_giveAsync( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	unsigned result;

	assert(job);
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
void job_sendAsync( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	while (job_giveAsync(job, fun) != SUCCESS)
		core_ctx_switch();
}

/* -------------------------------------------------------------------------- */

#endif//OS_ATOMICS
