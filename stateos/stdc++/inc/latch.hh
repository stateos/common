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
// Modified by Rajmund Szymanski @ StateOS, 23.05.2021

#ifndef _GLIBCXX_LATCH
#define _GLIBCXX_LATCH 1

#pragma GCC system_header

#if __cplusplus > 201703L
#include "critical_section.hh"

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  #define __cpp_lib_latch 201907L

  class latch
  {
  public:
    static constexpr ptrdiff_t
    max() noexcept
    { return __PTRDIFF_MAX__; }

    constexpr explicit latch(ptrdiff_t __expected) noexcept
    : _M_latch(__expected), _M_wait(nullptr)
	{ assert(__expected >= 0 && __expected <= max()); }

	~latch()
	{ assert(_M_wait == nullptr); }

    latch(const latch&) = delete;
    latch& operator=(const latch&) = delete;

    void
    count_down(ptrdiff_t __update = 1) noexcept
    {
      assert(__update >= 0 && __update <= _M_latch);
      critical_section cs;
      _M_latch -= __update;
      if (_M_latch == 0)
        core_all_wakeup(_M_wait, E_SUCCESS);
    }

    bool
    try_wait() noexcept
    {
      critical_section cs;
      return _M_latch == 0;
    }

    void
    wait() noexcept
    {
      critical_section cs;
      if (_M_latch != 0)
        core_tsk_waitFor(&_M_wait, INFINITE);
    }

    void
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
