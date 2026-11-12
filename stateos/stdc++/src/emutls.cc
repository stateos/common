/******************************************************************************

    @file    StateOS: emutls.cc
    @author  Rajmund Szymanski
    @date    08.11.2025
    @brief   This file provides set of variables and functions for StateOS.

 ******************************************************************************

   Copyright (c) 2018-2025 Rajmund Szymanski. All rights reserved.

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

#include <mutex>
#include <unordered_map>
#include <new>
#include <cstdlib>
#include <system_error>
#include <malloc.h>
#include "bits/gthr.h"

#ifdef _GLIBCXX_HAS_GTHREADS

extern "C"
{
#ifndef EMUTLS_ATTR
#define EMUTLS_ATTR
#endif

EMUTLS_ATTR
void *__emutls_get_address(void *);

EMUTLS_ATTR
void  __emutls_register_common(void *, size_t, size_t, void *);
}

static std::mutex emutls_mutex{};

struct __emutls_object
{
	size_t size;  // object size
	size_t align; // object alignment
	void  *key;   // __gthread_key_t
	void  *init;  // initial value
};

static void *emutls_alloc(__emutls_object *obj)
{
	void *ptr = ::memalign(obj->align, obj->size);
	if (ptr == nullptr)
		return nullptr;
	if (obj->init == nullptr)
		return ::memset(ptr, 0, obj->size);
	else
		return ::memcpy(ptr, obj->init, obj->size);
}

void *__emutls_get_address(void *ptr)
{
	__emutls_object *obj = static_cast<__emutls_object *>(ptr);
	__gthread_key_t key = static_cast<__gthread_key_t>(obj->key);
	if (key == nullptr)
	{
		std::lock_guard<std::mutex> lock(emutls_mutex);
		key = static_cast<__gthread_key_t>(obj->key);
		if (key == nullptr)
		{
			if (__gthread_key_create(&key, ::free) != 0)
				std::__throw_system_error(ENOMEM);
			obj->key = static_cast<void *>(key);
		}
	}
	void *address = __gthread_getspecific(key);
	if (address == nullptr)
	{
		address = emutls_alloc(obj);
		if (address == nullptr || __gthread_setspecific(key, address) != 0)
			std::__throw_system_error(ENOMEM);
	}
	return address;
}

void __emutls_register_common(void *ptr, size_t size, size_t align, void *init)
{
	__emutls_object *obj = static_cast<__emutls_object *>(ptr);
	if (obj->size < size)
	{
		obj->size = size;
		obj->init = init;
	}
	if (obj->align < align)
		obj->align = align;
}

// destructors of the thread_local objects: a list per thread
// (__cxa_thread_atexit of a toolchain with the single thread model keeps one list and runs it at exit)

struct __emutls_dtor
{
	void (*dtor)(void *);
	void  *obj;
	__emutls_dtor *next;
};

static std::unordered_map<__gthread_t, __emutls_dtor *> emutls_dtors{};

// call the destructors of the thread_local objects of the current thread, in the reverse order of the construction
// called at the exit of the thread (thread.cc) and of the program
void __emutls_thread_atexit()
{
	for (;;) // a destructor can construct another thread_local object
	{
		__emutls_dtor *elt;
		{
			std::lock_guard<std::mutex> lock(emutls_mutex);
			auto it = emutls_dtors.find(__gthread_self());
			if (it == emutls_dtors.end())
				return;
			elt = it->second;
			emutls_dtors.erase(it);
		}
		while (elt != nullptr) // without the mutex: a destructor can use the thread_local variables
		{
			__emutls_dtor *next = elt->next;
			elt->dtor(elt->obj);
			delete elt;
			elt = next;
		}
	}
}

extern "C"
int __cxa_thread_atexit(void (*dtor)(void *), void *obj, void * /*dso_handle*/)
{
	static bool queued = false;
	__emutls_dtor *elt = new (std::nothrow) __emutls_dtor{ dtor, obj, nullptr };
	if (elt == nullptr)
		return -1;
	bool first;
	{
		std::lock_guard<std::mutex> lock(emutls_mutex);
		__emutls_dtor *&head = emutls_dtors[__gthread_self()];
		elt->next = head;
		head = elt;
		first = !queued;
		queued = true;
	}
	if (first)
		std::atexit(__emutls_thread_atexit); // the thread that calls exit (it does not return from its function)
	return 0;
}

#if __GNUC__ >= 15

extern"C"
void *__aeabi_read_tp()
{
	static char tls[OS_STACK_SIZE] __attribute__((aligned(8)));
	return tls;
}

#endif

#endif // _GLIBCXX_HAS_GTHREADS
