// <latch> -*- C++ -*-

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
// Modified by Rajmund Szymanski @ StateOS, 21.05.2021

#ifndef _GLIBCXX_LATCH
#define _GLIBCXX_LATCH 1

#pragma GCC system_header

#if __cplusplus > 201703L
#include "inc/ostask.h"
#include "inc/critical_section.hh"
#include <limits>

#ifndef  _GLIBCXX_ALWAYS_INLINE
#define  _GLIBCXX_ALWAYS_INLINE inline __attribute__((__always_inline__))
#endif

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  #define __cpp_lib_latch 201907L

  class latch
  {
  public:
    static constexpr ptrdiff_t
    max() noexcept
    { return numeric_limits<ptrdiff_t>::max(); }

    constexpr explicit latch(ptrdiff_t __expected) noexcept
    : _M_latch(__expected), _M_wait(nullptr) { }

    ~latch() = default;
    latch(const latch&) = delete;
    latch& operator=(const latch&) = delete;

    _GLIBCXX_ALWAYS_INLINE void
    count_down(ptrdiff_t __update = 1)
    {
      critical_section cs;
      _M_latch -= __update;
      if (_M_latch == 0)
	    core_all_wakeup(_M_wait, E_SUCCESS);
    }

    _GLIBCXX_ALWAYS_INLINE bool
    try_wait() const noexcept
    { return _M_latch == 0; }

    _GLIBCXX_ALWAYS_INLINE void
    wait() const noexcept
    {
      if (!try_wait())
      {
        critical_section cs;
      	core_tsk_waitFor(const_cast<tsk_t **>(&_M_wait), INFINITE);
      }
    }

    _GLIBCXX_ALWAYS_INLINE void
    arrive_and_wait(ptrdiff_t __update = 1) noexcept
    {
      count_down(__update);
      wait();
    }

  private:
    ptrdiff_t _M_latch;
    tsk_t    *_M_wait;
  };
_GLIBCXX_END_NAMESPACE_VERSION
} // namespace
#endif // C++20
#endif // _GLIBCXX_LATCH