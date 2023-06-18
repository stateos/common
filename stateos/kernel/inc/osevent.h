/******************************************************************************

    @file    StateOS: osevent.h
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

#ifndef __STATEOS_EVT_H
#define __STATEOS_EVT_H

#include "oskernel.h"
#include "osclock.h"

/******************************************************************************
 *
 * Name              : event
 *
 ******************************************************************************/

typedef struct __evt evt_t;

struct __evt
{
	obj_t    obj;   // object header
};

typedef struct __evt evt_id [];

/******************************************************************************
 *
 * Name              : _EVT_INIT
 *
 * Description       : create and initialize an event object
 *
 * Parameters        : none
 *
 * Return            : event object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _EVT_INIT() { _OBJ_INIT() }

/******************************************************************************
 *
 * Name              : OS_EVT
 * Static alias      : static_EVT
 *
 * Description       : define and initialize an event object
 *
 * Parameters
 *   evt             : name of a pointer to event object
 *
 ******************************************************************************/

#define             OS_EVT( evt ) \
                       evt_t evt[] = { _EVT_INIT() }

#define         static_EVT( evt ) \
                static evt_t evt[] = { _EVT_INIT() }

/******************************************************************************
 *
 * Name              : EVT_INIT
 *
 * Description       : create and initialize an event object
 *
 * Parameters        : none
 *
 * Return            : event object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                EVT_INIT() \
                      _EVT_INIT()
#endif

/******************************************************************************
 *
 * Name              : EVT_CREATE
 * Alias             : EVT_NEW
 *
 * Description       : create and initialize an event object
 *
 * Parameters        : none
 *
 * Return            : event object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                EVT_CREATE() \
                     { EVT_INIT  () }
#define                EVT_NEW \
                       EVT_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : evt_init
 *
 * Description       : initialize an event object
 *
 * Parameters
 *   evt             : pointer to event object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void evt_init( evt_t *evt );

/******************************************************************************
 *
 * Name              : evt_create
 * Alias             : evt_new
 *
 * Description       : create and initialize a new event object
 *
 * Parameters        : none
 *
 * Return            : pointer to event object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

evt_t *evt_create( void );

__STATIC_INLINE
evt_t *evt_new( void ) { return evt_create(); }

/******************************************************************************
 *
 * Name              : evt_reset
 * Alias             : evt_kill
 *
 * Description       : reset the event object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   evt             : pointer to event object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void evt_reset( evt_t *evt );

__STATIC_INLINE
void evt_kill( evt_t *evt ) { evt_reset(evt); }

/******************************************************************************
 *
 * Name              : evt_destroy
 * Alias             : evt_delete
 *
 * Description       : reset the event object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * Parameters
 *   evt             : pointer to event object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void evt_destroy( evt_t *evt );

__STATIC_INLINE
void evt_delete( evt_t *evt ) { evt_destroy(evt); }

/******************************************************************************
 *
 * Name              : evt_waitFor
 *
 * Description       : wait for release the event object for given duration of time
 *
 * Parameters
 *   evt             : pointer to event object
 *   event           : pointer to store event value
 *   delay           : duration of time (maximum number of ticks to wait for release the event object)
 *                     IMMEDIATE: don't wait until the event object has been released
 *                     INFINITE:  wait indefinitely until the event object has been released
 *
 * Return
 *   E_SUCCESS       : event value was successfully transferred from the event object
 *   E_STOPPED       : event object was reseted before the specified timeout expired
 *   E_DELETED       : event object was deleted before the specified timeout expired
 *   E_TIMEOUT       : event object was not released before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int evt_waitFor( evt_t *evt, unsigned *event, cnt_t delay );

/******************************************************************************
 *
 * Name              : evt_waitUntil
 *
 * Description       : wait for release the event object until given timepoint
 *
 * Parameters
 *   evt             : pointer to event object
 *   event           : pointer to store event value
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : event value was successfully transferred from the event object
 *   E_STOPPED       : event object was reseted before the specified timeout expired
 *   E_DELETED       : event object was deleted before the specified timeout expired
 *   E_TIMEOUT       : event object was not released before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int evt_waitUntil( evt_t *evt, unsigned *event, cnt_t time );

/******************************************************************************
 *
 * Name              : evt_wait
 *
 * Description       : wait indefinitely until the event object has been released
 *
 * Parameters
 *   evt             : pointer to event object
 *   event           : pointer to store event value
 *
 * Return
 *   E_SUCCESS       : event value was successfully transferred from the event object
 *   E_STOPPED       : event object was reseted
 *   E_DELETED       : event object was deleted
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
int evt_wait( evt_t *evt, unsigned *event ) { return evt_waitFor(evt, event, INFINITE); }

/******************************************************************************
 *
 * Name              : evt_give
 * ISR alias         : evt_giveISR
 *
 * Description       : resume all tasks that are waiting on the event object
 *
 * Parameters
 *   evt             : pointer to event object
 *   event           : event value
 *
 * Return            : none
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

void evt_give( evt_t *evt, unsigned event );

__STATIC_INLINE
void evt_giveISR( evt_t *evt, unsigned event ) { evt_give(evt, event); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace stateos {

/******************************************************************************
 *
 * Class             : Event
 *
 * Description       : create and initialize an event object
 *
 * Constructor parameters
 *                   : none
 *
 ******************************************************************************/

struct Event : public __evt
{
	constexpr
	Event(): __evt _EVT_INIT() {}

	~Event() { assert(__evt::obj.queue == nullptr); }

	Event( Event&& ) = default;
	Event( const Event& ) = delete;
	Event& operator=( Event&& ) = delete;
	Event& operator=( const Event& ) = delete;

	void reset    ()                                    {        evt_reset    (this); }
	void kill     ()                                    {        evt_kill     (this); }
	void destroy  ()                                    {        evt_destroy  (this); }
	template<typename T>
	int  waitFor  ( unsigned *_event, const T& _delay ) { return evt_waitFor  (this, _event, Clock::count(_delay)); }
	template<typename T>
	int  waitUntil( unsigned *_event, const T& _time )  { return evt_waitUntil(this, _event, Clock::until(_time)); }
	int  wait     ( unsigned *_event )                  { return evt_wait     (this, _event); }
	void give     ( unsigned  _event )                  {        evt_give     (this, _event); }
	void giveISR  ( unsigned  _event )                  {        evt_giveISR  (this, _event); }

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<Event>;
#else
	using Ptr = Event *;
#endif

/******************************************************************************
 *
 * Name              : Event::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters        : none
 *
 * Return            : std::unique_pointer / pointer to Event object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create()
	{
		auto evt = new (std::nothrow) Event();
		if (evt != nullptr)
			evt->__evt::obj.res = evt;
		return Ptr(evt);
	}

};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__STATEOS_EVT_H
