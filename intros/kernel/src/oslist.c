/******************************************************************************

    @file    IntrOS: oslist.c
    @author  Rajmund Szymanski
    @date    07.05.2021
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

#include "inc/oslist.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_lst_init( lst_t *lst )
/* -------------------------------------------------------------------------- */
{
	memset(lst, 0, sizeof(lst_t));
}

/* -------------------------------------------------------------------------- */
void lst_init( lst_t *lst )
/* -------------------------------------------------------------------------- */
{
	assert(lst);

	sys_lock();
	{
		priv_lst_init(lst);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
void priv_lst_reset( lst_t *lst, int event )
/* -------------------------------------------------------------------------- */
{
	core_all_wakeup(&lst->obj.queue, event);
}

/* -------------------------------------------------------------------------- */
void lst_reset( lst_t *lst )
/* -------------------------------------------------------------------------- */
{
	assert(lst);

	sys_lock();
	{
		priv_lst_reset(lst, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
bool priv_lst_empty( lst_t *lst )
/* -------------------------------------------------------------------------- */
{
	return lst->head.next == NULL;
}

/* -------------------------------------------------------------------------- */
static
void *priv_lst_popFront( lst_t *lst )
/* -------------------------------------------------------------------------- */
{
	que_t *ptr = lst->head.next;
	lst->head.next = ptr->next;
	return ptr + 1;
}

/* -------------------------------------------------------------------------- */
static
int priv_lst_take( lst_t *lst, void **data )
/* -------------------------------------------------------------------------- */
{
	if (priv_lst_empty(lst))
		return E_TIMEOUT;

	*data = priv_lst_popFront(lst);

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int lst_take( lst_t *lst, void **data )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(lst);
	assert(data);

	sys_lock();
	{
		result = priv_lst_take(lst, data);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int lst_waitFor( lst_t *lst, void **data, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(lst);
	assert(data);

	sys_lock();
	{
		result = priv_lst_take(lst, data);
		if (result == E_TIMEOUT)
		{
			result = core_tsk_waitFor(&lst->obj.queue, delay);
			if (result == E_SUCCESS)
				*data = System.cur->tmp.lst.data;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int lst_waitUntil( lst_t *lst, void **data, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(lst);
	assert(data);

	sys_lock();
	{
		result = priv_lst_take(lst, data);
		if (result == E_TIMEOUT)
		{
			result = core_tsk_waitUntil(&lst->obj.queue, time);
			if (result == E_SUCCESS)
				*data = System.cur->tmp.lst.data;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
static
void priv_lst_pushBack( lst_t *lst, void *data )
/* -------------------------------------------------------------------------- */
{
	que_t *ptr = &lst->head;
	while (ptr->next) ptr = ptr->next;
	ptr = ptr->next = (que_t *)data - 1;
	ptr->next = NULL;
}

/* -------------------------------------------------------------------------- */
static
void priv_lst_give( lst_t *lst, void *data )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk = core_one_wakeup(&lst->obj.queue, E_SUCCESS);

	if (tsk)
	{
		tsk->tmp.lst.data = data;
	}
	else
	{
		priv_lst_pushBack(lst, data);
	}
}

/* -------------------------------------------------------------------------- */
void lst_give( lst_t *lst, void *data )
/* -------------------------------------------------------------------------- */
{
	assert(lst);
	assert(data);

	sys_lock();
	{
		priv_lst_give(lst, data);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
