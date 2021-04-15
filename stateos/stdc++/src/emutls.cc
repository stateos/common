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

// -----------------------------------------
// Modified by Rajmund Szymanski, 15.04.2021

#include "bits/gthr.h"

#ifdef _GLIBCXX_HAS_GTHREADS

struct __emutls_object
{
  size_t size;
  size_t align;
  union {
    uintptr_t offset;
    void *ptr;
  } loc;
  void *templ;
};

struct __emutls_array
{
  uintptr_t size;
  void **data[1];
};

extern "C"
{
void *__emutls_get_address(void *);
void  __emutls_register_common(void *, size_t, size_t, void *);
}

#ifdef __GTHREADS

#ifdef __GTHREAD_MUTEX_INIT
static __gthread_mutex_t emutls_mutex = __GTHREAD_MUTEX_INIT;
#else
static __gthread_mutex_t emutls_mutex;
#endif
static __gthread_key_t   emutls_key;
static   size_t          emutls_size{};

static void emutls_destroy(void *ptr)
{
  __emutls_array *arr = reinterpret_cast<__emutls_array *>(ptr);
  for (uintptr_t i = 0; i < arr->size; ++i)
  {
    if (arr->data[i])
      free(arr->data[i][-1]);
  }
  free(ptr);
}

static void emutls_init(void)
{
#ifndef __GTHREAD_MUTEX_INIT
  __GTHREAD_MUTEX_INIT_FUNCTION(&emutls_mutex);
#endif
  if (__gthread_key_create(&emutls_key, emutls_destroy) != 0)
    abort();
}

#endif // __GTHREADS

static void *emutls_alloc(__emutls_object *obj)
{
  void *ptr;
  void *ret;
  /* We could use here posix_memalign if available and adjust emutls_destroy accordingly. */
  if (obj->align <= sizeof(void *))
  {
    ptr = malloc(obj->size + sizeof(void *));
    ret = (void *)((uintptr_t)ptr + sizeof(void *));
  }
  else
  {
    ptr = malloc(obj->size + sizeof(void *) + obj->align - 1);
    ret = (void *)(((uintptr_t)ptr + sizeof(void *) + obj->align - 1) & ~(obj->align - 1));
  }

  if (ptr == nullptr)
    abort();

  ((void **)ret)[-1] = ptr;

  if (obj->templ)
    memcpy(ret, obj->templ, obj->size);
  else
    memset(ret, 0, obj->size);

  return ret;
}

void *__emutls_get_address(void *ptr)
{
  __emutls_object *obj = reinterpret_cast<__emutls_object *>(ptr);
#ifndef __GTHREADS
  abort();
#else
  uintptr_t offset = __atomic_load_n(&obj->loc.offset, __ATOMIC_ACQUIRE);

  if (__builtin_expect(offset == 0, 0))
  {
    static __gthread_once_t once = __GTHREAD_ONCE_INIT;
    __gthread_once(&once, emutls_init);
    __gthread_mutex_lock(&emutls_mutex);
    offset = obj->loc.offset;
    if (offset == 0)
    {
      offset = ++emutls_size;
      __atomic_store_n(&obj->loc.offset, offset, __ATOMIC_RELEASE);
    }
    __gthread_mutex_unlock(&emutls_mutex);
  }

  __emutls_array *arr = reinterpret_cast<__emutls_array *>(__gthread_getspecific(emutls_key));
  if (__builtin_expect(arr == nullptr, 0))
  {
    uintptr_t size = offset + 32;
    arr = reinterpret_cast<__emutls_array *>(calloc(size + 1, sizeof(void *)));
    if (arr == nullptr)
      abort();
    arr->size = size;
    __gthread_setspecific(emutls_key, (void *) arr);
  }
  else if (__builtin_expect(offset > arr->size, 0))
  {
    uintptr_t orig_size = arr->size;
    uintptr_t size = orig_size * 2;
    if (offset > size)
      size = offset + 32;
    arr = reinterpret_cast<__emutls_array *>(realloc(arr, (size + 1) * sizeof(void *)));
    if (arr == nullptr)
      abort();
    arr->size = size;
    memset(arr->data + orig_size, 0, (size - orig_size) * sizeof(void *));
    __gthread_setspecific(emutls_key, (void *) arr);
  }

  void *ret = arr->data[offset - 1];
  if (__builtin_expect(ret == nullptr, 0))
  {
    ret = emutls_alloc(obj);
    arr->data[offset - 1] = reinterpret_cast<void **>(ret);
  }
  return ret;
#endif
}

void
__emutls_register_common(void *ptr, size_t size, size_t align, void *templ)
{
  __emutls_object *obj = reinterpret_cast<__emutls_object *>(ptr);
  if (obj->size < size)
  {
    obj->size = size;
    obj->templ = nullptr;
  }
  if (obj->align < align)
    obj->align = align;
  if (templ && size == obj->size)
    obj->templ = templ;
}

#endif // _GLIBCXX_HAS_GTHREADS
