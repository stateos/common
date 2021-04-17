/******************************************************************************

    @file    StateOS: emutls.cc
    @author  Rajmund Szymanski
    @date    17.04.2021
    @brief   This file provides set of variables and functions for StateOS.

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

#include <mutex>
#include <unordered_map>
#include <system_error>
#include <malloc.h>
#include "bits/gthr.h"

#ifdef _GLIBCXX_HAS_GTHREADS

extern "C"
{
void *__emutls_get_address(void *);
void  __emutls_register_common(void *, size_t, size_t, void *);
}

struct __emutls_object
{
  size_t size;  // object size
  size_t align; // object alignment
  void  *key;   // __gthread_key_t
  void  *init;  // initial value
};

static std::mutex emutls_mutex{};

static void *emutls_alloc(__emutls_object *obj)
{
  void *ptr = ::memalign(obj->align, obj->size);
  if (ptr == nullptr)
    std::__throw_system_error(ENOMEM);
  if (obj->init)
    memcpy(ptr, obj->init, obj->size);
  else
    memset(ptr, 0, obj->size);
  return ptr;
}

void *__emutls_get_address(void *ptr)
{
  __emutls_object *obj = static_cast<__emutls_object *>(ptr);
  __gthread_key_t  key = static_cast<__gthread_key_t>(obj->key);
  if (!key)
  {
    std::lock_guard<std::mutex> lock(emutls_mutex);
    key = static_cast<__gthread_key_t>(obj->key);
    if (!key)
    {
      if (__gthread_key_create(&key, ::free) != 0)
        std::__throw_system_error(ENOMEM);
      obj->key = static_cast<void *>(key);
    }
  }
  void *address = __gthread_getspecific(key);
  if (!address)
  {
    address = emutls_alloc(obj);
    if (__gthread_setspecific(key, address) != 0)
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

#endif // _GLIBCXX_HAS_GTHREADS
