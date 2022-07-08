/******************************************************************************

    @file    IntrOS: osversion.h
    @author  Rajmund Szymanski
    @date    08.07.2022
    @brief   This file contains definitions for IntrOS.

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

#ifndef __INTROSVERSION_H
#define __INTROSVERSION_H

#define __INTROS_MAJOR        4
#define __INTROS_MINOR        9
#define __INTROS_BUILD        0

#define __INTROS        ((((__INTROS_MAJOR)&0xFFUL)<<24)|(((__INTROS_MINOR)&0xFFUL)<<16)|((__INTROS_BUILD)&0xFFFFUL))

#define __INTROS__           "IntrOS v" STRINGIZE(__INTROS_MAJOR) "." STRINGIZE(__INTROS_MINOR) "." STRINGIZE(__INTROS_BUILD)

#define STRINGIZE(n) STRINGIZE_HELPER(n)
#define STRINGIZE_HELPER(n) #n

#endif//__INTROSVERSION_H
