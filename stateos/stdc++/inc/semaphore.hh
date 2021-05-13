// <semaphore> -*- C++ -*-

// Copyright (C) 2020-2021 Free Software Foundation, Inc.
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
// Modified by Rajmund Szymanski @ StateOS, 13.05.2021

#ifndef _GLIBCXX_SEMAPHORE
#define _GLIBCXX_SEMAPHORE 1

#pragma GCC system_header

#if __cplusplus > 201703L
#include "inc/ossemaphore.h"
#include "inc/chrono.hh"

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  using __semaphore_impl = sem_t;

  #define __cpp_lib_semaphore 201907L

  template<ptrdiff_t __least_max_value = numeric_limits<ptrdiff_t>::max()>
  class counting_semaphore
  {
    static_assert(__least_max_value >= 0);
    static_assert(__least_max_value <= semCounting);

    __semaphore_impl _M_sem;

  public:
    explicit counting_semaphore(ptrdiff_t __desired = 0) noexcept
    : _M_sem(_SEM_INIT(__desired, __least_max_value))
    { }

    ~counting_semaphore() = default;

    counting_semaphore(const counting_semaphore&) = delete;
    counting_semaphore& operator=(const counting_semaphore&) = delete;

    static constexpr ptrdiff_t
    max() noexcept
    { return __least_max_value; }

    void
    release(ptrdiff_t __update = 1) noexcept(noexcept(sem_release(&_M_sem, 1)))
    { sem_release(&_M_sem, static_cast<unsigned>(__update)); }

    void
    acquire() noexcept(noexcept(sem_wait(&_M_sem)))
    { sem_wait(&_M_sem); }

    bool
    try_acquire() noexcept(noexcept(sem_tryWait(&_M_sem)))
    { return sem_tryWait(&_M_sem) == E_SUCCESS; }

    template<typename _Rep, typename _Period>
    bool
    try_acquire_for(const std::chrono::duration<_Rep, _Period>& __rtime)
    { return sem_waitFor(&_M_sem, chrono::systick::count(__rtime)) == E_SUCCESS; }

    template<typename _Clock, typename _Dur>
    bool
    try_acquire_until(const std::chrono::time_point<_Clock, _Dur>& __atime)
    { return sem_waitUntil(&_M_sem, chrono::systick::until(__atime)) == E_SUCCESS; }
  };

  using direct_semaphore = std::counting_semaphore<0>;
  using binary_semaphore = std::counting_semaphore<1>;
  using        semaphore = std::counting_semaphore< >;

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace
#endif // C++20
#endif // _GLIBCXX_SEMAPHORE
