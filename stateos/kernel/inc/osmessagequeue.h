/******************************************************************************

    @file    StateOS: osmessagequeue.h
    @author  Rajmund Szymanski
    @date    12.07.2022
    @brief   This file contains definitions for StateOS.

 ******************************************************************************

   Copyright (c) 2018-2021 Rajmund Szymanski. All rights reserved.

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

#ifndef __STATEOS_MSG_H
#define __STATEOS_MSG_H

#include "oskernel.h"
#include "osclock.h"

/******************************************************************************
 *
 * Name              : message header
 *
 ******************************************************************************/

typedef struct __msh msh_t;

__PACKED_STRUCT __msh
{
	size_t   size;   // size of a message (in bytes)
#ifndef __cplusplus  // ISO C++ forbids flexible array member
	char     data[]; // message data
#endif
};

#define MSG_SIZE( size ) \
                ( size + sizeof(msh_t) )

/******************************************************************************
 *
 * Name              : message queue
 *
 ******************************************************************************/

typedef struct __msg msg_t, * const msg_id;

struct __msg
{
	obj_t    obj;   // object header

	size_t   count; // size of used memory in the message queue buffer (in bytes)
	size_t   limit; // size of the message queue buffer (in bytes)
	size_t   size;  // max size of a single message with preceding size of the maeesage (in bytes)

	size_t   head;  // index to read from the data buffer (in bytes)
	size_t   tail;  // index to write to the data buffer (in bytes)
	char *   data;  // data buffer
};

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : _MSG_INIT
 *
 * Description       : create and initialize a message queue object
 *
 * Parameters
 *   limit           : size of a queue (max number of stored messages)
 *   size            : max size of a single message (in bytes)
 *   data            : message queue data buffer
 *
 * Return            : message queue object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _MSG_INIT( _limit, _size, _data ) \
                    { _OBJ_INIT(), 0, _limit * MSG_SIZE(_size), MSG_SIZE(_size), 0, 0, _data }

/******************************************************************************
 *
 * Name              : _MSG_DATA
 *
 * Description       : create a message queue data
 *
 * Parameters
 *   limit           : size of a queue (max number of stored messages)
 *   size            : max size of a single message (in bytes)
 *
 * Return            : message queue data
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#ifndef __cplusplus
#define               _MSG_DATA( _limit, _size ) (char[_limit * MSG_SIZE(_size)]){ 0 }
#endif

/******************************************************************************
 *
 * Name              : OS_MSG_BUFFER
 *
 * Description       : define message queue data buffer
 *
 * Parameters
 *   buf             : name of the buffer (passed to the init function)
 *   limit           : size of a queue (max number of stored messages)
 *   size            : max size of a single message (in bytes)
 *
 ******************************************************************************/

#define             OS_MSG_BUFFER( buf, limit, size ) \
                       char buf[limit * MSG_SIZE(size)]

/******************************************************************************
 *
 * Name              : OS_MSG
 * Static alias      : static_MSG
 *
 * Description       : define and initialize a message queue object
 *
 * Parameters
 *   msg             : name of a pointer to message queue object
 *   limit           : size of a queue (max number of stored messages)
 *   size            : max size of a single message (in bytes)
 *
 ******************************************************************************/

#define             OS_MSG( msg, limit, size )                                \
                       char msg##__buf[limit * MSG_SIZE(size)];                \
                       msg_t msg##__msg = _MSG_INIT( limit, size, msg##__buf ); \
                       msg_id msg = & msg##__msg

#define         static_MSG( msg, limit, size )                                \
                static char msg##__buf[limit * MSG_SIZE(size)];                \
                static msg_t msg##__msg = _MSG_INIT( limit, size, msg##__buf ); \
                static msg_id msg = & msg##__msg

