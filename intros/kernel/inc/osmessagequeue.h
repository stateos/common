/******************************************************************************

    @file    IntrOS: osmessagequeue.h
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

#ifndef __INTROS_MSG_H
#define __INTROS_MSG_H

#include "oskernel.h"

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
	size_t   count; // size of used memory in the message queue buffer (in bytes)
	size_t   limit; // size of the message queue buffer (in bytes)
	size_t   size;  // max size of a single message with preceding size of the maeesage (in bytes)

	size_t   head;  // index to read from the data buffer (in bytes)
	size_t   tail;  // index to write to the data buffer (in bytes)
	char *   data;  // data buffer

#ifdef __cplusplus
	void     init     ( size_t, void*, size_t );
	size_t   take     (       void*, size_t );
	size_t   tryWait  (       void*, size_t );
	size_t   wait     (       void*, size_t );
	unsigned give     ( const void*, size_t );
	unsigned send     ( const void*, size_t );
	unsigned push     ( const void*, size_t );
	unsigned getCount ();
	unsigned getSpace ();
	unsigned getLimit ();
	size_t   getSize  ();
#if OS_ATOMICS
	size_t   takeAsync(       void*, size_t );
	size_t   waitAsync(       void*, size_t );
	unsigned giveAsync( const void*, size_t );
	unsigned sendAsync( const void*, size_t );
#endif
#endif
};

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
                     { 0, _limit * MSG_SIZE(_size), MSG_SIZE(_size), 0, 0, _data }

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

#ifdef __cplusplus
extern "C" {
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
 ******************************************************************************/

void msg_init( msg_t *msg, size_t size, void *data, size_t bufsize );

/******************************************************************************
 *
 * Name              : msg_take
 * Alias             : msg_tryWait
 * Async alias       : msg_takeAsync
 *
 * Description       : try to transfer data from the message queue object,
 *                     don't wait if the message queue object is empty
 *
 * Parameters
 *   msg             : pointer to message queue object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *
 * Return            : number of read bytes
 *   0               : message queue object is empty or not enough space in the buffer
 *
 ******************************************************************************/

size_t msg_take( msg_t *msg, void *data, size_t size );

__STATIC_INLINE
size_t msg_tryWait( msg_t *msg, void *data, size_t size ) { return msg_take(msg, data, size); }

#if OS_ATOMICS
__STATIC_INLINE
size_t msg_takeAsync( msg_t *msg, void *data, size_t size ) { return msg_take(msg, data, size); }
#endif

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
 *
 * Return            : number of read bytes
 *   0               : not enough space in the buffer
 *
 ******************************************************************************/

size_t msg_wait( msg_t *msg, void *data, size_t size );

#if OS_ATOMICS
__STATIC_INLINE
size_t msg_waitAsync( msg_t *msg, void *data, size_t size ) { return msg_wait(msg, data, size); }
#endif

/******************************************************************************
 *
 * Name              : msg_give
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
 *   SUCCESS         : message data was successfully transferred to the message queue object
 *   FAILURE         : message queue object is full or too much data in the buffer
 *
 ******************************************************************************/

unsigned msg_give( msg_t *msg, const void *data, size_t size );

#if OS_ATOMICS
__STATIC_INLINE
unsigned msg_giveAsync( msg_t *msg, const void *data, size_t size ) { return msg_give(msg, data, size); }
#endif

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
 *   SUCCESS         : message data was successfully transferred to the message queue object
 *   FAILURE         : too much data in the buffer
 *
 ******************************************************************************/

unsigned msg_send( msg_t *msg, const void *data, size_t size );

#if OS_ATOMICS
__STATIC_INLINE
unsigned msg_sendAsync( msg_t *msg, const void *data, size_t size ) { return msg_send(msg, data, size); }
#endif

/******************************************************************************
 *
 * Name              : msg_push
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
 *   SUCCESS         : message data was successfully transferred to the message queue object
 *   FAILURE         : too much data in the buffer
 *
 ******************************************************************************/

unsigned msg_push( msg_t *msg, const void *data, size_t size );

/******************************************************************************
 *
 * Name              : msg_getCount
 *
 * Description       : return the amount of data contained in the message queue
 *
 * Parameters
 *   msg             : pointer to message queue object
 *
 * Return            : amount of data contained in the message queue
 *
 ******************************************************************************/

unsigned msg_getCount( msg_t *msg );

