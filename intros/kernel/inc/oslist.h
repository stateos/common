/******************************************************************************

    @file    IntrOS: oslist.h
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

#ifndef __INTROS_LST_H
#define __INTROS_LST_H

#include "oskernel.h"

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

#define               _LST_INIT() { _QUE_INIT() }

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
 * Name              : lst_take
 * Alias             : lst_tryWait
 *
 * Description       : try to get memory object from the list object,
 *                     don't wait if the list object is empty
 *
 * Parameters
 *   lst             : pointer to list object
 *
 * Return            : pointer to the memory object
 *   NULL            : list object is empty
 *
 ******************************************************************************/

void *lst_take( lst_t *lst );

__STATIC_INLINE
void *lst_tryWait( lst_t *lst ) { return lst_take(lst); }

/******************************************************************************
 *
 * Name              : lst_wait
 *
 * Description       : try to get memory object from the list object,
 *                     wait indefinitely while the list object is empty
 *
 * Parameters
 *   lst             : pointer to list object
 *
 * Return            : pointer to the memory object
 *
 ******************************************************************************/

void *lst_wait( lst_t *lst );

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

#ifdef __cplusplus
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

	ListTT( ListTT&& ) = default;
	ListTT( const ListTT& ) = delete;
	ListTT& operator=( ListTT&& ) = delete;
	ListTT& operator=( const ListTT& ) = delete;

	C  * take   ()           { return reinterpret_cast<C *>(lst_take   (this)); }
	C  * tryWait()           { return reinterpret_cast<C *>(lst_tryWait(this)); }
	C  * wait   ()           { return reinterpret_cast<C *>(lst_wait   (this)); }
	void give   ( C *_data ) {                              lst_give   (this, _data); }
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
