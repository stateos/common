/******************************************************************************

    @file    StateOS: osonceflag.h
    @author  Rajmund Szymanski
    @date    17.11.2025
    @brief   This file contains definitions for StateOS.

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

#ifndef __STATEOS_ONE_H
#define __STATEOS_ONE_H

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
 * Name              : _ONE_BUSY
 *
 * Description       : once flag state during execution of the once function
 *
 * Parameters        : none
 *
 * Return            : once flag object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _ONE_BUSY()   1

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

#define               _ONE_DONE()   2

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
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
void one_init( one_t *one ) { *one = 0; }

/******************************************************************************
 *
 * Name              : one_enter
 *
 * Description       : start the execution of the once function
 *                     if the once function is being executed by another task, wait for its completion
 *
 * Parameters
 *   one             : pointer to once flag object
 *
 * Return
 *   true            : the caller must execute the once function and then call one_leave
 *   false           : the once function has already been executed
 *                     (or is being executed and the caller is an interrupt handler)
 *
 * Note              : for internal use
 *
 ******************************************************************************/

bool one_enter( one_t *one );

/******************************************************************************
 *
 * Name              : one_leave
 *
 * Description       : end the execution of the once function and wake up waiting tasks
 *
 * Parameters
 *   one             : pointer to once flag object
 *   state           : new state of the once flag object
 *                     _ONE_DONE(): the once function has been executed
 *                     _ONE_INIT(): the once function has failed (exception) and can be executed again
 *
 * Return            : none
 *
 * Note              : for internal use
 *
 ******************************************************************************/

void one_leave( one_t *one, one_t state );

/******************************************************************************
 *
 * Name              : one_call
 *
 * Description       : try to execute given function
 *                     if the once flag has been previously set, the function is not executed
 *                     if the function is being executed by another task, wait for its completion
 *
 * Parameters
 *   one             : pointer to once flag object
 *   fun             : pointer to once function
 *
 * Return            : none
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     in handler mode the function does not wait for completion of the once function
 *                     calling one_call with the same once flag from the once function causes a deadlock
 *
 ******************************************************************************/

__STATIC_INLINE
void one_call( one_t *one, fun_t *fun )
{
	assert(one);
	assert(fun);

	if (one_enter(one))
	{
		fun();
		one_leave(one, _ONE_DONE());
	}
}

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(__STATEOS_STDCXX)
namespace stateos {

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

	OnceFlag( OnceFlag&& _src ): flg_{_src.flg_}
	{
		assert(_src.flg_ != _ONE_BUSY()); // the once function is not being executed
	}

	OnceFlag( const OnceFlag& ) = delete;
	OnceFlag& operator=( OnceFlag&& ) = delete;
	OnceFlag& operator=( const OnceFlag& ) = delete;

#if __cplusplus >= 201402L
	template<typename Callable, typename... Args>
	void call( Callable&& _fun, Args&&... _args )
	{
		if (one_enter(&flg_))
		{
	#if __cpp_exceptions
			try
			{
				_fun(std::forward<Args>(_args)...);
			}
			catch (...)
			{
				one_leave(&flg_, _ONE_INIT()); // the once function can be executed again
				throw;
			}
	#else
			_fun(std::forward<Args>(_args)...);
	#endif
			one_leave(&flg_, _ONE_DONE());
		}
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

#endif//__STATEOS_ONE_H
