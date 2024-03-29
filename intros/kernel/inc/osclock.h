/******************************************************************************

    @file    IntrOS: osclock.h
    @author  Rajmund Szymanski
    @date    10.07.2022
    @brief   This file implements steady clock for IntrOS.

 ******************************************************************************

   Copyright (c) 2018-2022 Rajmund Szymanski. All rights reserved.

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

#ifndef __INTROS_CLK_H
#define __INTROS_CLK_H

#include "oskernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : sys_time
 *
 * Description       : return current value of system counter
 *
 * Parameters        : none
 *
 * Return            : current value of system counter
 *
 ******************************************************************************/

cnt_t sys_time( void );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus

#if __cplusplus >= 201402L
#include <chrono>
#endif

namespace intros {

/******************************************************************************
 *
 * Class             : Clock
 *
 * Description       : steady clock
 *
 ******************************************************************************/

struct Clock
{
	using rep        = cnt_t;
#if __cplusplus >= 201402L
	using period     = std::ratio<1, OS_FREQUENCY>;
	using duration   = std::chrono::duration<rep, period>;
	using time_point = std::chrono::time_point<Clock>;

	static constexpr bool is_steady = true;

	static
	time_point now()
	{
		return time_point{duration{::sys_time()}};
	}

	template<class _Rep, class _Period> static constexpr
	rep count( const std::chrono::duration<_Rep, _Period>& _delay )
	{
		return std::chrono::duration_cast<duration>(_delay).count();
	}

	template<class _Clock, class _Duration> static constexpr
	rep until( const std::chrono::time_point<_Clock, _Duration>& _time )
	{
		return std::chrono::duration_cast<duration>(_time.time_since_epoch()).count();
	}
#endif
	static constexpr
	rep count( const rep _delay ) { return _delay; }

	static constexpr
	rep until( const rep _time )  { return _time; }
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_CLK_H
