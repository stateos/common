/******************************************************************************

    @file    StateOS: osmailboxqueue.h
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

#ifndef __STATEOS_BOX_H
#define __STATEOS_BOX_H

#include "oskernel.h"
#include "osclock.h"

/******************************************************************************
 *
 * Name              : mailbox queue
 *
 ******************************************************************************/

typedef struct __box box_t;

struct __box
{
	obj_t    obj;   // object header

	size_t   count; // size of used memory in the mailbox buffer (in bytes)
	size_t   limit; // size of the mailbox buffer (in bytes)
	size_t   size;  // size of a single mail (in bytes)

	size_t   head;  // first element to read from data buffer
	size_t   tail;  // first element to write into data buffer
	char *   data;  // data buffer
};

typedef struct __box box_id [];

/******************************************************************************
 *
 * Name              : _BOX_INIT
 *
 * Description       : create and initialize a mailbox queue object
 *
 * Parameters
 *   limit           : size of a queue (max number of stored mails)
 *   size            : size of a single mail (in bytes)
 *   data            : mailbox queue data buffer
 *
 * Return            : mailbox queue object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _BOX_INIT( _limit, _size, _data ) { _OBJ_INIT(), 0, _limit * _size, _size, 0, 0, _data }

/******************************************************************************
 *
 * Name              : _BOX_DATA
 *
 * Description       : create a mailbox queue data buffer
 *
 * Parameters
 *   limit           : size of a queue (max number of stored mails)
 *   size            : size of a single mail (in bytes)
 *
 * Return            : mailbox queue data buffer
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#ifndef __cplusplus
#define               _BOX_DATA( _limit, _size ) (char[_limit * _size]){ 0 }
#endif

/******************************************************************************
 *
 * Name              : OS_BOX_BUFFER
 *
 * Description       : define mailbox queue data buffer
 *
 * Parameters
 *   buf             : name of the buffer (passed to the init function)
 *   limit           : size of a queue (max number of stored mails)
 *   size            : size of a single mail (in bytes)
 *
 ******************************************************************************/

#define             OS_BOX_BUFFER( buf, limit, size ) \
                       char buf[limit * size]

/******************************************************************************
 *
 * Name              : OS_BOX
 * Static alias      : static_BOX
 *
 * Description       : define and initialize a mailbox queue object
 *
 * Parameters
 *   box             : name of a pointer to mailbox queue object
 *   limit           : size of a queue (max number of stored mails)
 *   size            : size of a single mail (in bytes)
 *
 ******************************************************************************/

