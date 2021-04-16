/* TLS emulation.
   Copyright (C) 2006-2021 Free Software Foundation, Inc.
   Contributed by Jakub Jelinek <jakub@redhat.com>.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

// ---------------------------------------------------
// Modified by Rajmund Szymanski @ StateOS, 16.04.2021

#include <atomic>
#include <mutex>
#include <vector>
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
  union {
    uintptr_t index;
    void *address;
  } loc;
  void *value;  // initial value
};

using __emutls_array = std::vector<void *>;

static __gthread_key_t emutls_key{};
static std::once_flag  emutls_once{};
static std::mutex      emutls_mutex{};

static void *emutls_alloc(__emutls_object *obj)
{
  void *ptr = memalign(obj->align, obj->size);
  if (ptr == nullptr)
    abort();
  if (obj->value)
    memcpy(ptr, obj->value, obj->size);
  else
    memset(ptr, 0, obj->size);
  return obj->loc.address = ptr;
}

static void emutls_free(void *ptr)
{
  __emutls_array *arr = reinterpret_cast<__emutls_array *>(ptr);
  for (void *p: *arr)
    if (p)
      free(p);
  delete arr;
}

static void emutls_init()
{
  int err = __gthread_key_create(&emutls_key, emutls_free);
  if (err)
    abort();
}

void *__emutls_get_address(void *ptr)
{
  std::call_once(emutls_once, emutls_init);
  __emutls_array *arr = reinterpret_cast<__emutls_array *>(__gthread_getspecific(emutls_key));
  if (arr == nullptr)
  {
    arr = new __emutls_array;
    __gthread_setspecific(emutls_key, reinterpret_cast<void *>(arr));
  }
  __emutls_object *obj = reinterpret_cast<__emutls_object *>(ptr);
  uintptr_t index = std::atomic_load_explicit(reinterpret_cast<std::atomic_uintptr_t *>(&obj->loc.index),
                                                               std::memory_order_acquire);
  if (!index)
  {
    std::lock_guard<std::mutex> lock(emutls_mutex);
    index = obj->loc.index;
    if (!index)
    {
      void *ret = emutls_alloc(obj);
      arr->push_back(ret);
      std::atomic_store_explicit(reinterpret_cast<std::atomic_uintptr_t *>(&obj->loc.index), arr->size(),
                                                  std::memory_order_release);
      return ret;
    }
  }
  return (*arr)[index - 1];
}

void __emutls_register_common(void *ptr, size_t size, size_t align, void *value)
{
  __emutls_object *obj = reinterpret_cast<__emutls_object *>(ptr);
  if (obj->size < size)
  {
    obj->size = size;
    obj->value = value;
  }
  if (obj->align < align)
    obj->align = align;
}

#endif // _GLIBCXX_HAS_GTHREADS
