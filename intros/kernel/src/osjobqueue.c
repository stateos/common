/******************************************************************************

    @file    IntrOS: osjobqueue.c
    @author  Rajmund Szymanski
    @date    22.07.2022
    @brief   This file provides set of functions for IntrOS.

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
#include "inc/oscriticalsection.h"
#include "inc/ostask.h"

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
bool priv_job_empty( job_t *job )
/* -------------------------------------------------------------------------- */
{
#if OS_ATOMICS
	return atomic_load((atomic_size_t *)&job->count) == 0;
#else
	return job->count == 0;
#endif
}

/* -------------------------------------------------------------------------- */
static
bool priv_job_full( job_t *job )
/* -------------------------------------------------------------------------- */
{
#if OS_ATOMICS
	return atomic_load((atomic_size_t *)&job->count) == job->limit;
#else
	return job->count == job->limit;
#endif
}

/* -------------------------------------------------------------------------- */
static
void priv_job_dec( job_t *job )
/* -------------------------------------------------------------------------- */
{
	job->head = job->head + 1 < job->limit ? job->head + 1 : 0;
#if OS_ATOMICS
	atomic_fetch_sub((atomic_size_t *)&job->count, 1);
#else
	job->count -= 1;
#endif
}

/* -------------------------------------------------------------------------- */
static
void priv_job_inc( job_t *job )
/* -------------------------------------------------------------------------- */
{
	job->tail = job->tail + 1 < job->limit ? job->tail + 1 : 0;
#if OS_ATOMICS
	atomic_fetch_add((atomic_size_t *)&job->count, 1);
#else
	job->count += 1;
#endif
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
unsigned priv_job_take( job_t *job, fun_t **fun )
/* -------------------------------------------------------------------------- */
{
	if (priv_job_empty(job))
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
		tsk_yield();
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_job_give( job_t *job, fun_t *fun )
/* -------------------------------------------------------------------------- */
{
	if (priv_job_full(job))
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
		tsk_yield();
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
		if (priv_job_empty(job))
			priv_job_dec(job);
		priv_job_put(job, fun);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
unsigned job_getCount( job_t *job )
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
unsigned job_getSpace( job_t *job )
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
unsigned job_getLimit( job_t *job )
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
