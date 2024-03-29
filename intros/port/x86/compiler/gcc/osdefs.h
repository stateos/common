/******************************************************************************

    @file    IntrOS: osdefs.h
    @author  Rajmund Szymanski
    @date    18.05.2021
    @brief   IntrOS port file for X86.

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

#ifndef __INTROSDEFS_H
#define __INTROSDEFS_H

/* -------------------------------------------------------------------------- */

#ifndef __ASM
#define __ASM               __asm__
#endif
#ifndef __CONSTRUCTOR
#define __CONSTRUCTOR       __attribute__((constructor))
#endif
#ifndef __NO_RETURN
#define __NO_RETURN         __attribute__((noreturn))
#endif
#ifndef __STATIC_INLINE
#define __STATIC_INLINE       static inline
#endif
#ifndef __STATIC_FORCEINLINE
#define __STATIC_FORCEINLINE  static inline \
                            __attribute__((always_inline))
#endif
#ifndef __COMPILER_BARRIER
#define __COMPILER_BARRIER() \
                            __asm__ volatile("":::"memory")
#endif
#ifndef __PACKED
#define __PACKED            __attribute__((packed, aligned(1)))
#endif
#ifndef __PACKED_STRUCT
#define __PACKED_STRUCT       struct __attribute__((packed, aligned(1)))
#endif

/* -------------------------------------------------------------------------- */

#endif//__INTROSDEFS_H
