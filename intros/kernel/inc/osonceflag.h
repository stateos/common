/******************************************************************************

    @file    IntrOS: osonceflag.h
    @author  Rajmund Szymanski
    @date    26.07.2022
    @brief   This file contains definitions for IntrOS.

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

#ifndef __INTROS_ONE_H
#define __INTROS_ONE_H

#include "oskernel.h"
#include "oscriticalsection.h"

/******************************************************************************
 *
 * Name              : once flag
 *
 ******************************************************************************/

typedef uint_fast8_t one_t;
typedef uint_fast8_t one_id [];

/******************************************************************************
 *
 * Name              : _ONE_INIT
 *
 * Description       : once flag state after initialization
 *
 * Parameters        : none
 *
 * Return            : once flag object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _ONE_INIT()   0

/******************************************************************************
 *
 * Name              : _ONE_DONE
 *
 * Description       : once flag state after use
 *
 * Parameters        : none
 *
 * Return            : once flag object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _ONE_DONE()   1

/******************************************************************************
 *
 * Name              : OS_ONE
 * Static alias      : static_ONE
 *
 * Description       : define and initialize a once flag object
 *
 * Parameters
 *   one             : name of a pointer to once flag object
 *
 ******************************************************************************/

#define             OS_ONE( one ) \
                       one_t one[] = { _ONE_INIT() }

#define         static_ONE( one ) \
                static one_t one[] = { _ONE_INIT() }

/******************************************************************************
 *
 * Name              : ONE_INIT
 *
 * Description       : create and initialize a once flag object
 *
 * Parameters        : none
 *
 * Return            : once flag object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                ONE_INIT() \
                      _ONE_INIT()
#endif

/******************************************************************************
 *
 * Name              : ONE_CREATE
 * Alias             : ONE_NEW
 *
 * Description       : create and initialize a once flag object
 *
 * Parameters        : none
 *
 * Return            : once flag object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                ONE_CREATE() \
                     { ONE_INIT  () }
#define                ONE_NEW \
                       ONE_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : one_init
 *
 * Description       : initialize a once flag object
 *
 * Parameters
 *   one             : pointer to once flag object
 *
 * Return            : none
 *
 ******************************************************************************/

__STATIC_INLINE
void one_init( one_t *one ) { *one = 0; }

/******************************************************************************
 *
 * Name              : one_call
 *
 * Description       : try to execute given function
 *                     if the once flag has been previously set, the function is not executed
 *
 * Parameters
 *   one             : pointer to once flag object
 *   fun             : pointer to once function
 *
 * Return            : none
 *
 ******************************************************************************/

__STATIC_INLINE
void one_call( one_t *one, fun_t *fun )
{
	one_t flag;
	assert(one);
	assert(fun);
#if OS_ATOMICS
	flag = __STD atomic_exchange((__STD atomic_uint_fast8_t *)one, _ONE_DONE());
#else
	sys_lock(); flag = *one; *one = _ONE_DONE(); sys_unlock();
#endif
	if (flag == _ONE_INIT()) fun();
}

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
namespace intros {

/******************************************************************************
 *
 * Class             : OnceFlag
 *
 * Description       : use a once flag object
 *
 * Constructor parameters
 *                   : none
 *
 ******************************************************************************/

struct OnceFlag
{
	constexpr
	OnceFlag(): flg_{_ONE_INIT()} {}

	OnceFlag( OnceFlag&& ) = default;
	OnceFlag( const OnceFlag& ) = delete;
	OnceFlag& operator=( OnceFlag&& ) = delete;
	OnceFlag& operator=( const OnceFlag& ) = delete;

#if __cplusplus >= 201402L
	template<typename Callable, typename... Args>
	void call( Callable&& _fun, Args&&... _args )
	{
		one_t flag;
	#if OS_ATOMICS
		flag = std::atomic_exchange(&flg_, _ONE_DONE());
	#else
		{ CriticalSection cri; flag = flg_; flg_ = _ONE_DONE(); }
	#endif
		if (flag == _ONE_INIT()) _fun(std::forward<Args>(_args)...);
	}
#else
	void call( fun_t *_fun )
	{
		one_call(&flg_, _fun);
	}
#endif

	private:
#if OS_ATOMICS && __cplusplus >= 201402L
	std::atomic<one_t> flg_;
#else
	one_t flg_;
#endif
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_ONE_H