/******************************************************************************
 *
 * Name              : msg_getSpace
 *
 * Description       : return the amount of free space in the message queue
 *
 * Parameters
 *   msg             : pointer to message queue object
 *
 * Return            : amount of free space in the message queue
 *
 ******************************************************************************/

unsigned msg_getSpace( msg_t *msg );

/******************************************************************************
 *
 * Name              : msg_getLimit
 *
 * Description       : return the size of the message queue
 *
 * Parameters
 *   msg             : pointer to message queue object
 *
 * Return            : size of the message queue
 *
 ******************************************************************************/

unsigned msg_getLimit( msg_t *msg );

/******************************************************************************
 *
 * Name              : msg_getSize
 *
 * Description       : return max size of a message in the message queue
 *
 * Parameters
 *   msg             : pointer to message queue object
 *
 * Return            : max size of a message in the message queue
 *
 ******************************************************************************/

__STATIC_INLINE
size_t msg_getSize( msg_t *msg ) { return msg->size - sizeof(size_t); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus

inline void     __msg::init     ( size_t _size, void *_data, size_t _bufsize ) {        msg_init     (this, _size, _data, _bufsize); }
inline size_t   __msg::take     (       void *_data, size_t _size )            { return msg_take     (this, _data, _size); }
inline size_t   __msg::tryWait  (       void *_data, size_t _size )            { return msg_tryWait  (this, _data, _size); }
inline size_t   __msg::wait     (       void *_data, size_t _size )            { return msg_wait     (this, _data, _size); }
inline unsigned __msg::give     ( const void *_data, size_t _size )            { return msg_give     (this, _data, _size); }
inline unsigned __msg::send     ( const void *_data, size_t _size )            { return msg_send     (this, _data, _size); }
inline unsigned __msg::push     ( const void *_data, size_t _size )            { return msg_push     (this, _data, _size); }
inline unsigned __msg::getCount ()                                             { return msg_getCount (this); }
inline unsigned __msg::getSpace ()                                             { return msg_getSpace (this); }
inline unsigned __msg::getLimit ()                                             { return msg_getLimit (this); }
inline size_t   __msg::getSize  ()                                             { return msg_getSize  (this); }
#if OS_ATOMICS
inline size_t   __msg::takeAsync(       void *_data, size_t _size )            { return msg_takeAsync(this, _data, _size); }
inline size_t   __msg::waitAsync(       void *_data, size_t _size )            { return msg_waitAsync(this, _data, _size); }
inline unsigned __msg::giveAsync( const void *_data, size_t _size )            { return msg_giveAsync(this, _data, _size); }
inline unsigned __msg::sendAsync( const void *_data, size_t _size )            { return msg_sendAsync(this, _data, _size); }
#endif

namespace intros {

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

	MessageQueueT( MessageQueueT&& ) = default;
	MessageQueueT( const MessageQueueT& ) = delete;
	MessageQueueT& operator=( MessageQueueT&& ) = delete;
	MessageQueueT& operator=( const MessageQueueT& ) = delete;

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

	unsigned take     (       C *_data ) { return __msg::take     (_data, sizeof(C)) == sizeof(C) ? SUCCESS : FAILURE; }
	unsigned tryWait  (       C *_data ) { return __msg::tryWait  (_data, sizeof(C)) == sizeof(C) ? SUCCESS : FAILURE; }
	unsigned wait     (       C *_data ) { return __msg::wait     (_data, sizeof(C)) == sizeof(C) ? SUCCESS : FAILURE; }
	unsigned give     ( const C *_data ) { return __msg::give     (_data, sizeof(C)); }
	unsigned send     ( const C *_data ) { return __msg::send     (_data, sizeof(C)); }
	unsigned push     ( const C *_data ) { return __msg::push     (_data, sizeof(C)); }
#if OS_ATOMICS
	unsigned takeAsync(       C *_data ) { return __msg::takeAsync(_data, sizeof(C)) == sizeof(C) ? SUCCESS : FAILURE; }
	unsigned waitAsync(       C *_data ) { return __msg::waitAsync(_data, sizeof(C)) == sizeof(C) ? SUCCESS : FAILURE; }
	unsigned giveAsync( const C *_data ) { return __msg::giveAsync(_data, sizeof(C)); }
	unsigned sendAsync( const C *_data ) { return __msg::sendAsync(_data, sizeof(C)); }
#endif
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_MSG_H
