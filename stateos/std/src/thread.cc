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
// Modified by Rajmund Szymanski, 10.04.2021

#include <memory> // include this first so <thread> can use shared_ptr
#include <thread>
#include <cerrno>
#include <cxxabi.h>

#ifdef _GLIBCXX_HAS_GTHREADS

namespace std _GLIBCXX_VISIBILITY(default)
{
  extern "C"
  {
    static void
    execute_native_thread_routine(void *ptr)
    {
      auto task = __gthread_self();
      task->state = nullptr;
      task->arg = nullptr;
      static_cast<thread::_State *>(ptr)->_M_run();
      if (task->state)
        reinterpret_cast<thd_t *>(task->state)(task->arg);
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
