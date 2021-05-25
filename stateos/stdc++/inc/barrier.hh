// <barrier> -*- C++ -*-

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

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

// This implementation is based on libcxx/include/barrier
//===-- barrier.h --------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------===//
// Modified by Rajmund Szymanski @ StateOS, 25.05.2021

#ifndef _GLIBCXX_BARRIER
#define _GLIBCXX_BARRIER 1

#pragma GCC system_header

#if __cplusplus > 201703L
#include "critical_section.hh"
#include <utility>

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  #define __cpp_lib_barrier 201907L

  struct __empty_completion
  {
    void
    operator()() noexcept
    { }
  };

  template<typename _CompletionF = __empty_completion>
  class barrier
  {
  public:
    using arrival_token = ptrdiff_t;

    static constexpr ptrdiff_t
    max() noexcept
    { return __PTRDIFF_MAX__; }

    explicit
    barrier(ptrdiff_t __count, _CompletionF __completion = _CompletionF())
    : _M_phase(0), _M_expected(__count), _M_completion(std::move(__completion)), _M_b(__count), _M_wait(nullptr)
	{ assert(__count >= 0 && __count <= max()); }

    barrier(barrier const&) = delete;
    barrier& operator=(barrier const&) = delete;

    arrival_token
    arrive(ptrdiff_t __update = 1)
    {
      critical_section cs;
      assert(__update > 0 && __update <= _M_b);
      arrival_token result = _M_phase;
      _M_b -= __update;
      if (_M_b == 0)
      {
        _M_completion();
        ++_M_phase;
        _M_b = _M_expected;
        core_all_wakeup(_M_wait, E_SUCCESS);
      }
      return result;
    }

    void
    wait(arrival_token __phase)
    {
      critical_section cs;
      while (__phase == _M_phase)
        core_tsk_waitFor(&_M_wait, INFINITE);
    }

    void
    arrive_and_wait()
    { wait(arrive()); }

    void
    arrive_and_drop()
    {
      critical_section cs;
      --_M_expected;
      arrive(1);
    }

  private:
    arrival_token _M_phase;
    ptrdiff_t     _M_expected;
    _CompletionF  _M_completion;
    ptrdiff_t     _M_b;
    tsk_t        *_M_wait;
  };

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace
#endif // C++20
#endif // _GLIBCXX_BARRIER
