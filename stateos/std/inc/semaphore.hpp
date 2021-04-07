/******************************************************************************

    @file    StateOS: std_semaphore.hpp
    @author  Rajmund Szymanski
    @date    07.04.2021
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

#ifndef __STATEOSSTD_SEMAPHORE_HPP
#define __STATEOSSTD_SEMAPHORE_HPP

#include "inc/ossemaphore.h"
#include "inc/chrono.hpp"

namespace std {

/******************************************************************************
 *
 * Class             : std::counting_semaphore
 *                   : std::binary_semaphore
 *                   : std::semaphore
 *
 ******************************************************************************/

template<ptrdiff_t least_max_value = numeric_limits<ptrdiff_t>::max()>
struct counting_semaphore : public __sem
{
	constexpr explicit
	counting_semaphore( const ptrdiff_t desired = 0 ) :
	__sem _SEM_INIT(static_cast<unsigned>(desired), static_cast<unsigned>(least_max_value))
	{ static_assert(least_max_value >= 0 && least_max_value <= semCounting); }

	~counting_semaphore() = default;

	counting_semaphore( counting_semaphore&& ) = default;
	counting_semaphore( const counting_semaphore& ) = delete;
	counting_semaphore& operator=( counting_semaphore&& ) = delete;
	counting_semaphore& operator=( const counting_semaphore& ) = delete;

	void release()
	{
		sem_post(this);
	}
	
	void release( ptrdiff_t _num )
	{
		sem_giveNum(this, static_cast<unsigned>(_num));
	}
	
	void acquire()
	{
		sem_wait(this);
	}
	
	bool try_acquire() noexcept
	{
		return sem_tryWait(this) == E_SUCCESS;
	}

	template<class Rep, class Period>
	bool try_acquire_for( const chrono::duration<Rep, Period>& _delay )
	{
		return sem_waitFor(this, chrono::systick::count(_delay)) == E_SUCCESS;
	}
	
	template<class Clock, class Duration>
	bool try_acquire_until( const chrono::time_point<Clock, Duration>& _time )
	{
		return sem_waitUntil(this, chrono::systick::until(_time)) == E_SUCCESS;
	}

	static constexpr ptrdiff_t max() noexcept { return least_max_value; }
};

using direct_semaphore = counting_semaphore<0>;
using binary_semaphore = counting_semaphore<1>;
using        semaphore = counting_semaphore<>;

}     //  namespace std
#endif//__STATEOSSTD_HPP
