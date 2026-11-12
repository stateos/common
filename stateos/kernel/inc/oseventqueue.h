/******************************************************************************

    @file    StateOS: oseventqueue.h
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

#ifndef __STATEOS_EVQ_H
#define __STATEOS_EVQ_H

#include "oskernel.h"
#include "osclock.h"

/**
 * @defgroup eventqueue Event queue
 * @brief    event queues (unsigned values)
 * @{
 */

/**
 * @brief   event queue
 */

typedef struct __evq evq_t;

struct __evq
{
	obj_t    obj;   ///< object header

	unsigned count; ///< inherited from semaphore
	unsigned limit; ///< inherited from semaphore

	unsigned head;  ///< first element to read from data buffer
	unsigned tail;  ///< first element to write into data buffer
	unsigned*data;  ///< data buffer
};

typedef struct __evq evq_id [];

/******************************************************************************
 *
 * Name              : _EVQ_INIT
 *
 * Description       : create and initialize an event queue object
 *
 * Parameters
 *   limit           : size of a queue (max number of stored events)
 *   data            : event queue data buffer
 *
 * Return            : event queue object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _EVQ_INIT( _limit, _data ) { _OBJ_INIT(), 0, _limit, 0, 0, _data }

/******************************************************************************
 *
 * Name              : _EVQ_DATA
 *
 * Description       : create an event queue data buffer
 *
 * Parameters
 *   limit           : size of a queue (max number of stored events)
 *
 * Return            : event queue data buffer
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#ifndef __cplusplus
#define               _EVQ_DATA( _limit ) (unsigned[_limit]){ 0 }
#endif

/**
 * @brief   define event queue data buffer
 *
 * @param   buf        name of the buffer (passed to the init function)
 * @param   limit      size of a queue (max number of stored events)
 */

#define             OS_EVQ_BUFFER( buf, limit ) \
                       unsigned buf[limit]

/**
 * @brief   define and initialize an event queue object
 *
 * @param   evq        name of a pointer to event queue object
 * @param   limit      size of a queue (max number of stored events)
 */

