// thread -*- C++ -*-

// Copyright (C) 2008-2025 Free Software Foundation, Inc.
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

// ---------------------------------------------------
// Modified by Rajmund Szymanski @ StateOS, 08.11.2025

#include <thread>
#include <mutex>
#include <unordered_map>
#include <system_error>
#include <cxxabi.h>

#ifdef _GLIBCXX_HAS_GTHREADS

static std::mutex key_mutex{};

struct oskey_t : public std::unordered_map<__gthread_t, void *>
{
  using unordered_map<__gthread_t, void *>::unordered_map;
};

static std::unordered_map<__gthread_key_t, void (*)(void *)> key_map{};

int __gthread_key_create(__gthread_key_t *keyp, void (*dtor)(void *))
{
  assert(keyp);
  std::lock_guard<std::mutex> lock(key_mutex);
#if __cplusplus < 201402L
  auto ptr = std::unique_ptr<oskey_t>(new oskey_t());
#else
  auto ptr = std::make_unique<oskey_t>();
#endif
  if (!ptr || !key_map.insert({ ptr.get(), dtor }).second)
    return 1;
  *keyp = ptr.release();
  return 0;
}

int __gthread_key_delete(__gthread_key_t key)
{
  assert(key);
  std::lock_guard<std::mutex> lock(key_mutex);
  auto ptr = std::unique_ptr<oskey_t>(key);
  return key_map.erase(ptr.get()) != 0 ? 0 : 1;
}

void *__gthread_getspecific(__gthread_key_t key)
{
  assert(key);
  std::lock_guard<std::mutex> lock(key_mutex);
  auto it = key->find(__gthread_self());
  return it != key->end() ? it->second : nullptr;
}

int __gthread_setspecific(__gthread_key_t key, const void *ptr)
{
  assert(key);
  std::lock_guard<std::mutex> lock(key_mutex);
#if __cplusplus < 201703L
  (*key)[__gthread_self()] = const_cast<void *>(ptr);
#else
  key->insert_or_assign(__gthread_self(), const_cast<void *>(ptr));
#endif
  return 0;
}

static void __gthread_atexit()
{
  std::lock_guard<std::mutex> lock(key_mutex);
  auto task = __gthread_self();
#if __cplusplus < 201703L
  for (auto const& item : key_map)
  {
    auto const& key = item.first;
    auto const& dtor = item.second;
#else
  for (auto const& [key, dtor] : key_map)
  {
#endif
    auto const& it = key->find(task);
    if (it != key->end())
    {
      void *ptr = it->second;
      if (dtor != nullptr && ptr != nullptr)
        dtor(ptr);
      key->erase(it);
    }
  }
}

namespace std _GLIBCXX_VISIBILITY(default)
{
  extern "C"
  {
    static void
    execute_native_thread_routine(void* __p)
    {
      thread::_State_ptr __t{ static_cast<thread::_State*>(__p) };
      __t->_M_run();
      __gthread_atexit();
    }

#if _GLIBCXX_THREAD_ABI_COMPAT
    static void
    execute_native_thread_routine_compat(void* __p)
    {
      thread::_Impl_base* __t = static_cast<thread::_Impl_base*>(__p);
      thread::__shared_base_type __local;
      // Now that a new thread has been created we can transfer ownership of
      // the thread state to a local object, breaking the reference cycle
      // created in thread::_M_start_thread.
      __local.swap(__t->_M_this_ptr);
      __t->_M_run();
      __gthread_atexit();
    }
#endif
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
				     &execute_native_thread_routine,
				     state.get());
    if (err)
      __throw_system_error(err);
    state.release();
  }

#if _GLIBCXX_THREAD_ABI_COMPAT
  void
  thread::_M_start_thread(__shared_base_type __b)
  {
    _M_start_thread(std::move(__b), nullptr);
  }

  void
  thread::_M_start_thread(__shared_base_type __b, void (*)())
  {
    auto ptr = __b.get();
    // Create a reference cycle that will be broken in the new thread.
    ptr->_M_this_ptr = std::move(__b);
    int __e = __gthread_create(&_M_id._M_thread,
			       &execute_native_thread_routine_compat, ptr);
    if (__e)
    {
      ptr->_M_this_ptr.reset();  // break reference cycle, destroying *ptr.
      __throw_system_error(__e);
    }
  }
#endif

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