/******************************************************************************
 *
 * Name              : MSG_INIT
 *
 * Description       : create and initialize a message queue object
 *
 * Parameters
 *   limit           : size of a queue (max number of stored messages)
 *   size            : max size of a single message (in bytes)
 *
 * Return            : message queue object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                MSG_INIT( limit, size ) \
                      _MSG_INIT( limit, size, _MSG_DATA( limit, size ) )
#endif

/******************************************************************************
 *
 * Name              : MSG_CREATE
 * Alias             : MSG_NEW
 *
 * Description       : create and initialize a message queue object
 *
 * Parameters
 *   limit           : size of a queue (max number of stored messages)
 *   size            : max size of a single message (in bytes)
 *
 * Return            : pointer to message queue object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                MSG_CREATE( limit, size ) \
           (msg_t[]) { MSG_INIT  ( limit, size ) }
#define                MSG_NEW \
                       MSG_CREATE
#endif

/******************************************************************************
 *
 * Name              : msg_init
 *
 * Description       : initialize a message queue object
 *
 * Parameters
 *   msg             : pointer to message queue object
 *   size            : max size of a single message (in bytes)
 *   data            : message queue data buffer
 *   bufsize         : size of the data buffer (in bytes)
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void msg_init( msg_t *msg, size_t size, void *data, size_t bufsize );

/******************************************************************************
 *
 * Name              : msg_create
 * Alias             : msg_new
 *
 * Description       : create and initialize a new message queue object
 *
 * Parameters
 *   limit           : size of a queue (max number of stored messages)
 *   size            : max size of a single message (in bytes)
 *
 * Return            : pointer to message queue object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

msg_t *msg_create( unsigned limit, size_t size );

__STATIC_INLINE
msg_t *msg_new( unsigned limit, size_t size ) { return msg_create(limit, size); }

/******************************************************************************
 *
 * Name              : msg_reset
 * Alias             : msg_kill
 *
 * Description       : reset the message queue object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   msg             : pointer to message queue object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void msg_reset( msg_t *msg );

__STATIC_INLINE
void msg_kill( msg_t *msg ) { msg_reset(msg); }

/******************************************************************************
 *
 * Name              : msg_destroy
 * Alias             : msg_delete
 *
 * Description       : reset the message queue object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * Parameters
 *   msg             : pointer to message queue object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void msg_destroy( msg_t *msg );

__STATIC_INLINE
void msg_delete( msg_t *msg ) { msg_destroy(msg); }

/******************************************************************************
 *
 * Name              : msg_take
 * Alias             : msg_tryWait
 * ISR alias         : msg_takeISR
 * Async alias       : msg_takeAsync
 *
 * Description       : try to transfer data from the message queue object,
 *                     don't wait if the message queue object is empty
 *
 * Parameters
 *   msg             : pointer to message queue object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *   read            : pointer to the variable getting number of read bytes
 *
 * Return
 *   E_SUCCESS       : variable 'read' contains the number of bytes read from the message queue
 *   E_FAILURE       : not enough space in the buffer
 *   E_TIMEOUT       : message queue object is empty, try again
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *                     use Async alias for communication with unmasked interrupt handlers
 *
 ******************************************************************************/

int msg_take( msg_t *msg, void *data, size_t size, size_t *read );

__STATIC_INLINE
int msg_tryWait( msg_t *msg, void *data, size_t size, size_t *read ) { return msg_take(msg, data, size, read); }

__STATIC_INLINE
int msg_takeISR( msg_t *msg, void *data, size_t size, size_t *read ) { return msg_take(msg, data, size, read); }

#if OS_ATOMICS
int msg_takeAsync( msg_t *msg, void *data, size_t size, size_t *read );
#endif

