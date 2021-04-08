/******************************************************************************

    @file    StateOS: key.hh
    @author  Rajmund Szymanski
    @date    08.04.2021
    @brief   This file contains definitions for StateOS.

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

#ifndef __STATEOS_KEY_HH
#define __STATEOS_KEY_HH

static inline
int __gthread_key_create(__gthread_key_t *keyp, void(*dtor)(void *))
{
	return 0;
}

static inline
int __gthread_key_delete(__gthread_key_t key)
{
	return 0;
}

static inline
void *__gthread_getspecific(__gthread_key_t key)
{
	return nullptr;
}

static inline
int __gthread_setspecific(__gthread_key_t key, const void *ptr)
{
	return 0;
}

#endif//__STATEOS_KEY_HH
