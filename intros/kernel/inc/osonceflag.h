/******************************************************************************

    @file    IntrOS: osonceflag.h
    @author  Rajmund Szymanski
    @date    17.11.2025
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

/**
 * @defgroup onceflag Once flag
 * @brief    once flags (one-time call of a function)
 * @{
 */

/**
 * @brief   once flag
 */

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

/**
 * @brief   define and initialize a once flag object
 *
 * @param   one        name of a pointer to once flag object
 */

#define             OS_ONE( one ) \
                       one_t one[] = { _ONE_INIT() }

/** static alias of OS_ONE() */
#define         static_ONE( one ) \
                static one_t one[] = { _ONE_INIT() }

/**
 * @brief   create and initialize a once flag object
 *
 * @return  once flag object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                ONE_INIT() \
                      _ONE_INIT()
#endif

/**
 * @brief   create and initialize a once flag object
 *
 * @return  once flag object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                ONE_CREATE() \
                     { ONE_INIT  () }
/** alias of ONE_CREATE() */
#define                ONE_NEW \
                       ONE_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize a once flag object
 *
 * @param   one        pointer to once flag object
 */

__STATIC_INLINE
void one_init( one_t *one ) { *one = 0; }

/**
 * @brief   try to execute given function
 *
 *          if the once flag has been previously set, the function is not executed
 *
 * @param   one        pointer to once flag object
 * @param   fun        pointer to once function
 *
 * @note    the once flag is set before the function is called, so another caller (another task
 *          or a timer callback, when the function gives up the processor, or an interrupt handler)
 *          does not wait for the completion of the function
 */

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

#if defined(__cplusplus)
namespace intros {

/**
 * @brief   use a once flag object
 *
 * @note    the once flag is set before the function is called, so another caller (another task
 *          or a timer callback, when the function gives up the processor, or an interrupt handler)
 *          does not wait for the completion of the function
 */

struct OnceFlag
{
	/// constructor, see the constructor parameters of the class
	constexpr
	OnceFlag(): flg_{_ONE_INIT()} {}

	/// move constructor
	OnceFlag( OnceFlag&& ) = default;
	OnceFlag( const OnceFlag& ) = delete;
	OnceFlag& operator=( OnceFlag&& ) = delete;
	OnceFlag& operator=( const OnceFlag& ) = delete;

#if __cplusplus >= 201402L
	/// call _fun(_args...) once, like std::call_once (one_call())
	template<typename Callable, typename... Args>
	void call( Callable&& _fun, Args&&... _args )
	{
		one_t flag;
	#if OS_ATOMICS
		flag = std::atomic_exchange((std::atomic_uint_fast8_t *)&flg_, _ONE_DONE());
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
	one_t flg_;
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

/** @} */

#endif//__INTROS_ONE_H
