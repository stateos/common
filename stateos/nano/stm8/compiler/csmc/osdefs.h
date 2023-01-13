/******************************************************************************

    @file    StateOS: osdefs.h
    @author  Rajmund Szymanski
    @date    11.07.2022
    @brief   StateOS port file for STM8 uC.

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

#ifndef __STATEOSDEFS_H
#define __STATEOSDEFS_H

/* -------------------------------------------------------------------------- */

#ifndef __NO_RETURN
#define __NO_RETURN
#endif
#ifndef __STATIC_INLINE
#define __STATIC_INLINE       static inline
#endif
#ifndef __WFI
#define __WFI                 wfi
#endif
#ifndef __ISB
#define __ISB                 nop
#endif
#ifndef __COMPILER_BARRIER
#define __COMPILER_BARRIER()
#endif
#ifndef __PACKED
#define __PACKED
#endif
#ifndef __PACKED_STRUCT
#define __PACKED_STRUCT       struct
#endif

/* -------------------------------------------------------------------------- */

#ifndef   static_assert
#define   static_assert(C, M) assert(C)
#endif

/* -------------------------------------------------------------------------- */

#endif//__STATEOSDEFS_H
