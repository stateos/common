/******************************************************************************

    @file    IntrOS: oseventqueue.h
    @author  Rajmund Szymanski
    @date    22.07.2022
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

#ifndef __INTROS_EVQ_H
#define __INTROS_EVQ_H

#include "oskernel.h"

/******************************************************************************
 *
 * Name              : event queue
 *
 ******************************************************************************/

typedef struct __evq evq_t, * const evq_id;

struct __evq
{
	unsigned count; // inherited from semaphore
	unsigned limit; // inherited from semaphore

	unsigned head;  // first element to read from data buffer
	unsigned tail;  // first element to write into data buffer
	unsigned*data;  // data buffer
};

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

#define               _EVQ_INIT( _limit, _data ) { 0, _limit, 0, 0, _data }

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

/******************************************************************************
 *
 * Name              : OS_EVQ_BUFFER
 *
 * Description       : define event queue data buffer
 *
 * Parameters
 *   buf             : name of the buffer (passed to the init function)
 *   limit           : size of a queue (max number of stored events)
 *
 ******************************************************************************/

#define             OS_EVQ_BUFFER( buf, limit ) \
                       unsigned buf[limit]

/******************************************************************************
 *
 * Name              : OS_EVQ
 * Static alias      : static_EVQ
 *
 * Description       : define and initialize an event queue object
 *
 * Parameters
 *   evq             : name of a pointer to event queue object
 *   limit           : size of a queue (max number of stored events)
 *
 ******************************************************************************/

#define             OS_EVQ( evq, limit )                                \
                       unsigned evq##__buf[limit];                       \
                       evq_t evq##__evq = _EVQ_INIT( limit, evq##__buf ); \
                       evq_id evq = & evq##__evq

#define         static_EVQ( evq, limit )                                \
                static unsigned evq##__buf[limit];                       \
                static evq_t evq##__evq = _EVQ_INIT( limit, evq##__buf ); \
                static evq_id evq = & evq##__evq

/******************************************************************************
 *
 * Name              : EVQ_INIT
 *
 * Description       : create and initialize an event queue object
 *
 * Parameters
 *   limit           : size of a queue (max number of stored events)
 *
 * Return            : event queue object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                EVQ_INIT( limit ) \
                      _EVQ_INIT( limit, _EVQ_DATA( limit ) )
#endif

/******************************************************************************
 *
 * Name              : EVQ_CREATE
 * Alias             : EVQ_NEW
 *
 * Description       : create and initialize an event queue object
 *
 * Parameters
 *   limit           : size of a queue (max number of stored events)
 *
 * Return            : pointer to event queue object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                EVQ_CREATE( limit ) \
           (evq_t[]) { EVQ_INIT  ( limit ) }
#define                EVQ_NEW \
                       EVQ_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : evq_init
 *
 * Description       : initialize an event queue object
 *
 * Parameters
 *   evq             : pointer to event queue object
 *   data            : event queue data buffer
 *   bufsize         : size of the data buffer (in bytes)
 *
 * Return            : none
 *
 ******************************************************************************/

void evq_init( evq_t *evq, unsigned *data, size_t bufsize );

/******************************************************************************
 *
 * Name              : evq_take
 * Alias             : evq_tryWait
 * Async alias       : evq_takeAsync
 *
 * Description       : try to transfer event value from the event queue object,
 *                     don't wait if the event queue object is empty
 *
 * Parameters
 *   evq             : pointer to event queue object
 *
 * Return            : event value
 *   FAILURE         : event queue object is empty
 *
 ******************************************************************************/

unsigned evq_take( evq_t *evq );

__STATIC_INLINE
unsigned evq_tryWait( evq_t *evq ) { return evq_take(evq); }

#if OS_ATOMICS
__STATIC_INLINE
unsigned evq_takeAsync( evq_t *evq ) { return evq_take(evq); }
#endif

/******************************************************************************
 *
 * Name              : evq_wait
 * Async alias       : evq_waitAsync
 *
 * Description       : try to transfer event value from the event queue object,
 *                     wait indefinitely while the event queue object is empty
 *
 * Parameters
 *   evq             : pointer to event queue object
 *
 * Return            : event value
 *
 ******************************************************************************/

unsigned evq_wait( evq_t *evq );

#if OS_ATOMICS
__STATIC_INLINE
unsigned evq_waitAsync( evq_t *evq ) { return evq_wait(evq); }
#endif

