/******************************************************************************

    @file    DemOS: osversion.h
    @author  Rajmund Szymański
    @date    18.03.2023
    @brief   This file provides set of functions for DemOS.

 ******************************************************************************

   Copyright (c) 2018-2023 Rajmund Szymanski. All rights reserved.

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

#ifndef __DEMOSVERSION_H
#define __DEMOSVERSION_H

#define __DEMOS_MAJOR        1
#define __DEMOS_MINOR        1
#define __DEMOS_BUILD        0

#define __DEMOS        ((((__DEMOS_MAJOR)&0xFFUL)<<24)|(((__DEMOS_MINOR)&0xFFUL)<<16)|((__DEMOS_BUILD)&0xFFFFUL))

#define __DEMOS__           "DemOS v" STRINGIZE(__DEMOS_MAJOR) "." STRINGIZE(__DEMOS_MINOR) "." STRINGIZE(__DEMOS_BUILD)

#define STRINGIZE(n) STRINGIZE_HELPER(n)
#define STRINGIZE_HELPER(n) #n

#endif//__DEMOSVERSION_H
