/******************************************************************************

    @file    StateOS: osevent.h
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

#ifndef __STATEOS_EVT_H
#define __STATEOS_EVT_H

#include "oskernel.h"
#include "osclock.h"

/**
 * @defgroup event Event
 * @brief    events (transfer of a value to all waiting tasks)
 * @{
 */

/**
 * @brief   event
 */

typedef struct __evt evt_t;

struct __evt
{
	obj_t    obj;   ///< object header
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

/**
 * @brief   define and initialize an event object
 *
 * @param   evt        name of a pointer to event object
 */

#define             OS_EVT( evt ) \
                       evt_t evt[] = { _EVT_INIT() }

/** static alias of OS_EVT() */
#define         static_EVT( evt ) \
                static evt_t evt[] = { _EVT_INIT() }

/**
 * @brief   create and initialize an event object
 *
 * @return  event object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                EVT_INIT() \
                      _EVT_INIT()
#endif

/**
 * @brief   create and initialize an event object
 *
 * @return  event object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                EVT_CREATE() \
                     { EVT_INIT  () }
/** alias of EVT_CREATE() */
#define                EVT_NEW \
                       EVT_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize an event object
 *
 * @param   evt        pointer to event object
 *
 * @note    use only in thread mode
 */

void evt_init( evt_t *evt );

/**
 * @brief   create and initialize a new event object
 *
 * @return  pointer to event object
 * @retval  NULL       object not created (not enough free memory)
 *
 * @note    use only in thread mode
 */

evt_t *evt_create( void );

/** alias of evt_create() */
__STATIC_INLINE
evt_t *evt_new( void ) { return evt_create(); }

/**
 * @brief   reset the event object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * @param   evt        pointer to event object
 *
 * @note    use only in thread mode
 */

void evt_reset( evt_t *evt );

/** alias of evt_reset() */
__STATIC_INLINE
void evt_kill( evt_t *evt ) { evt_reset(evt); }

/**
 * @brief   reset the event object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * @param   evt        pointer to event object
 *
 * @note    use only in thread mode
 */

void evt_destroy( evt_t *evt );

/** alias of evt_destroy() */
__STATIC_INLINE
void evt_delete( evt_t *evt ) { evt_destroy(evt); }

/**
 * @brief   wait for release the event object for given duration of time
 *
 * @param   evt        pointer to event object
 * @param   event      pointer to store event value
 * @param   delay      duration of time (maximum number of ticks to wait for release the event object)
 *                     - IMMEDIATE: don't wait until the event object has been released
 *                     - INFINITE:  wait indefinitely until the event object has been released
 *
 * @retval  E_SUCCESS  event value was successfully transferred from the event object
 * @retval  E_STOPPED  event object was reseted before the specified timeout expired
 * @retval  E_DELETED  event object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  event object was not released before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int evt_waitFor( evt_t *evt, unsigned *event, cnt_t delay );

/**
 * @brief   wait for release the event object until given timepoint
 *
 * @param   evt        pointer to event object
 * @param   event      pointer to store event value
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  event value was successfully transferred from the event object
 * @retval  E_STOPPED  event object was reseted before the specified timeout expired
 * @retval  E_DELETED  event object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  event object was not released before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int evt_waitUntil( evt_t *evt, unsigned *event, cnt_t time );

/**
 * @brief   wait indefinitely until the event object has been released
 *
 * @param   evt        pointer to event object
 * @param   event      pointer to store event value
 *
 * @retval  E_SUCCESS  event value was successfully transferred from the event object
 * @retval  E_STOPPED  event object was reseted
 * @retval  E_DELETED  event object was deleted
 *
 * @note    use only in thread mode
 */

__STATIC_INLINE
int evt_wait( evt_t *evt, unsigned *event ) { return evt_waitFor(evt, event, INFINITE); }

/**
 * @brief   resume all tasks that are waiting on the event object
 *
 * @param   evt        pointer to event object
 * @param   event      event value
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

void evt_give( evt_t *evt, unsigned event );

/** ISR alias of evt_give() */
__STATIC_INLINE
void evt_giveISR( evt_t *evt, unsigned event ) { evt_give(evt, event); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(__STATEOS_STDCXX)
namespace stateos {

/**
 * @brief   create and initialize an event object
 */

struct Event : public __evt
{
	/// constructor, see the constructor parameters of the class
	constexpr
	Event(): __evt _EVT_INIT() {}

	~Event() { assert(__evt::obj.queue == nullptr); }

	/// move constructor
	Event( Event&& _src ): __evt(_src)
	{
		assert(_src.obj.queue == nullptr);
	}

	Event( const Event& ) = delete;
	Event& operator=( Event&& ) = delete;
	Event& operator=( const Event& ) = delete;

	void reset    ()                                    {        evt_reset    (this); }                                ///< evt_reset()
	void kill     ()                                    {        evt_kill     (this); }                                ///< evt_kill()
	void destroy  ()                                    {        evt_destroy  (this); }                                ///< evt_destroy()
	template<typename T>
	int  waitFor  ( unsigned *_event, const T& _delay ) { return evt_waitFor  (this,  _event, Clock::count(_delay)); } ///< evt_waitFor()
	template<typename T>
	int  waitFor  ( unsigned &_event, const T& _delay ) { return evt_waitFor  (this, &_event, Clock::count(_delay)); } ///< evt_waitFor()
	template<typename T>
	int  waitUntil( unsigned *_event, const T& _time )  { return evt_waitUntil(this,  _event, Clock::until(_time)); }  ///< evt_waitUntil()
	template<typename T>
	int  waitUntil( unsigned &_event, const T& _time )  { return evt_waitUntil(this, &_event, Clock::until(_time)); }  ///< evt_waitUntil()
	int  wait     ( unsigned *_event = nullptr )        { return evt_wait     (this,  _event); }                       ///< evt_wait()
	int  wait     ( unsigned &_event )                  { return evt_wait     (this, &_event); }                       ///< evt_wait()
	void give     ( unsigned  _event )                  {        evt_give     (this,  _event); }                       ///< evt_give()
	void giveISR  ( unsigned  _event )                  {        evt_giveISR  (this,  _event); }                       ///< evt_giveISR()

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<Event>; ///< result type of Create()
#else
	using Ptr = Event *; ///< result type of Create()
#endif

/**
 * @brief   create dynamic object with manageable resources
 *
 * @return  std::unique_pointer / pointer to Event object
 *
 * @note    use only in thread mode
 */

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

/** @} */

#endif//__STATEOS_EVT_H
