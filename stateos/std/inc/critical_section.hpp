/******************************************************************************

    @file    StateOS: critical_section.hpp
    @author  Rajmund Szymanski
    @date    02.04.2021
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

#ifndef __STATEOSSTD_CRITICAL_SECTION_HPP
#define __STATEOSSTD_CRITICAL_SECTION_HPP

#include "inc/oscriticalsection.h"

namespace std {

/******************************************************************************
 *
 * Class             : std::critical_section
 *
 ******************************************************************************/

struct critical_section
{
	critical_section() { lck_ = core_set_lock(); }

	~critical_section() { core_put_lock(lck_); }

	critical_section( critical_section&& ) = delete;
	critical_section( const critical_section& ) = delete;
	critical_section& operator=( critical_section&& ) = delete;
	critical_section& operator=( const critical_section& ) = delete;

	private:
	lck_t lck_;
};

}     //  namespace std
#endif//__STATEOSSTD_CRITICAL_SECTION_HPP