/******************************************************************************
 *
 * Name              : msg_waitFor
 *
 * Description       : try to transfer data from the message queue object,
 *                     wait for given duration of time while the message queue object is empty
 *
 * Parameters
 *   msg             : pointer to message queue object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *   read            : pointer to the variable getting number of read bytes
 *   delay           : duration of time (maximum number of ticks to wait while the message queue object is empty)
 *                     IMMEDIATE: don't wait if the message queue object is empty
 *                     INFINITE:  wait indefinitely while the message queue object is empty
 *
 * Return
 *   E_SUCCESS       : variable 'read' contains the number of bytes read from the message queue
 *   E_FAILURE       : not enough space in the buffer
 *   E_STOPPED       : message queue object was reseted before the specified timeout expired
 *   E_DELETED       : message queue object was deleted before the specified timeout expired
 *   E_TIMEOUT       : message queue object is empty and was not received data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int msg_waitFor( msg_t *msg, void *data, size_t size, size_t *read, cnt_t delay );

/******************************************************************************
 *
 * Name              : msg_waitUntil
 *
 * Description       : try to transfer data from the message queue object,
 *                     wait until given timepoint while the message queue object is empty
 *
 * Parameters
 *   msg             : pointer to message queue object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *   read            : pointer to the variable getting number of read bytes
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : variable 'read' contains the number of bytes read from the message queue
 *   E_FAILURE       : not enough space in the buffer
 *   E_STOPPED       : message queue object was reseted before the specified timeout expired
 *   E_DELETED       : message queue object was deleted before the specified timeout expired
 *   E_TIMEOUT       : message queue object is empty and was not received data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int msg_waitUntil( msg_t *msg, void *data, size_t size, size_t *read, cnt_t time );

/******************************************************************************
 *
 * Name              : msg_wait
 * Async alias       : msg_waitAsync
 *
 * Description       : try to transfer data from the message queue object,
 *                     wait indefinitely while the message queue object is empty
 *
 * Parameters
 *   msg             : pointer to message queue object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *   read            : pointer to the variable getting number of read bytes
 *
 * Return
 *   E_SUCCESS       : variable 'read' contains the number of bytes read from the message queue
 *   E_FAILURE       : not enough space in the buffer
 *   E_STOPPED       : message queue object was reseted
 *   E_DELETED       : message queue object was deleted
 *
 * Note              : use only in thread mode
 *                     use Async alias for communication with unmasked interrupt handlers
 *
 ******************************************************************************/

__STATIC_INLINE
int msg_wait( msg_t *msg, void *data, size_t size, size_t *read ) { return msg_waitFor(msg, data, size, read, INFINITE); }

#if OS_ATOMICS
int msg_waitAsync( msg_t *msg, void *data, size_t size, size_t *read );
#endif

/******************************************************************************
 *
 * Name              : msg_give
 * ISR alias         : msg_giveISR
 * Async alias       : msg_giveAsync
 *
 * Description       : try to transfer data to the message queue object,
 *                     don't wait if the message queue object is full
 *
 * Parameters
 *   msg             : pointer to message queue object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *
 * Return
 *   E_SUCCESS       : message data was successfully transferred to the message queue object
 *   E_FAILURE       : too much data in the buffer
 *   E_TIMEOUT       : message queue object is full, try again
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *                     use Async alias for communication with unmasked interrupt handlers
 *
 ******************************************************************************/

int msg_give( msg_t *msg, const void *data, size_t size );

__STATIC_INLINE
int msg_giveISR( msg_t *msg, const void *data, size_t size ) { return msg_give(msg, data, size); }

#if OS_ATOMICS
int msg_giveAsync( msg_t *msg, const void *data, size_t size );
#endif

