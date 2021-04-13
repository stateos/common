// thread -*- C++ -*-

// Copyright (C) 2008-2021 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

// -----------------------------------------
// Modified by Rajmund Szymanski, 13.04.2021

#include <memory> // include this first so <thread> can use shared_ptr
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <system_error>
#include <cxxabi.h>
#include <cerrno>

#ifdef _GLIBCXX_HAS_GTHREADS

struct oskey_t : private std::unordered_map<__gthread_t, void *>
{
  oskey_t(void (*func)(void *)) : dtor{func} {}

  void *get(__gthread_t task);
  void  set(__gthread_t task, void *ptr);
  void  run(__gthread_t task);

  private:
  void (*dtor)(void *);
};

void *oskey_t::get(__gthread_t task)
{
  auto it = find(task);
  return it != end() ? it->second : nullptr;
}

void oskey_t::set(__gthread_t task, void *ptr)
{
  insert_or_assign(task, ptr);
}

void oskey_t::run(__gthread_t task)
{
  auto it = find(task);
  if (it != end())
  {
    if (dtor && it->second)
      dtor(it->second);
    erase(it);
  }
}

struct oskey_set_t : private std::unordered_set<oskey_t *>
{
  oskey_t *add(void(*dtor)(void *));
  size_t   del(oskey_t *key);
  void     run(__gthread_t task);

  std::mutex mtx;
};

oskey_t *oskey_set_t::add(void(*dtor)(void *))
{
  auto key = new oskey_t(dtor);
  if (key)
  {
    if (insert(key).second)
      return key;
    delete key;
  }
  return nullptr;
}

size_t oskey_set_t::del(oskey_t *key)
{
  return erase(key);
}

void oskey_set_t::run(__gthread_t task)
{
  for (auto key : *this)
    key->run(task);
}

static oskey_set_t keys{};

int __gthread_key_create(__gthread_key_t *keyp, void(*dtor)(void *))
{
  std::lock_guard<std::mutex> lock(keys.mtx);
  assert(keyp);
  *keyp = keys.add(dtor);
  return *keyp == nullptr;
}

int __gthread_key_delete(__gthread_key_t key)
{
  std::lock_guard<std::mutex> lock(keys.mtx);
  assert(key);
  return keys.del(key) == 0;
}

void *__gthread_getspecific(__gthread_key_t key)
{
  std::lock_guard<std::mutex> lock(keys.mtx);
  assert(key);
  return key->get(__gthread_self());
}

int __gthread_setspecific(__gthread_key_t key, const void *ptr)
{
  std::lock_guard<std::mutex> lock(keys.mtx);
  assert(key);
  key->set(__gthread_self(), const_cast<void *>(ptr));
  return 0;
}

int __gthread_atexit(__gthread_t task)
{
  std::lock_guard<std::mutex> lock(keys.mtx);
  keys.run(task);
  return 0;
}

namespace std _GLIBCXX_VISIBILITY(default)
{
  extern "C"
  {
    static void
    execute_native_thread_routine(void *ptr)
    {
      static_cast<thread::_State *>(ptr)->_M_run();
      __gthread_atexit(__gthread_self());
    }
  } // extern "C"

_GLIBCXX_BEGIN_NAMESPACE_VERSION

  thread::_State::~_State() = default;

  void
  thread::join()
  {
    int __e = EINVAL;

    if (_M_id != id())
      __e = __gthread_join(_M_id._M_thread, nullptr);

    if (__e)
      __throw_system_error(__e);

    _M_id = id();
  }

  void
  thread::detach()
  {
    int __e = EINVAL;

    if (_M_id != id())
      __e = __gthread_detach(_M_id._M_thread);

    if (__e)
      __throw_system_error(__e);

    _M_id = id();
  }

  void
  thread::_M_start_thread(_State_ptr state, void (*)())
  {
    const int err = __gthread_create(&_M_id._M_thread,
                                     execute_native_thread_routine,
                                     state.get());
    if (err)
      __throw_system_error(err);
    state.release();
  }

  unsigned int
  thread::hardware_concurrency() noexcept
  {
    return 0;
  }

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace std

#endif // _GLIBCXX_HAS_GTHREADS

#ifndef _GLIBCXX_NO_SLEEP
namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION
namespace this_thread
{
  void
  __sleep_for(chrono::seconds __s, chrono::nanoseconds __ns)
  {
    tsk_sleepFor(chrono::systick::count(__s) +
                 chrono::systick::count(__ns));
  }
}
_GLIBCXX_END_NAMESPACE_VERSION
} // namespace std
#endif // ! NO_SLEEP
