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

/******************************************************************************
 *
 * Name              : queue
 *
 ******************************************************************************/

typedef struct __que que_t;

struct __que
{
	que_t *  next; // next object in the queue
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

/******************************************************************************
 *
 * Name              : list
 *
 ******************************************************************************/

typedef struct __lst lst_t;

struct __lst
{
	obj_t    obj;   // object header

	que_t    head;  // list head
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

/******************************************************************************
 *
 * Name              : OS_LST
 * Static alias      : static_LST
 *
 * Description       : define and initialize a list object
 *
 * Parameters
 *   lst             : name of a pointer to list object
 *
 ******************************************************************************/

#define             OS_LST( lst ) \
                       lst_t lst[] = { _LST_INIT() }

#define         static_LST( lst ) \
                static lst_t lst[] = { _LST_INIT() }

/******************************************************************************
 *
 * Name              : LST_INIT
 *
 * Description       : create and initialize a list object
 *
 * Parameters        : none
 *
 * Return            : list object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                LST_INIT() \
                      _LST_INIT()
#endif

/******************************************************************************
 *
 * Name              : LST_CREATE
 * Alias             : LST_NEW
 *
 * Description       : create and initialize a list object
 *
 * Parameters        : none
 *
 * Return            : list object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                LST_CREATE() \
                     { LST_INIT  () }
#define                LST_NEW \
                       LST_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : lst_init
 *
 * Description       : initialize a list object
 *
 * Parameters
 *   lst             : pointer to list object
 *
 * Return            : none
 *
 ******************************************************************************/

void lst_init( lst_t *lst );

/******************************************************************************
 *
 * Name              : lst_reset
 * Alias             : lst_kill
 *
 * Description       : wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   lst             : pointer to list object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void lst_reset( lst_t *lst );

__STATIC_INLINE
void lst_kill( lst_t *lst ) { lst_reset(lst); }

/******************************************************************************
 *
 * Name              : lst_take
 * Alias             : lst_tryWait
 *
 * Description       : try to get memory object from the list object,
 *                     don't wait if the list object is empty
 *
 * Parameters
 *   lst             : pointer to list object
 *   data            : pointer to store the pointer to the memory object
 *
 * Return
 *   E_SUCCESS       : pointer to memory object was successfully transferred to the data pointer
 *   E_TIMEOUT       : list object is empty, try again
 *
 ******************************************************************************/

int lst_take( lst_t *lst, void **data );

__STATIC_INLINE
int lst_tryWait( lst_t *lst, void **data ) { return lst_take(lst, data); }

/******************************************************************************
 *
 * Name              : lst_waitFor
 *
 * Description       : try to get memory object from the list object,
 *                     wait for given duration of time while the list object is empty
 *
 * Parameters
 *   lst             : pointer to list object
 *   data            : pointer to store the pointer to the memory object
 *   delay           : duration of time (maximum number of ticks to wait while the list object is empty)
 *                     IMMEDIATE: don't wait if the list object is empty
 *                     INFINITE:  wait indefinitely while the list object is empty
 *
 * Return
 *   E_SUCCESS       : pointer to memory object was successfully transferred to the data pointer
 *   E_STOPPED       : list object was reseted before the specified timeout expired
 *   E_TIMEOUT       : list object is empty and was not received data before the specified timeout expired
 *
 ******************************************************************************/

int lst_waitFor( lst_t *lst, void **data, cnt_t delay );

/******************************************************************************
 *
 * Name              : lst_waitUntil
 *
 * Description       : try to get memory object from the list object,
 *                     wait until given timepoint while the list object is empty
 *
 * Parameters
 *   lst             : pointer to list object
 *   data            : pointer to store the pointer to the memory object
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : pointer to memory object was successfully transferred to the data pointer
 *   E_STOPPED       : list object was reseted before the specified timeout expired
 *   E_TIMEOUT       : list object is empty and was not received data before the specified timeout expired
 *
 ******************************************************************************/

int lst_waitUntil( lst_t *lst, void **data, cnt_t time );

/******************************************************************************
 *
 * Name              : lst_wait
 *
 * Description       : try to get memory object from the list object,
 *                     wait indefinitely while the list object is empty
 *
 * Parameters
 *   lst             : pointer to list object
 *   data            : pointer to store the pointer to the memory object
 *
 * Return
 *   E_SUCCESS       : pointer to memory object was successfully transferred to the data pointer
 *   E_STOPPED       : list object was reseted
 *
 ******************************************************************************/

__STATIC_INLINE
int lst_wait( lst_t *lst, void **data ) { return lst_waitFor(lst, data, INFINITE); }

/******************************************************************************
 *
 * Name              : lst_give
 *
 * Description       : transfer memory object to the list object,
 *
 * Parameters
 *   lst             : pointer to list object
 *   data            : pointer to memory object
 *
 * Return            : none
 *
 ******************************************************************************/

void lst_give( lst_t *lst, void *data );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus)
namespace intros {

/******************************************************************************
 *
 * Class             : ListTT<>
 *
 * Description       : create and initialize a list object
 *
 * Constructor parameters
 *   C               : class of a list object
 *
 ******************************************************************************/

template<class C>
struct ListTT : public __lst
{
	constexpr
	ListTT(): __lst _LST_INIT() {}

	~ListTT() { assert(__lst::obj.queue == nullptr); }

	ListTT( ListTT&& ) = default;
	ListTT( const ListTT& ) = delete;
	ListTT& operator=( ListTT&& ) = delete;
	ListTT& operator=( const ListTT& ) = delete;

	void reset    ()                               {        lst_reset    (this); }
	void kill     ()                               {        lst_kill     (this); }
	int  take     ( C   **_data )                  { return lst_take     (this, reinterpret_cast<void **>(_data)); }
	int  tryWait  ( C   **_data )                  { return lst_tryWait  (this, reinterpret_cast<void **>(_data)); }
	template<typename T>
	int  waitFor  ( C   **_data, const T& _delay ) { return lst_waitFor  (this, reinterpret_cast<void **>(_data), Clock::count(_delay)); }
	template<typename T>
	int  waitUntil( C   **_data, const T& _time )  { return lst_waitUntil(this, reinterpret_cast<void **>(_data), Clock::until(_time)); }
	int  wait     ( C   **_data )                  { return lst_wait     (this, reinterpret_cast<void **>(_data)); }
	void give     ( void *_data )                  {        lst_give     (this,                           _data); }
};

/******************************************************************************
 *
 * Class             : List
 *
 * Description       : create and initialize a list object
 *
 ******************************************************************************/

using List = ListTT<void>;

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_LST_H