/******************************************************************************
 *
 * Name              : evq_give
 * Async alias       : evq_giveAsync
 *
 * Description       : try to transfer event value to the event queue object,
 *                     don't wait if the event queue object is full
 *
 * Parameters
 *   evq             : pointer to event queue object
 *   event           : event value
 *
 * Return
 *   SUCCESS         : event value was successfully transferred to the event queue object
 *   FAILURE         : event queue object is full
 *
 ******************************************************************************/

unsigned evq_give( evq_t *evq, unsigned event );

#if OS_ATOMICS
__STATIC_INLINE
unsigned evq_giveAsync( evq_t *evq, unsigned event ) { return evq_give(evq, event); }
#endif

/******************************************************************************
 *
 * Name              : evq_send
 * Async alias       : evq_sendAsync
 *
 * Description       : try to transfer event value to the event queue object,
 *                     wait indefinitely while the event queue object is full
 *
 * Parameters
 *   evq             : pointer to event queue object
 *   event           : event value
 *
 * Return            : none
 *
 ******************************************************************************/

void evq_send( evq_t *evq, unsigned event );

#if OS_ATOMICS
__STATIC_INLINE
void evq_sendAsync( evq_t *evq, unsigned event ) { evq_send(evq, event); }
#endif

/******************************************************************************
 *
 * Name              : evq_push
 *
 * Description       : transfer event value to the event queue object,
 *                     remove the oldest event value if the event queue object is full
 *
 * Parameters
 *   evq             : pointer to event queue object
 *   event           : event value
 *
 * Return            : none
 *
 ******************************************************************************/

void evq_push( evq_t *evq, unsigned event );

/******************************************************************************
 *
 * Name              : evq_getCount
 *
 * Description       : return the amount of data contained in the event queue
 *
 * Parameters
 *   evq             : pointer to event queue object
 *
 * Return            : amount of data contained in the event queue
 *
 ******************************************************************************/

unsigned evq_getCount( evq_t *evq );

/******************************************************************************
 *
 * Name              : evq_getSpace
 *
 * Description       : return the amount of free space in the event queue
 *
 * Parameters
 *   evq             : pointer to event queue object
 *
 * Return            : amount of free space in the event queue
 *
 ******************************************************************************/

unsigned evq_getSpace( evq_t *evq );

/******************************************************************************
 *
 * Name              : evq_getLimit
 *
 * Description       : return the size of the event queue
 *
 * Parameters
 *   evq             : pointer to event queue object
 *
 * Return            : size of the event queue
 *
 ******************************************************************************/

unsigned evq_getLimit( evq_t *evq );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
namespace intros {

/******************************************************************************
 *
 * Class             : EventQueueT<>
 *
 * Description       : create and initialize an event queue object
 *
 * Constructor parameters
 *   limit           : size of a queue (max number of stored events)
 *
 ******************************************************************************/

template<unsigned limit_>
struct EventQueueT : public __evq
{
	constexpr
	EventQueueT(): __evq _EVQ_INIT(limit_, data_) {}

	EventQueueT( EventQueueT&& ) = default;
	EventQueueT( const EventQueueT& ) = delete;
	EventQueueT& operator=( EventQueueT&& ) = delete;
	EventQueueT& operator=( const EventQueueT& ) = delete;

	unsigned take     ()                  { return evq_take     (this); }
	unsigned tryWait  ()                  { return evq_tryWait  (this); }
	unsigned wait     ()                  { return evq_wait     (this); }
	unsigned give     ( unsigned _event ) { return evq_give     (this, _event); }
	void     send     ( unsigned _event ) {        evq_send     (this, _event); }
	void     push     ( unsigned _event ) {        evq_push     (this, _event); }
	unsigned getCount ()                  { return evq_getCount (this); }
	unsigned getSpace ()                  { return evq_getSpace (this); }
	unsigned getLimit ()                  { return evq_getLimit (this); }
#if OS_ATOMICS
	unsigned takeAsync()                  { return evq_takeAsync(this); }
	unsigned waitAsync()                  { return evq_waitAsync(this); }
	unsigned giveAsync( unsigned _event ) { return evq_giveAsync(this, _event); }
	void     sendAsync( unsigned _event ) {        evq_sendAsync(this, _event); }
#endif

	private:
	unsigned data_[limit_];
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_EVQ_H
