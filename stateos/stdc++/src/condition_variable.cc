// condition_variable -*- C++ -*-

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

#include <condition_variable>
#include <cstdlib>

#ifdef _GLIBCXX_HAS_GTHREADS

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  condition_variable::condition_variable() noexcept = default;

  condition_variable::~condition_variable() noexcept = default;

  void
  condition_variable::wait(unique_lock<mutex>& __lock)
  {
    cnd_wait(_M_cond.native_handle(), __lock.mutex()->native_handle());
  }

  void
  condition_variable::notify_one() noexcept
  {
    cnd_notifyOne(_M_cond.native_handle());
  }

  void
  condition_variable::notify_all() noexcept
  {
    cnd_notifyAll(_M_cond.native_handle());
  }

  extern void
  __at_thread_exit(__at_thread_exit_elt*);

  namespace
  {
    __gthread_key_t key;

    void run(void* p)
    {
      auto elt = (__at_thread_exit_elt*)p;
      while (elt)
	{
	  auto next = elt->_M_next;
	  elt->_M_cb(elt);
	  elt = next;
	}
    }

    void run()
    {
      auto elt = (__at_thread_exit_elt*)__gthread_getspecific(key);
      __gthread_setspecific(key, nullptr);
      run(elt);
    }

    struct notifier final : __at_thread_exit_elt
    {
      notifier(condition_variable& v, unique_lock<mutex>& l)
      : cv(&v), mx(l.release())
      {
	_M_cb = &notifier::run;
	__at_thread_exit(this);
      }

      ~notifier()
      {
	mx->unlock();
	cv->notify_all();
      }

      condition_variable* cv;
      mutex* mx;

      static void run(void* p) { delete static_cast<notifier*>(p); }
    };


    void key_init() {
      struct key_s {
	key_s() { __gthread_key_create (&key, run); }
	~key_s() { __gthread_key_delete (key); }
      };
      static key_s ks;
      // Also make sure the callbacks are run by std::exit.
      std::atexit (run);
    }
  }

  void
  __at_thread_exit(__at_thread_exit_elt* elt)
  {
    static __gthread_once_t once = __GTHREAD_ONCE_INIT;
    __gthread_once (&once, key_init);

    elt->_M_next = (__at_thread_exit_elt*)__gthread_getspecific(key);
    __gthread_setspecific(key, elt);
  }

  void
  notify_all_at_thread_exit(condition_variable& cv, unique_lock<mutex> l)
  {
    (void) new notifier{cv, l};
  }

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace

#endif // _GLIBCXX_HAS_GTHREADS