/******************************************************************************
 *
 * Name              : msg_sendFor
 *
 * Description       : try to transfer data to the message queue object,
 *                     wait for given duration of time while the message queue object is full
 *
 * Parameters
 *   msg             : pointer to message queue object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *   delay           : duration of time (maximum number of ticks to wait while the message queue object is full)
 *                     IMMEDIATE: don't wait if the message queue object is full
 *                     INFINITE:  wait indefinitely while the message queue object is full
 *
 * Return
 *   E_SUCCESS       : message data was successfully transferred to the message queue object
 *   E_FAILURE       : too much data in the buffer
 *   E_STOPPED       : message queue object was reseted before the specified timeout expired
 *   E_DELETED       : message queue object was deleted before the specified timeout expired
 *   E_TIMEOUT       : message queue object is full and was not issued data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int msg_sendFor( msg_t *msg, const void *data, size_t size, cnt_t delay );

/******************************************************************************
 *
 * Name              : msg_sendUntil
 *
 * Description       : try to transfer data to the message queue object,
 *                     wait until given timepoint while the message queue object is full
 *
 * Parameters
 *   msg             : pointer to message queue object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : message data was successfully transferred to the message queue object
 *   E_FAILURE       : too much data in the buffer
 *   E_STOPPED       : message queue object was reseted before the specified timeout expired
 *   E_DELETED       : message queue object was deleted before the specified timeout expired
 *   E_TIMEOUT       : message queue object is full and was not issued data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int msg_sendUntil( msg_t *msg, const void *data, size_t size, cnt_t time );

/******************************************************************************
 *
 * Name              : msg_send
 * Async alias       : msg_sendAsync
 *
 * Description       : try to transfer data to the message queue object,
 *                     wait indefinitely while the message queue object is full
 *
 * Parameters
 *   msg             : pointer to message queue object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *
 * Return
 *   E_SUCCESS       : message data was successfully transferred to the message queue object
 *   E_FAILURE       : too much data in the buffer
 *   E_STOPPED       : message queue object was reseted
 *   E_DELETED       : message queue object was deleted
 *
 * Note              : use only in thread mode
 *                     use Async alias for communication with unmasked interrupt handlers
 *
 ******************************************************************************/

__STATIC_INLINE
int msg_send( msg_t *msg, const void *data, size_t size ) { return msg_sendFor(msg, data, size, INFINITE); }

#if OS_ATOMICS
int msg_sendAsync( msg_t *msg, const void *data, size_t size );
#endif

/******************************************************************************
 *
 * Name              : msg_push
 * ISR alias         : msg_pushISR
 *
 * Description       : try to transfer data to the message queue object,
 *                     remove the oldest data if the message queue object is full
 *
 * Parameters
 *   msg             : pointer to message queue object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *
 * Return
 *   E_SUCCESS       : message data was successfully transferred to the message queue object
 *   E_FAILURE       : too much data in the buffer
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

int msg_push( msg_t *msg, const void *data, size_t size );

__STATIC_INLINE
int msg_pushISR( msg_t *msg, const void *data, size_t size ) { return msg_push(msg, data, size); }

/******************************************************************************
 *
 * Name              : msg_count
 * ISR alias         : msg_countISR
 *
 * Description       : return the amount of data contained in the message queue
 *
 * Parameters
 *   msg             : pointer to message queue object
 *
 * Return            : amount of data contained in the message queue
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

size_t msg_count( msg_t *msg );

__STATIC_INLINE
size_t msg_countISR( msg_t *msg ) { return msg_count(msg); }

/******************************************************************************
 *
 * Name              : msg_space
 * ISR alias         : msg_spaceISR
 *
 * Description       : return the amount of free space in the message queue
 *
 * Parameters
 *   msg             : pointer to message queue object
 *
 * Return            : amount of free space in the message queue
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

size_t msg_space( msg_t *msg );

__STATIC_INLINE
size_t msg_spaceISR( msg_t *msg ) { return msg_space(msg); }

/******************************************************************************
 *
 * Name              : msg_limit
 * ISR alias         : msg_limitISR
 *
 * Description       : return the size of the message queue
 *
 * Parameters
 *   msg             : pointer to message queue object
 *
 * Return            : size of the message queue
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

size_t msg_limit( msg_t *msg );

__STATIC_INLINE
size_t msg_limitISR( msg_t *msg ) { return msg_limit(msg); }

/******************************************************************************
 *
 * Name              : msg_size
 * ISR alias         : msg_sizeISR
 *
 * Description       : return max size of a message in the message queue
 *
 * Parameters
 *   msg             : pointer to message queue object
 *
 * Return            : max size of a message in the message queue
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

__STATIC_INLINE
size_t msg_size( msg_t *msg ) { return msg->size - sizeof(size_t); }

__STATIC_INLINE
size_t msg_sizeISR( msg_t *msg ) { return msg_size(msg); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace stateos {

/******************************************************************************
 *
 * Class             : MessageQueueT<>
 *
 * Description       : create and initialize a message queue object
 *
 * Constructor parameters
 *   limit           : size of a queue (max number of stored messages)
 *   size            : max size of a single message (in bytes)
 *
 ******************************************************************************/