#define             OS_EVQ( evq, limit )          \
                static unsigned evq##__buf[limit]; \
                       evq_t evq[] = { _EVQ_INIT( limit, evq##__buf ) }

/** static alias of OS_EVQ() */
#define         static_EVQ( evq, limit )          \
                static unsigned evq##__buf[limit]; \
                static evq_t evq[] = { _EVQ_INIT( limit, evq##__buf ) }

/**
 * @brief   create and initialize an event queue object
 *
 * @param   limit      size of a queue (max number of stored events)
 *
 * @return  event queue object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                EVQ_INIT( limit ) \
                      _EVQ_INIT( limit, _EVQ_DATA( limit ) )
#endif

/**
 * @brief   create and initialize an event queue object
 *
 * @param   limit      size of a queue (max number of stored events)
 *
 * @return  event queue object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                EVQ_CREATE( limit ) \
                     { EVQ_INIT  ( limit ) }
/** alias of EVQ_CREATE() */
#define                EVQ_NEW \
                       EVQ_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize an event queue object
 *
 * @param   evq        pointer to event queue object
 * @param   data       event queue data buffer
 * @param   bufsize    size of the data buffer (in bytes)
 *
 * @note    use only in thread mode
 */

void evq_init( evq_t *evq, unsigned *data, size_t bufsize );

/**
 * @brief   create and initialize a new event queue object
 *
 * @param   limit      size of a queue (max number of stored events)
 *
 * @return  pointer to event queue object
 * @retval  NULL       object not created (not enough free memory)
 *
 * @note    use only in thread mode
 */

evq_t *evq_create( unsigned limit );

/** alias of evq_create() */
__STATIC_INLINE
evq_t *evq_new( unsigned limit ) { return evq_create(limit); }

/**
 * @brief   reset the event queue object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * @param   evq        pointer to event queue object
 *
 * @note    use only in thread mode
 */

void evq_reset( evq_t *evq );

/** alias of evq_reset() */
__STATIC_INLINE
void evq_kill( evq_t *evq ) { evq_reset(evq); }

/**
 * @brief   reset the event queue object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * @param   evq        pointer to event queue object
 *
 * @note    use only in thread mode
 */

void evq_destroy( evq_t *evq );

/** alias of evq_destroy() */
__STATIC_INLINE
void evq_delete( evq_t *evq ) { evq_destroy(evq); }

/**
 * @brief   try to transfer event value from the event queue object,
 *          don't wait if the event queue object is empty
 *
 * @param   evq        pointer to event queue object
 * @param   event      pointer to store event value
 *
 * @retval  E_SUCCESS  event value was successfully transferred from the event queue object
 * @retval  E_TIMEOUT  event queue object is empty, try again
 *
 * @note    can be used in both thread and handler mode
 * @note    use ISR alias in blockable interrupt handlers
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

int evq_take( evq_t *evq, unsigned *event );

/** alias of evq_take() */
__STATIC_INLINE
int evq_tryWait( evq_t *evq, unsigned *event ) { return evq_take(evq, event); }

/** ISR alias of evq_take() */
__STATIC_INLINE
int evq_takeISR( evq_t *evq, unsigned *event ) { return evq_take(evq, event); }

#if OS_ATOMICS
/** Async alias of evq_take() */
int evq_takeAsync( evq_t *evq, unsigned *event );
#endif

/**
 * @brief   try to transfer event value from the event queue object,
 *          wait for given duration of time while the event queue object is empty
 *
 * @param   evq        pointer to event queue object
 * @param   event      pointer to store event value
 * @param   delay      duration of time (maximum number of ticks to wait while the event queue object is empty)
 *                     - IMMEDIATE: don't wait if the event queue object is empty
 *                     - INFINITE:  wait indefinitely while the event queue object is empty
 *
 * @retval  E_SUCCESS  event value was successfully transferred from the event queue object
 * @retval  E_STOPPED  event queue object was reseted before the specified timeout expired
 * @retval  E_DELETED  event queue object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  event queue object is empty and was not received data before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int evq_waitFor( evq_t *evq, unsigned *event, cnt_t delay );

/**
 * @brief   try to transfer event value from the event queue object,
 *          wait until given timepoint while the event queue object is empty
 *
 * @param   evq        pointer to event queue object
 * @param   event      pointer to store event value
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  event value was successfully transferred from the event queue object
 * @retval  E_STOPPED  event queue object was reseted before the specified timeout expired
 * @retval  E_DELETED  event queue object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  event queue object is empty and was not received data before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int evq_waitUntil( evq_t *evq, unsigned *event, cnt_t time );

/**
 * @brief   try to transfer event value from the event queue object,
 *          wait indefinitely while the event queue object is empty
 *
 * @param   evq        pointer to event queue object
 * @param   event      pointer to store event value
 *
 * @retval  E_SUCCESS  event value was successfully transferred from the event queue object
 * @retval  E_STOPPED  event queue object was reseted (unavailable for async version)
 * @retval  E_DELETED  event queue object was deleted (unavailable for async version)
 *
 * @note    use only in thread mode
 * @note    use Async alias for communication with unmasked interrupt handlers
 * @note    the Async alias is intended for tasks of equal priority: it waits in a loop with tsk_yield,
 *          lower priority tasks do not run meanwhile
 */

__STATIC_INLINE
int evq_wait( evq_t *evq, unsigned *event ) { return evq_waitFor(evq, event, INFINITE); }

#if OS_ATOMICS
/** Async alias of evq_wait() */
int evq_waitAsync( evq_t *evq, unsigned *event );
#endif

/**
 * @brief   try to transfer event value to the event queue object,
 *          don't wait if the event queue object is full
 *
 * @param   evq        pointer to event queue object
 * @param   event      event value
 *
 * @retval  E_SUCCESS  event value was successfully transferred to the event queue object
 * @retval  E_TIMEOUT  event queue object is full, try again
 *
 * @note    can be used in both thread and handler mode
 * @note    use ISR alias in blockable interrupt handlers
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

int evq_give( evq_t *evq, unsigned event );

/** ISR alias of evq_give() */
__STATIC_INLINE
int evq_giveISR( evq_t *evq, unsigned event ) { return evq_give(evq, event); }

#if OS_ATOMICS
/** Async alias of evq_give() */
int evq_giveAsync( evq_t *evq, unsigned event );
#endif

/**
 * @brief   try to transfer event value to the event queue object,
 *          wait for given duration of time while the event queue object is full
 *
 * @param   evq        pointer to event queue object
 * @param   event      event value
 * @param   delay      duration of time (maximum number of ticks to wait while the event queue object is full)
 *                     - IMMEDIATE: don't wait if the event queue object is full
 *                     - INFINITE:  wait indefinitely while the event queue object is full
 *
 * @retval  E_SUCCESS  event value was successfully transferred to the event queue object
 * @retval  E_STOPPED  event queue object was reseted before the specified timeout expired
 * @retval  E_DELETED  event queue object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  event queue object is full and was not issued data before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int evq_sendFor( evq_t *evq, unsigned event, cnt_t delay );

/**
 * @brief   try to transfer event value to the event queue object,
 *          wait until given timepoint while the event queue object is full
 *
 * @param   evq        pointer to event queue object
 * @param   event      event value
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  event value was successfully transferred to the event queue object
 * @retval  E_STOPPED  event queue object was reseted before the specified timeout expired
 * @retval  E_DELETED  event queue object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  event queue object is full and was not issued data before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int evq_sendUntil( evq_t *evq, unsigned event, cnt_t time );

/**
 * @brief   try to transfer event value to the event queue object,
 *          wait indefinitely while the event queue object is full
 *
 * @param   evq        pointer to event queue object
 * @param   event      event value
 *
 * @retval  E_SUCCESS  event value was successfully transferred to the event queue object
 * @retval  E_STOPPED  event queue object was reseted (unavailable for async version)
 * @retval  E_DELETED  event queue object was deleted (unavailable for async version)
 *
 * @note    use only in thread mode
 * @note    use Async alias for communication with unmasked interrupt handlers
 * @note    the Async alias is intended for tasks of equal priority: it waits in a loop with tsk_yield,
 *          lower priority tasks do not run meanwhile
 */

__STATIC_INLINE
int evq_send( evq_t *evq, unsigned event ) { return evq_sendFor(evq, event, INFINITE); }

#if OS_ATOMICS
/** Async alias of evq_send() */
int evq_sendAsync( evq_t *evq, unsigned event );
#endif

/**
 * @brief   transfer event value to the event queue object,
 *          remove the oldest event value if the event queue object is full
 *
 * @param   evq        pointer to event queue object
 * @param   event      event value
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

void evq_push( evq_t *evq, unsigned event );

/** ISR alias of evq_push() */
__STATIC_INLINE
void evq_pushISR( evq_t *evq, unsigned event ) { evq_push(evq, event); }

/**
 * @brief   return the amount of data contained in the event queue
 *
 * @param   evq        pointer to event queue object
 *
 * @return  amount of data contained in the event queue
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

unsigned evq_count( evq_t *evq );

/** ISR alias of evq_count() */
__STATIC_INLINE
unsigned evq_countISR( evq_t *evq ) { return evq_count(evq); }

/**
 * @brief   return the amount of free space in the event queue
 *
 * @param   evq        pointer to event queue object
 *
 * @return  amount of free space in the event queue
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

unsigned evq_space( evq_t *evq );

/** ISR alias of evq_space() */
__STATIC_INLINE
unsigned evq_spaceISR( evq_t *evq ) { return evq_space(evq); }

/**
 * @brief   return the size of the event queue
 *
 * @param   evq        pointer to event queue object
 *
 * @return  size of the event queue
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

unsigned evq_limit( evq_t *evq );

/** ISR alias of evq_limit() */
__STATIC_INLINE
unsigned evq_limitISR( evq_t *evq ) { return evq_limit(evq); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(__STATEOS_STDCXX)
namespace stateos {

/**
 * @brief   create and initialize an event queue object
 *
 * @par     Constructor parameters
 *          - limit: size of a queue (max number of stored events)
 */

template<unsigned limit_>
struct EventQueueT : public __evq
{
	/// constructor, see the constructor parameters of the class
	constexpr
	EventQueueT(): __evq _EVQ_INIT(limit_, data_) {}

	~EventQueueT() { assert(__evq::obj.queue == nullptr); }

	/// move constructor
	EventQueueT( EventQueueT&& _src ): __evq _EVQ_INIT(limit_, data_)
	{
		assert(_src.__evq::count == 0 && _src.obj.queue == nullptr);
		(void) _src;
	}

	EventQueueT( const EventQueueT& ) = delete;
	EventQueueT& operator=( EventQueueT&& ) = delete;
	EventQueueT& operator=( const EventQueueT& ) = delete;

	void     reset    ()                                    {        evq_reset    (this); }                                ///< evq_reset()
	void     kill     ()                                    {        evq_kill     (this); }                                ///< evq_kill()
	void     destroy  ()                                    {        evq_destroy  (this); }                                ///< evq_destroy()
	int      take     ( unsigned *_event = nullptr )        { return evq_take     (this,  _event); }                       ///< evq_take()
	int      take     ( unsigned &_event )                  { return evq_take     (this, &_event); }                       ///< evq_take()
	int      tryWait  ( unsigned *_event = nullptr )        { return evq_tryWait  (this,  _event); }                       ///< evq_tryWait()
	int      tryWait  ( unsigned &_event )                  { return evq_tryWait  (this, &_event); }                       ///< evq_tryWait()
	int      takeISR  ( unsigned *_event )                  { return evq_takeISR  (this,  _event); }                       ///< evq_takeISR()
	template<typename T>
	int      waitFor  ( unsigned *_event, const T& _delay ) { return evq_waitFor  (this,  _event, Clock::count(_delay)); } ///< evq_waitFor()
	template<typename T>
	int      waitUntil( unsigned *_event, const T& _time )  { return evq_waitUntil(this,  _event, Clock::until(_time)); }  ///< evq_waitUntil()
	int      wait     ( unsigned *_event = nullptr )        { return evq_wait     (this,  _event); }                       ///< evq_wait()
	int      wait     ( unsigned &_event )                  { return evq_wait     (this, &_event); }                       ///< evq_wait()
	int      give     ( unsigned  _event )                  { return evq_give     (this,  _event); }                       ///< evq_give()
	int      giveISR  ( unsigned  _event )                  { return evq_giveISR  (this,  _event); }                       ///< evq_giveISR()
	template<typename T>
	int      sendFor  ( unsigned  _event, const T& _delay ) { return evq_sendFor  (this,  _event, Clock::count(_delay)); } ///< evq_sendFor()
	template<typename T>
	int      sendUntil( unsigned  _event, const T& _time )  { return evq_sendUntil(this,  _event, Clock::until(_time)); }  ///< evq_sendUntil()
	int      send     ( unsigned  _event )                  { return evq_send     (this,  _event); }                       ///< evq_send()
	void     push     ( unsigned  _event )                  {        evq_push     (this,  _event); }                       ///< evq_push()
	void     pushISR  ( unsigned  _event )                  {        evq_pushISR  (this,  _event); }                       ///< evq_pushISR()
	unsigned count    ()                                    { return evq_count    (this); }                                ///< evq_count()
	unsigned countISR ()                                    { return evq_countISR (this); }                                ///< evq_countISR()
	unsigned space    ()                                    { return evq_space    (this); }                                ///< evq_space()
	unsigned spaceISR ()                                    { return evq_spaceISR (this); }                                ///< evq_spaceISR()
	unsigned limit    ()                                    { return evq_limit    (this); }                                ///< evq_limit()
	unsigned limitISR ()                                    { return evq_limitISR (this); }                                ///< evq_limitISR()
#if OS_ATOMICS
	int      takeAsync( unsigned *_event = nullptr )        { return evq_takeAsync(this,  _event); }                       ///< evq_takeAsync()
	int      takeAsync( unsigned &_event )                  { return evq_takeAsync(this, &_event); }                       ///< evq_takeAsync()
	int      waitAsync( unsigned *_event = nullptr )        { return evq_waitAsync(this,  _event); }                       ///< evq_waitAsync()
	int      waitAsync( unsigned &_event )                  { return evq_waitAsync(this, &_event); }                       ///< evq_waitAsync()
	int      giveAsync( unsigned  _event )                  { return evq_giveAsync(this,  _event); }                       ///< evq_giveAsync()
	int      sendAsync( unsigned  _event )                  { return evq_sendAsync(this,  _event); }                       ///< evq_sendAsync()
#endif

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<EventQueueT<limit_>>; ///< result type of Create()
#else
	using Ptr = EventQueueT<limit_> *; ///< result type of Create()
#endif

/**
 * @brief   create dynamic object with manageable resources
 *
 * @par     Parameters
 *          - limit: size of a queue (max number of stored events)
 *
 * @return  std::unique_pointer / pointer to EventQueueT<> object
 *
 * @note    use only in thread mode
 */

	static
	Ptr Create()
	{
		auto evq = new (std::nothrow) EventQueueT<limit_>();
		if (evq != nullptr)
			evq->__evq::obj.res = evq;
		return Ptr(evq);
	}

	private:
	unsigned data_[limit_];
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

/** @} */

#endif//__STATEOS_EVQ_H
