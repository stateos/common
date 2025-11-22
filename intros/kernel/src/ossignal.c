/******************************************************************************

    @file    IntrOS: ossignal.c
    @author  Rajmund Szymanski
    @date    19.11.2025
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

#include "inc/ossignal.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_sig_init( sig_t *sig )
/* -------------------------------------------------------------------------- */
{
	memset(sig, 0, sizeof(sig_t));
}

/* -------------------------------------------------------------------------- */
void sig_init( sig_t *sig )
/* -------------------------------------------------------------------------- */
{
	assert(sig);

	sys_lock();
	{
		priv_sig_init(sig);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
void priv_sig_reset( sig_t *sig, int event )
/* -------------------------------------------------------------------------- */
{
	sig->sigset = 0;

	core_all_wakeup(&sig->obj.queue, event);
}

/* -------------------------------------------------------------------------- */
void sig_reset( sig_t *sig )
/* -------------------------------------------------------------------------- */
{
	assert(sig);

	sys_lock();
	{
		priv_sig_reset(sig, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
unsigned priv_sig_signo( unsigned sigset )
{
	unsigned signo;

	for (signo = 0; sigset >>= 1; signo++);

	return signo;
}

/* -------------------------------------------------------------------------- */
static
int priv_sig_take( sig_t *sig, unsigned sigset, unsigned *signo )
/* -------------------------------------------------------------------------- */
{
	sigset &= sig->sigset;

	if (sigset)
	{
		sigset &= -sigset;
		sig->sigset &= ~sigset;
		if (signo != NULL)
			*signo = priv_sig_signo(sigset);
		return E_SUCCESS;
	}

	return E_TIMEOUT;
}

/* -------------------------------------------------------------------------- */
int sig_take( sig_t *sig, unsigned sigset, unsigned *signo )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(sig);

	sys_lock();
	{
		result = priv_sig_take(sig, sigset, signo);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int sig_waitFor( sig_t *sig, unsigned sigset, unsigned *signo, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(sig);

	sys_lock();
	{
		result = priv_sig_take(sig, sigset, signo);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.sig.sigset = sigset;
			result = core_tsk_waitFor(&sig->obj.queue, delay);
			if (result == E_SUCCESS && signo != NULL)
				*signo = System.cur->tmp.sig.signo;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int sig_waitUntil( sig_t *sig, unsigned sigset, unsigned *signo, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(sig);

	sys_lock();
	{
		result = priv_sig_take(sig, sigset, signo);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.sig.sigset = sigset;
			result = core_tsk_waitUntil(&sig->obj.queue, time);
			if (result == E_SUCCESS && signo != NULL)
				*signo = System.cur->tmp.sig.signo;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
void sig_give( sig_t *sig, unsigned signo )
/* -------------------------------------------------------------------------- */
{
	unsigned sigset = SIGSET(signo);
	tsk_t ** que;

	assert(sig);

	sys_lock();
	{
		sig->sigset |= sigset;

		que = &sig->obj.queue;
		while (*que)
		{
			if (((*que)->tmp.sig.sigset & sigset) != 0 || (*que)->tmp.sig.sigset == 0)
			{
				sig->sigset &= ~sigset;
				(*que)->tmp.sig.signo = signo;
				core_one_wakeup(que, E_SUCCESS);
				continue;
			}
			que = &(*que)->obj.queue;
		}
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void sig_clear( sig_t *sig, unsigned signo )
/* -------------------------------------------------------------------------- */
{
	unsigned sigset = SIGSET(signo);

	assert(sig);

	sys_lock();
	{
		sig->sigset &= ~sigset;
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
