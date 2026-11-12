/******************************************************************************

    @file    IntrOS: oslist.h
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

#ifndef __INTROS_LST_H
#define __INTROS_LST_H

#include "oskernel.h"
#include "osclock.h"

/**
 * @defgroup list List
 * @brief    lists of memory objects
 * @{
 */

/**
 * @brief   queue
 */

typedef struct __que que_t;

struct __que
{
	que_t *  next; ///< next object in the queue
};

/******************************************************************************
 *
 * Name              : _QUE_INIT
 *
 * Description       : create and initialize a queue object
 *
 * Parameters        : none
 *
 * Return            : queue object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _QUE_INIT() { NULL }

/**
 * @brief   list
 */

typedef struct __lst lst_t;

struct __lst
{
	obj_t    obj;   ///< object header

	que_t    head;  ///< list head
};

typedef struct __lst lst_id [];

/******************************************************************************
 *
 * Name              : _LST_INIT
 *
 * Description       : create and initialize a list object
 *
 * Parameters        : none
 *
 * Return            : list object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _LST_INIT() { _OBJ_INIT(), _QUE_INIT() }

/**
 * @brief   define and initialize a list object
 *
 * @param   lst        name of a pointer to list object
 */

#define             OS_LST( lst ) \
                       lst_t lst[] = { _LST_INIT() }

/** static alias of OS_LST() */
#define         static_LST( lst ) \
                static lst_t lst[] = { _LST_INIT() }

/**
 * @brief   create and initialize a list object
 *
 * @return  list object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                LST_INIT() \
                      _LST_INIT()
#endif

/**
 * @brief   create and initialize a list object
 *
 * @return  list object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                LST_CREATE() \
                     { LST_INIT  () }
/** alias of LST_CREATE() */
#define                LST_NEW \
                       LST_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize a list object
 *
 * @param   lst        pointer to list object
 */

void lst_init( lst_t *lst );

/**
 * @brief   wake up all waiting tasks with 'E_STOPPED' event value
 *
 * @param   lst        pointer to list object
 *
 * @note    use only in thread mode
 */

void lst_reset( lst_t *lst );

/** alias of lst_reset() */
__STATIC_INLINE
void lst_kill( lst_t *lst ) { lst_reset(lst); }

/**
 * @brief   try to get memory object from the list object,
 *          don't wait if the list object is empty
 *
 * @param   lst        pointer to list object
 * @param   data       pointer to store the pointer to the memory object
 *
 * @retval  E_SUCCESS  pointer to memory object was successfully transferred to the data pointer
 * @retval  E_TIMEOUT  list object is empty, try again
 */

int lst_take( lst_t *lst, void **data );

/** alias of lst_take() */
__STATIC_INLINE
int lst_tryWait( lst_t *lst, void **data ) { return lst_take(lst, data); }

/**
 * @brief   try to get memory object from the list object,
 *          wait for given duration of time while the list object is empty
 *
 * @param   lst        pointer to list object
 * @param   data       pointer to store the pointer to the memory object
 * @param   delay      duration of time (maximum number of ticks to wait while the list object is empty)
 *                     - IMMEDIATE: don't wait if the list object is empty
 *                     - INFINITE:  wait indefinitely while the list object is empty
 *
 * @retval  E_SUCCESS  pointer to memory object was successfully transferred to the data pointer
 * @retval  E_STOPPED  list object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  list object is empty and was not received data before the specified timeout expired
 */

int lst_waitFor( lst_t *lst, void **data, cnt_t delay );

/**
 * @brief   try to get memory object from the list object,
 *          wait until given timepoint while the list object is empty
 *
 * @param   lst        pointer to list object
 * @param   data       pointer to store the pointer to the memory object
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  pointer to memory object was successfully transferred to the data pointer
 * @retval  E_STOPPED  list object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  list object is empty and was not received data before the specified timeout expired
 */

int lst_waitUntil( lst_t *lst, void **data, cnt_t time );

/**
 * @brief   try to get memory object from the list object,
 *          wait indefinitely while the list object is empty
 *
 * @param   lst        pointer to list object
 * @param   data       pointer to store the pointer to the memory object
 *
 * @retval  E_SUCCESS  pointer to memory object was successfully transferred to the data pointer
 * @retval  E_STOPPED  list object was reseted
 */

__STATIC_INLINE
int lst_wait( lst_t *lst, void **data ) { return lst_waitFor(lst, data, INFINITE); }

/**
 * @brief   transfer memory object to the list object,
 *
 * @param   lst        pointer to list object
 * @param   data       pointer to memory object
 *                     (taken from a memory pool: the list uses the header preceding the memory object)
 */

void lst_give( lst_t *lst, void *data );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus)
namespace intros {

/**
 * @brief   create and initialize a list object
 *
 * @par     Constructor parameters
 *          - C: class of a list object
 */

template<class C>
struct ListTT : public __lst
{
	/// constructor, see the constructor parameters of the class
	constexpr
	ListTT(): __lst _LST_INIT() {}

	~ListTT() { assert(__lst::obj.queue == nullptr); }

	/// move constructor
	ListTT( ListTT&& _src ): __lst(_src)
	{
		assert(_src.obj.queue == nullptr && _src.head.next == nullptr);
	}

	ListTT( const ListTT& ) = delete;
	ListTT& operator=( ListTT&& ) = delete;
	ListTT& operator=( const ListTT& ) = delete;

	void reset    ()                               {        lst_reset    (this); }                                                         ///< lst_reset()
	void kill     ()                               {        lst_kill     (this); }                                                         ///< lst_kill()
	int  take     ( C   **_data )                  { return lst_take     (this, reinterpret_cast<void **>(_data)); }                       ///< lst_take()
	int  tryWait  ( C   **_data )                  { return lst_tryWait  (this, reinterpret_cast<void **>(_data)); }                       ///< lst_tryWait()
	template<typename T>
	int  waitFor  ( C   **_data, const T& _delay ) { return lst_waitFor  (this, reinterpret_cast<void **>(_data), Clock::count(_delay)); } ///< lst_waitFor()
	template<typename T>
	int  waitUntil( C   **_data, const T& _time )  { return lst_waitUntil(this, reinterpret_cast<void **>(_data), Clock::until(_time)); }  ///< lst_waitUntil()
	int  wait     ( C   **_data )                  { return lst_wait     (this, reinterpret_cast<void **>(_data)); }                       ///< lst_wait()
	void give     ( void *_data )                  {        lst_give     (this,                           _data); }                        ///< lst_give()
};

/**
 * @brief   create and initialize a list object
 */

using List = ListTT<void>;

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

/** @} */

#endif//__INTROS_LST_H