template<unsigned limit_, size_t size_>
struct MessageQueueT : public __msg
{
	constexpr
	MessageQueueT(): __msg _MSG_INIT(limit_, size_, data_) {}

	~MessageQueueT() { assert(__msg::obj.queue == nullptr); }

	MessageQueueT( MessageQueueT&& ) = default;
	MessageQueueT( const MessageQueueT& ) = delete;
	MessageQueueT& operator=( MessageQueueT&& ) = delete;
	MessageQueueT& operator=( const MessageQueueT& ) = delete;

	void     reset    ()                                                                   {        msg_reset    (this); }
	void     kill     ()                                                                   {        msg_kill     (this); }
	void     destroy  ()                                                                   {        msg_destroy  (this); }
	int      take     (       void *_data, size_t _size, size_t *_read = nullptr )         { return msg_take     (this, _data, _size, _read); }
	int      tryWait  (       void *_data, size_t _size, size_t *_read = nullptr )         { return msg_tryWait  (this, _data, _size, _read); }
	int      takeISR  (       void *_data, size_t _size, size_t *_read = nullptr )         { return msg_takeISR  (this, _data, _size, _read); }
	template<typename T>
	int      waitFor  (       void *_data, size_t _size, size_t *_read,  const T& _delay ) { return msg_waitFor  (this, _data, _size, _read, Clock::count(_delay)); }
	template<typename T>
	int      waitUntil(       void *_data, size_t _size, size_t *_read,  const T& _time )  { return msg_waitUntil(this, _data, _size, _read, Clock::until(_time)); }
	int      wait     (       void *_data, size_t _size, size_t *_read = nullptr )         { return msg_wait     (this, _data, _size, _read); }
	int      give     ( const void *_data, size_t _size )                                  { return msg_give     (this, _data, _size); }
	int      giveISR  ( const void *_data, size_t _size )                                  { return msg_giveISR  (this, _data, _size); }
	template<typename T>
	int      sendFor  ( const void *_data, size_t _size, const T& _delay )                 { return msg_sendFor  (this, _data, _size, Clock::count(_delay)); }
	template<typename T>
	int      sendUntil( const void *_data, size_t _size, const T& _time )                  { return msg_sendUntil(this, _data, _size, Clock::until(_time)); }
	int      send     ( const void *_data, size_t _size )                                  { return msg_send     (this, _data, _size); }
	int      push     ( const void *_data, size_t _size )                                  { return msg_push     (this, _data, _size); }
	int      pushISR  ( const void *_data, size_t _size )                                  { return msg_pushISR  (this, _data, _size); }
	unsigned count    ()                                                                   { return msg_count    (this); }
	unsigned countISR ()                                                                   { return msg_countISR (this); }
	unsigned space    ()                                                                   { return msg_space    (this); }
	unsigned spaceISR ()                                                                   { return msg_spaceISR (this); }
	unsigned limit    ()                                                                   { return msg_limit    (this); }
	unsigned limitISR ()                                                                   { return msg_limitISR (this); }
	size_t   size     ()                                                                   { return msg_size     (this); }
	size_t   sizeISR  ()                                                                   { return msg_sizeISR  (this); }
#if OS_ATOMICS
	int      takeAsync(       void *_data, size_t _size, size_t *_read = nullptr )         { return msg_takeAsync(this, _data, _size, _read); }
	int      waitAsync(       void *_data, size_t _size, size_t *_read = nullptr )         { return msg_waitAsync(this, _data, _size, _read); }
	int      giveAsync( const void *_data, size_t _size )                                  { return msg_giveAsync(this, _data, _size); }
	int      sendAsync( const void *_data, size_t _size )                                  { return msg_sendAsync(this, _data, _size); }
#endif

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<MessageQueueT<limit_, size_>>;
#else
	using Ptr = MessageQueueT<limit_, size_> *;
#endif

/******************************************************************************
 *
 * Name              : MessageQueueT<>::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters
 *   limit           : size of a buffer (max number of stored bytes)
 *
 * Return            : std::unique_pointer / pointer to MessageQueueT<> object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create()
	{
		auto msg = new MessageQueueT<limit_, size_>();
		if (msg != nullptr)
			msg->__msg::obj.res = msg;
		return Ptr(msg);
	}

	private:
	char data_[limit_ * MSG_SIZE(size_)];
};

/******************************************************************************
 *
 * Class             : MessageQueueTT<>
 *
 * Description       : create and initialize a message queue object
 *
 * Constructor parameters
 *   limit           : size of a buffer (max number of stored objects)
 *   C               : class of a single message
 *
 ******************************************************************************/

template<unsigned limit_, class C>
struct MessageQueueTT : public MessageQueueT<limit_, sizeof(C)>
{
	constexpr
	MessageQueueTT(): MessageQueueT<limit_, sizeof(C)>() {}