#define             OS_BOX( box, limit, size )       \
                static char box##__buf[limit * size]; \
                       box_t box[] = { _BOX_INIT( limit, size, box##__buf ) }

#define         static_BOX( box, limit, size )       \
                static char box##__buf[limit * size]; \
                static box_t box[] = { _BOX_INIT( limit, size, box##__buf ) }

/******************************************************************************
 *
 * Name              : BOX_INIT
 *
 * Description       : create and initialize a mailbox queue object
 *
 * Parameters
 *   limit           : size of a queue (max number of stored mails)
 *   size            : size of a single mail (in bytes)
 *
 * Return            : mailbox queue object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                BOX_INIT( limit, size ) \
                      _BOX_INIT( limit, size, _BOX_DATA( limit, size ) )
#endif

/******************************************************************************
 *
 * Name              : BOX_CREATE
 * Alias             : BOX_NEW
 *
 * Description       : create and initialize a mailbox queue object
 *
 * Parameters
 *   limit           : size of a queue (max number of stored mails)
 *   size            : size of a single mail (in bytes)
 *
 * Return            : mailbox queue object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                BOX_CREATE( limit, size ) \
                     { BOX_INIT  ( limit, size ) }
#define                BOX_NEW \
                       BOX_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : box_init
 *
 * Description       : initialize a mailbox queue object
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *   size            : size of a single mail (in bytes)
 *   data            : mailbox queue data buffer
 *   bufsize         : size of the data buffer (in bytes)
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void box_init( box_t *box, size_t size, void *data, size_t bufsize );

/******************************************************************************
 *
 * Name              : box_create
 * Alias             : box_new
 *
 * Description       : create and initialize a new mailbox queue object
 *
 * Parameters
 *   limit           : size of a queue (max number of stored mails)
 *   size            : size of a single mail (in bytes)
 *
 * Return            : pointer to mailbox queue object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

box_t *box_create( unsigned limit, size_t size );

__STATIC_INLINE
box_t *box_new( unsigned limit, size_t size ) { return box_create(limit, size); }

/******************************************************************************
 *
 * Name              : box_reset
 * Alias             : box_kill
 *
 * Description       : reset the mailbox queue object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void box_reset( box_t *box );

__STATIC_INLINE
void box_kill( box_t *box ) { box_reset(box); }

/******************************************************************************
 *
 * Name              : box_destroy
 * Alias             : box_delete
 *
 * Description       : reset the mailbox queue object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void box_destroy( box_t *box );

__STATIC_INLINE
void box_delete( box_t *box ) { box_destroy(box); }

/******************************************************************************
 *
 * Name              : box_take
 * Alias             : box_tryWait
 * ISR alias         : box_takeISR
 * Async alias       : box_takeAsync
 *
 * Description       : try to transfer mailbox data from the mailbox queue object,
 *                     don't wait if the mailbox queue object is empty
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *   data            : pointer to store mailbox data
 *
 * Return
 *   E_SUCCESS       : mailbox data was successfully transferred from the mailbox queue object
 *   E_TIMEOUT       : mailbox queue object is empty, try again
 *
 * Note              : can be used in both thread and handler mode
 *                     use ISR alias in blockable interrupt handlers
 *                     use Async alias for communication with unmasked interrupt handlers
 *
 ******************************************************************************/

int box_take( box_t *box, void *data );

__STATIC_INLINE
int box_tryWait( box_t *box, void *data ) { return box_take(box, data); }

__STATIC_INLINE
int box_takeISR( box_t *box, void *data ) { return box_take(box, data); }

#if OS_ATOMICS
int box_takeAsync( box_t *box, void *data );
#endif

/******************************************************************************
 *
 * Name              : box_waitFor
 *
 * Description       : try to transfer mailbox data from the mailbox queue object,
 *                     wait for given duration of time while the mailbox queue object is empty
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *   data            : pointer to store mailbox data
 *   delay           : duration of time (maximum number of ticks to wait while the mailbox queue object is empty)
 *                     IMMEDIATE: don't wait if the mailbox queue object is empty
 *                     INFINITE:  wait indefinitely while the mailbox queue object is empty
 *
 * Return
 *   E_SUCCESS       : mailbox data was successfully transferred from the mailbox queue object
 *   E_STOPPED       : mailbox queue object was reseted before the specified timeout expired
 *   E_DELETED       : mailbox queue object was deleted before the specified timeout expired
 *   E_TIMEOUT       : mailbox queue object is empty and was not received data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int box_waitFor( box_t *box, void *data, cnt_t delay );

/******************************************************************************
 *
 * Name              : box_waitUntil
 *
 * Description       : try to transfer mailbox data from the mailbox queue object,
 *                     wait until given timepoint while the mailbox queue object is empty
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *   data            : pointer to store mailbox data
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : mailbox data was successfully transferred from the mailbox queue object
 *   E_STOPPED       : mailbox queue object was reseted before the specified timeout expired
 *   E_DELETED       : mailbox queue object was deleted before the specified timeout expired
 *   E_TIMEOUT       : mailbox queue object is empty and was not received data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int box_waitUntil( box_t *box, void *data, cnt_t time );

/******************************************************************************
 *
 * Name              : box_wait
 * Async alias       : box_waitAsync
 *
 * Description       : try to transfer mailbox data from the mailbox queue object,
 *                     wait indefinitely while the mailbox queue object is empty
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *   data            : pointer to store mailbox data
 *
 * Return
 *   E_SUCCESS       : mailbox data was successfully transferred from the mailbox queue object
 *   E_STOPPED       : mailbox queue object was reseted (unavailable for async version)
 *   E_DELETED       : mailbox queue object was deleted (unavailable for async version)
 *
 * Note              : use only in thread mode
 *                     use Async alias for communication with unmasked interrupt handlers
 *
 ******************************************************************************/

__STATIC_INLINE
int box_wait( box_t *box, void *data ) { return box_waitFor(box, data, INFINITE); }

#if OS_ATOMICS
int box_waitAsync( box_t *box, void *data );
#endif

/******************************************************************************
 *
 * Name              : box_give
 * ISR alias         : box_giveISR
 * Async alias       : box_giveAsync
 *
 * Description       : try to transfer mailbox data to the mailbox queue object,
 *                     don't wait if the mailbox queue object is full
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *   data            : pointer to mailbox data
 *
 * Return
 *   E_SUCCESS       : mailbox data was successfully transferred to the mailbox queue object
 *   E_TIMEOUT       : mailbox queue object is full, try again
 *
 * Note              : can be used in both thread and handler mode
 *                     use ISR alias in blockable interrupt handlers
 *                     use Async alias for communication with unmasked interrupt handlers
 *
 ******************************************************************************/

int box_give( box_t *box, const void *data );

__STATIC_INLINE
int box_giveISR( box_t *box, const void *data ) { return box_give(box, data); }

#if OS_ATOMICS
int box_giveAsync( box_t *box, const void *data );
#endif

/******************************************************************************
 *
 * Name              : box_sendFor
 *
 * Description       : try to transfer mailbox data to the mailbox queue object,
 *                     wait for given duration of time while the mailbox queue object is full
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *   data            : pointer to mailbox data
 *   delay           : duration of time (maximum number of ticks to wait while the mailbox queue object is full)
 *                     IMMEDIATE: don't wait if the mailbox queue object is full
 *                     INFINITE:  wait indefinitely while the mailbox queue object is full
 *
 * Return
 *   E_SUCCESS       : mailbox data was successfully transferred to the mailbox queue object
 *   E_STOPPED       : mailbox queue object was reseted before the specified timeout expired
 *   E_DELETED       : mailbox queue object was deleted before the specified timeout expired
 *   E_TIMEOUT       : mailbox queue object is full and was not issued data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int box_sendFor( box_t *box, const void *data, cnt_t delay );

/******************************************************************************
 *
 * Name              : box_sendUntil
 *
 * Description       : try to transfer mailbox data to the mailbox queue object,
 *                     wait until given timepoint while the mailbox queue object is full
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *   data            : pointer to mailbox data
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : mailbox data was successfully transferred to the mailbox queue object
 *   E_STOPPED       : mailbox queue object was reseted before the specified timeout expired
 *   E_DELETED       : mailbox queue object was deleted before the specified timeout expired
 *   E_TIMEOUT       : mailbox queue object is full and was not issued data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int box_sendUntil( box_t *box, const void *data, cnt_t time );

/******************************************************************************
 *
 * Name              : box_send
 * Async alias       : box_sendAsync
 *
 * Description       : try to transfer mailbox data to the mailbox queue object,
 *                     wait indefinitely while the mailbox queue object is full
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *   data            : pointer to mailbox data
 *
 * Return
 *   E_SUCCESS       : mailbox data was successfully transferred to the mailbox queue object
 *   E_STOPPED       : mailbox queue object was reseted (unavailable for async version)
 *   E_DELETED       : mailbox queue object was deleted (unavailable for async version)
 *
 * Note              : use only in thread mode
 *                     use Async alias for communication with unmasked interrupt handlers
 *
 ******************************************************************************/

__STATIC_INLINE
int box_send( box_t *box, const void *data ) { return box_sendFor(box, data, INFINITE); }

#if OS_ATOMICS
int box_sendAsync( box_t *box, const void *data );
#endif

/******************************************************************************
 *
 * Name              : box_push
 * ISR alias         : box_pushISR
 *
 * Description       : try to transfer mailbox data to the mailbox queue object,
 *                     remove the oldest mailbox data if the mailbox queue object is full
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *   data            : pointer to mailbox data
 *
 * Return            : none
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

void box_push( box_t *box, const void *data );

__STATIC_INLINE
void box_pushISR( box_t *box, const void *data ) { box_push(box, data); }

/******************************************************************************
 *
 * Name              : box_count
 * ISR alias         : box_countISR
 *
 * Description       : return the amount of data contained in the mailbox queue
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *
 * Return            : amount of data contained in the mailbox queue
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

unsigned box_count( box_t *box );

__STATIC_INLINE
unsigned box_countISR( box_t *box ) { return box_count(box); }

/******************************************************************************
 *
 * Name              : box_space
 * ISR alias         : box_spaceISR
 *
 * Description       : return the amount of free space in the mailbox queue
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *
 * Return            : amount of free space in the mailbox queue
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

unsigned box_space( box_t *box );

__STATIC_INLINE
unsigned box_spaceISR( box_t *box ) { return box_space(box); }

/******************************************************************************
 *
 * Name              : box_limit
 * ISR alias         : box_limitISR
 *
 * Description       : return the size of the mailbox queue
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *
 * Return            : size of the mailbox queue
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

unsigned box_limit( box_t *box );

__STATIC_INLINE
unsigned box_limitISR( box_t *box ) { return box_limit(box); }

/******************************************************************************
 *
 * Name              : box_size
 * ISR alias         : box_sizeISR
 *
 * Description       : return size of a single mail
 *
 * Parameters
 *   box             : pointer to mailbox queue object
 *
 * Return            : size of a single mail in the mailbox queue
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

__STATIC_INLINE
size_t box_size( box_t *box ) { return box->size; }

__STATIC_INLINE
size_t box_sizeISR( box_t *box ) { return box_size(box); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace stateos {

/******************************************************************************
 *
 * Class             : MailBoxQueueT<>
 *
 * Description       : create and initialize a mailbox queue object
 *
 * Constructor parameters
 *   limit           : size of a queue (max number of stored mails)
 *   size            : size of a single mail (in bytes)
 *
 ******************************************************************************/

template<unsigned limit_, size_t size_>
struct MailBoxQueueT : public __box
{
	constexpr
	MailBoxQueueT(): __box _BOX_INIT(limit_, size_, data_) {}

	~MailBoxQueueT() { assert(__box::obj.queue == nullptr); }

	MailBoxQueueT( MailBoxQueueT&& ) = default;
	MailBoxQueueT( const MailBoxQueueT& ) = delete;
	MailBoxQueueT& operator=( MailBoxQueueT&& ) = delete;
	MailBoxQueueT& operator=( const MailBoxQueueT& ) = delete;

	void     reset    ()                                     {        box_reset    (this); }
	void     kill     ()                                     {        box_kill     (this); }
	void     destroy  ()                                     {        box_destroy  (this); }
	int      take     (       void *_data )                  { return box_take     (this, _data); }
	int      tryWait  (       void *_data )                  { return box_tryWait  (this, _data); }
	int      takeISR  (       void *_data )                  { return box_takeISR  (this, _data); }
	template<typename T>
	int      waitFor  (       void *_data, const T& _delay ) { return box_waitFor  (this, _data, Clock::count(_delay)); }
	template<typename T>
	int      waitUntil(       void *_data, const T& _time )  { return box_waitUntil(this, _data, Clock::until(_time)); }
	int      wait     (       void *_data )                  { return box_wait     (this, _data); }
	int      give     ( const void *_data )                  { return box_give     (this, _data); }
	int      giveISR  ( const void *_data )                  { return box_giveISR  (this, _data); }
	template<typename T>
	int      sendFor  ( const void *_data, const T& _delay ) { return box_sendFor  (this, _data, Clock::count(_delay)); }
	template<typename T>
	int      sendUntil( const void *_data, const T& _time )  { return box_sendUntil(this, _data, Clock::until(_time)); }
	int      send     ( const void *_data )                  { return box_send     (this, _data); }
	void     push     ( const void *_data )                  {        box_push     (this, _data); }
	void     pushISR  ( const void *_data )                  {        box_pushISR  (this, _data); }
	unsigned count    ()                                     { return box_count    (this); }
	unsigned countISR ()                                     { return box_countISR (this); }
	unsigned space    ()                                     { return box_space    (this); }
	unsigned spaceISR ()                                     { return box_spaceISR (this); }
	unsigned limit    ()                                     { return box_limit    (this); }
	unsigned limitISR ()                                     { return box_limitISR (this); }
	size_t   size     ()                                     { return box_size     (this); }
	size_t   sizeISR  ()                                     { return box_sizeISR  (this); }
#if OS_ATOMICS
	int      takeAsync(       void *_data )                  { return box_takeAsync(this, _data); }
	int      waitAsync(       void *_data )                  { return box_waitAsync(this, _data); }
	int      giveAsync( const void *_data )                  { return box_giveAsync(this, _data); }
	int      sendAsync( const void *_data )                  { return box_sendAsync(this, _data); }
#endif

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<MailBoxQueueT<limit_, size_>>;
#else
	using Ptr = MailBoxQueueT<limit_, size_> *;
#endif

/******************************************************************************
 *
 * Name              : MailBoxQueueT<>::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters
 *   limit           : size of a queue (max number of stored mails)
 *   size            : size of a single mail (in bytes)
 *
 * Return            : std::unique_pointer / pointer to MailBoxQueueT<> object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create()
	{
		auto box = new (std::nothrow) MailBoxQueueT<limit_, size_>();
		if (box != nullptr)
			box->__box::obj.res = box;
		return Ptr(box);
	}

	private:
	char data_[limit_ * size_];
};

/******************************************************************************
 *
 * Class             : MailBoxQueueTT<>
 *
 * Description       : create and initialize a mailbox queue object
 *
 * Constructor parameters
 *   limit           : size of a queue (max number of stored mails)
 *   C               : class of a single mail
 *
 ******************************************************************************/

template<unsigned limit_, class C>
struct MailBoxQueueTT : public MailBoxQueueT<limit_, sizeof(C)>
{
	constexpr
	MailBoxQueueTT(): MailBoxQueueT<limit_, sizeof(C)>() {}

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<MailBoxQueueTT<limit_, C>>;
#else
	using Ptr = MailBoxQueueTT<limit_, C> *;
#endif

/******************************************************************************
 *
 * Name              : MailBoxQueueTT<>::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters
 *   limit           : size of a queue (max number of stored mails)
 *   C               : class of a single mail
 *
 * Return            : std::unique_pointer / pointer to MailBoxQueueTT<> object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create()
	{
		auto box = new (std::nothrow) MailBoxQueueTT<limit_, C>();
		if (box != nullptr)
			box->__box::obj.res = box;
		return Ptr(box);
	}

};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__STATEOS_BOX_H
