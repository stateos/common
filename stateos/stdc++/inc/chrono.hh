/******************************************************************************

    @file    StateOS: chrono.hh
    @author  Rajmund Szymanski
    @date    11.04.2021
    @brief   This file contains definitions for StateOS.

 ******************************************************************************

   Copyright (c) 2018-2021 Rajmund Szymanski. All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.

 ******************************************************************************/

#ifndef __STATEOS_CHRONO_HH
#define __STATEOS_CHRONO_HH

#include "inc/osclock.h"
#include <chrono>

namespace std {
namespace chrono {

/******************************************************************************
 *
 * Class             : std::chrono::systick
 *
 ******************************************************************************/

struct systick
{
	using rep        = cnt_t;
	using period     = ratio<1, OS_FREQUENCY>;
	using duration   = chrono::duration<rep, period>;
	using time_point = chrono::time_point<systick>;

	static constexpr bool is_steady = true;

	static
	time_point now() noexcept
	{
		return time_point(duration(::sys_time()));
	}

	template<class Rep, class Period> static constexpr
	rep count( const chrono::duration<Rep, Period>& _delay )
	{
		return duration_cast<duration>(_delay).count();
	}

	template<class Clock, class Duration> static constexpr
	rep until( const chrono::time_point<Clock, Duration>& _time )
	{
		return duration_cast<duration>(_time.time_since_epoch()).count();
	}

	static constexpr
	rep count( const rep _delay ) { return _delay; }

	static constexpr
	rep until( const rep _time )  { return _time; }
};

}     //  namespace chrono
}     //  namespace std

//-----------------------------------------------------------------------------

struct ostime_t : public ::timespec
{
	cnt_t to_ticks() const
	{
		auto sec  = std::chrono::seconds(tv_sec);
		auto nsec = std::chrono::nanoseconds(tv_nsec);
		return std::chrono::systick::count(sec) + std::chrono::systick::count(nsec);
	}
};

#endif//__STATEOS_CHRONO_HH