	int      take     (       C *_data, size_t *_read = nullptr )         { return msg_take     (this, _data, sizeof(C), _read); }
	int      tryWait  (       C *_data, size_t *_read = nullptr )         { return msg_tryWait  (this, _data, sizeof(C), _read); }
	int      takeISR  (       C *_data, size_t *_read = nullptr )         { return msg_takeISR  (this, _data, sizeof(C), _read); }
	template<typename T>
	int      waitFor  (       C *_data, size_t *_read,  const T& _delay ) { return msg_waitFor  (this, _data, sizeof(C), _read, Clock::count(_delay)); }
	template<typename T>
	int      waitUntil(       C *_data, size_t *_read,  const T& _time )  { return msg_waitUntil(this, _data, sizeof(C), _read, Clock::until(_time)); }
	int      wait     (       C *_data, size_t *_read = nullptr )         { return msg_wait     (this, _data, sizeof(C), _read); }
	int      give     ( const C *_data )                                  { return msg_give     (this, _data, sizeof(C)); }
	int      giveISR  ( const C *_data )                                  { return msg_giveISR  (this, _data, sizeof(C)); }
	template<typename T>
	int      sendFor  ( const C *_data, const T& _delay )                 { return msg_sendFor  (this, _data, sizeof(C), Clock::count(_delay)); }
	template<typename T>
	int      sendUntil( const C *_data, const T& _time )                  { return msg_sendUntil(this, _data, sizeof(C), Clock::until(_time)); }
	int      send     ( const C *_data )                                  { return msg_send     (this, _data, sizeof(C)); }
	int      push     ( const C *_data )                                  { return msg_push     (this, _data, sizeof(C)); }
	int      pushISR  ( const C *_data )                                  { return msg_pushISR  (this, _data, sizeof(C)); }
#if OS_ATOMICS
	int      takeAsync(       C *_data, size_t *_read = nullptr )         { return msg_takeAsync(this, _data, sizeof(C), _read); }
	int      waitAsync(       C *_data, size_t *_read = nullptr )         { return msg_waitAsync(this, _data, sizeof(C), _read); }
	int      giveAsync( const C *_data )                                  { return msg_giveAsync(this, _data, sizeof(C)); }
	int      sendAsync( const C *_data )                                  { return msg_sendAsync(this, _data, sizeof(C)); }
#endif

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<MessageQueueTT<limit_, C>>;
#else
	using Ptr = MessageQueueTT<limit_, C> *;
#endif

/******************************************************************************
 *
 * Name              : MessageQueueTT<>::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters
 *   limit           : size of a buffer (max number of stored objects)
 *   C               : class of an object
 *
 * Return            : std::unique_pointer / pointer to MessageQueueTT<> object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create()
	{
		auto msg = new MessageQueueTT<limit_, C>();
		if (msg != nullptr)
			msg->__msg::obj.res = msg;
		return Ptr(msg);
	}

};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__STATEOS_MSG_H
