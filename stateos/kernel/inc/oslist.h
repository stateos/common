/******************************************************************************

    @file    StateOS: oslist.h
    @author  Rajmund Szymanski
    @date    18.06.2023
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

#ifndef __STATEOS_LST_H
#define __STATEOS_LST_H

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
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void lst_init( lst_t *lst );

/******************************************************************************
 *
 * Name              : lst_create
 * Alias             : lst_new
 *
 * Description       : create and initialize a new list object
 *
 * Parameters        : none
 *
 * Return            : pointer to list object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

lst_t *lst_create( void );

__STATIC_INLINE
lst_t *lst_new( void ) { return lst_create(); }

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
 * Name              : lst_destroy
 * Alias             : lst_delete
 *
 * Description       : reset the list object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * Parameters
 *   lst             : pointer to list object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void lst_destroy( lst_t *lst );

__STATIC_INLINE
void lst_delete( lst_t *lst ) { lst_destroy(lst); }

/******************************************************************************
 *
 * Name              : lst_take
 * Alias             : lst_tryWait
 * ISR alias         : lst_takeISR
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
 * Note              : may be used both in thread and handler mode
 *
 ******************************************************************************/

int lst_take( lst_t *lst, void **data );

__STATIC_INLINE
int lst_tryWait( lst_t *lst, void **data ) { return lst_take(lst, data); }

__STATIC_INLINE
int lst_takeISR( lst_t *lst, void **data ) { return lst_take(lst, data); }

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
 *   E_DELETED       : list object was deleted before the specified timeout expired
 *   E_TIMEOUT       : list object is empty and was not received data before the specified timeout expired
 *
 * Note              : use only in thread mode
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
 *   E_DELETED       : list object was deleted before the specified timeout expired
 *   E_TIMEOUT       : list object is empty and was not received data before the specified timeout expired
 *
 * Note              : use only in thread mode
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
 *   E_DELETED       : list object was deleted
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
int lst_wait( lst_t *lst, void **data ) { return lst_waitFor(lst, data, INFINITE); }

/******************************************************************************
 *
 * Name              : lst_give
 * ISR alias         : lst_giveISR
 *
 * Description       : transfer memory object to the list object,
 *
 * Parameters
 *   lst             : pointer to list object
 *   data            : pointer to memory object
 *
 * Return            : none
 *
 * Note              : may be used both in thread and handler mode
 *
 ******************************************************************************/

void lst_give( lst_t *lst, void *data );

__STATIC_INLINE
void lst_giveISR( lst_t *lst, void *data ) { lst_give(lst, data); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace stateos {

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
	void destroy  ()                               {        lst_destroy  (this); }
	int  take     ( C   **_data )                  { return lst_take     (this, reinterpret_cast<void **>(_data)); }
	int  tryWait  ( C   **_data )                  { return lst_tryWait  (this, reinterpret_cast<void **>(_data)); }
	int  takeISR  ( C   **_data )                  { return lst_takeISR  (this, reinterpret_cast<void **>(_data)); }
	template<typename T>
	int  waitFor  ( C   **_data, const T& _delay ) { return lst_waitFor  (this, reinterpret_cast<void **>(_data), Clock::count(_delay)); }
	template<typename T>
	int  waitUntil( C   **_data, const T& _time )  { return lst_waitUntil(this, reinterpret_cast<void **>(_data), Clock::until(_time)); }
	int  wait     ( C   **_data )                  { return lst_wait     (this, reinterpret_cast<void **>(_data)); }
	void give     ( void *_data )                  {        lst_give     (this,                           _data); }
	void giveISR  ( void *_data )                  {        lst_giveISR  (this,                           _data); }

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<ListTT<C>>;
#else
	using Ptr = ListTT<C> *;
#endif

/******************************************************************************
 *
 * Name              : ListTT<>::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters
 *   C               : class of a list object
 *
 * Return            : std::unique_pointer / pointer to ListTT<> object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create()
	{
		auto lst = new (std::nothrow) ListTT<C>();
		if (lst != nullptr)
			lst->__lst::obj.res = lst;
		return Ptr(lst);
	}

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

#endif//__STATEOS_LST_H
