/******************************************************************************

    @file    IntrOS: osmessagequeue.h
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

#ifndef __INTROS_MSG_H
#define __INTROS_MSG_H

#include "oskernel.h"
#include "osclock.h"

/**
 * @defgroup messagequeue Message queue
 * @brief    message queues (messages of variable size)
 * @{
 */

/**
 * @brief   message header
 */

typedef struct __msh msh_t;

__PACKED_STRUCT __msh
{
	size_t   size;   ///< size of a message (in bytes)
#ifndef __cplusplus  // ISO C++ forbids flexible array member
	char     data[]; ///< message data
#endif
};

#define MSG_SIZE( size ) \
                ( (size_t)( size ) + sizeof(msh_t) )

/**
 * @brief   message queue
 */

typedef struct __msg msg_t;

struct __msg
{
	obj_t    obj;   ///< object header

	size_t   count; ///< size of used memory in the message queue buffer (in bytes)
	size_t   limit; ///< size of the message queue buffer (in bytes)
	size_t   size;  ///< max size of a single message with preceding size of the maeesage (in bytes)

	size_t   head;  ///< index to read from the data buffer (in bytes)
	size_t   tail;  ///< index to write to the data buffer (in bytes)
	char *   data;  ///< data buffer
};

typedef struct __msg msg_id [];

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

/**
 * @brief   define message queue data buffer
 *
 * @param   buf        name of the buffer (passed to the init function)
 * @param   limit      size of a queue (max number of stored messages)
 * @param   size       max size of a single message (in bytes)
 */

#define             OS_MSG_BUFFER( buf, limit, size ) \
                       char buf[limit * MSG_SIZE(size)]

/**
 * @brief   define and initialize a message queue object
 *
 * @param   msg        name of a pointer to message queue object
 * @param   limit      size of a queue (max number of stored messages)
 * @param   size       max size of a single message (in bytes)
 */

#define             OS_MSG( msg, limit, size )                 \
                static char msg##__buf[limit * MSG_SIZE(size)]; \
                       msg_t msg[] = { _MSG_INIT( limit, size, msg##__buf ) }

/** static alias of OS_MSG() */
#define         static_MSG( msg, limit, size )                 \
                static char msg##__buf[limit * MSG_SIZE(size)]; \
                static msg_t msg[] = { _MSG_INIT( limit, size, msg##__buf ) }

/**
 * @brief   create and initialize a message queue object
 *
 * @param   limit      size of a queue (max number of stored messages)
 * @param   size       max size of a single message (in bytes)
 *
 * @return  message queue object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                MSG_INIT( limit, size ) \
                      _MSG_INIT( limit, size, _MSG_DATA( limit, size ) )
#endif

/**
 * @brief   create and initialize a message queue object
 *
 * @param   limit      size of a queue (max number of stored messages)
 * @param   size       max size of a single message (in bytes)
 *
 * @return  message queue object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                MSG_CREATE( limit, size ) \
                     { MSG_INIT  ( limit, size ) }
/** alias of MSG_CREATE() */
#define                MSG_NEW \
                       MSG_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize a message queue object
 *
 * @param   msg        pointer to message queue object
 * @param   size       max size of a single message (in bytes)
 * @param   data       message queue data buffer
 * @param   bufsize    size of the data buffer (in bytes)
 */

void msg_init( msg_t *msg, size_t size, void *data, size_t bufsize );

/**
 * @brief   reset the message queue object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * @param   msg        pointer to message queue object
 *
 * @note    use only in thread mode
 */

void msg_reset( msg_t *msg );

/** alias of msg_reset() */
__STATIC_INLINE
void msg_kill( msg_t *msg ) { msg_reset(msg); }

/**
 * @brief   try to transfer data from the message queue object,
 *          don't wait if the message queue object is empty
 *
 * @param   msg        pointer to message queue object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 * @param   read       pointer to the variable getting number of read bytes
 *
 * @retval  E_SUCCESS  variable 'read' contains the number of bytes read from the message queue
 * @retval  E_FAILURE  not enough space in the buffer
 * @retval  E_TIMEOUT  message queue object is empty, try again
 *
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

int msg_take( msg_t *msg, void *data, size_t size, size_t *read );

/** alias of msg_take() */
__STATIC_INLINE
int msg_tryWait( msg_t *msg, void *data, size_t size, size_t *read ) { return msg_take(msg, data, size, read); }

#if OS_ATOMICS
/** Async alias of msg_take() */
int msg_takeAsync( msg_t *msg, void *data, size_t size, size_t *read );
#endif

/**
 * @brief   try to transfer data from the message queue object,
 *          wait for given duration of time while the message queue object is empty
 *
 * @param   msg        pointer to message queue object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 * @param   read       pointer to the variable getting number of read bytes
 * @param   delay      duration of time (maximum number of ticks to wait while the message queue object is empty)
 *                     - IMMEDIATE: don't wait if the message queue object is empty
 *                     - INFINITE:  wait indefinitely while the message queue object is empty
 *
 * @retval  E_SUCCESS  variable 'read' contains the number of bytes read from the message queue
 * @retval  E_FAILURE  not enough space in the buffer
 * @retval  E_STOPPED  message queue object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  message queue object is empty and was not received data before the specified timeout expired
 */

int msg_waitFor( msg_t *msg, void *data, size_t size, size_t *read, cnt_t delay );

/**
 * @brief   try to transfer data from the message queue object,
 *          wait until given timepoint while the message queue object is empty
 *
 * @param   msg        pointer to message queue object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 * @param   read       pointer to the variable getting number of read bytes
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  variable 'read' contains the number of bytes read from the message queue
 * @retval  E_FAILURE  not enough space in the buffer
 * @retval  E_STOPPED  message queue object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  message queue object is empty and was not received data before the specified timeout expired
 */

int msg_waitUntil( msg_t *msg, void *data, size_t size, size_t *read, cnt_t time );

/**
 * @brief   try to transfer data from the message queue object,
 *          wait indefinitely while the message queue object is empty
 *
 * @param   msg        pointer to message queue object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 * @param   read       pointer to the variable getting number of read bytes
 *
 * @retval  E_SUCCESS  variable 'read' contains the number of bytes read from the message queue
 * @retval  E_FAILURE  not enough space in the buffer
 * @retval  E_STOPPED  message queue object was reseted
 *
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

__STATIC_INLINE
int msg_wait( msg_t *msg, void *data, size_t size, size_t *read ) { return msg_waitFor(msg, data, size, read, INFINITE); }

#if OS_ATOMICS
/** Async alias of msg_wait() */
int msg_waitAsync( msg_t *msg, void *data, size_t size, size_t *read );
#endif

/**
 * @brief   try to transfer data to the message queue object,
 *          don't wait if the message queue object is full
 *
 * @param   msg        pointer to message queue object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 *
 * @retval  E_SUCCESS  message data was successfully transferred to the message queue object
 * @retval  E_FAILURE  too much data in the buffer
 * @retval  E_TIMEOUT  message queue object is full, try again
 *
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

int msg_give( msg_t *msg, const void *data, size_t size );

#if OS_ATOMICS
/** Async alias of msg_give() */
int msg_giveAsync( msg_t *msg, const void *data, size_t size );
#endif

/**
 * @brief   try to transfer data to the message queue object,
 *          wait for given duration of time while the message queue object is full
 *
 * @param   msg        pointer to message queue object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 * @param   delay      duration of time (maximum number of ticks to wait while the message queue object is full)
 *                     - IMMEDIATE: don't wait if the message queue object is full
 *                     - INFINITE:  wait indefinitely while the message queue object is full
 *
 * @retval  E_SUCCESS  message data was successfully transferred to the message queue object
 * @retval  E_FAILURE  too much data in the buffer
 * @retval  E_STOPPED  message queue object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  message queue object is full and was not issued data before the specified timeout expired
 */

int msg_sendFor( msg_t *msg, const void *data, size_t size, cnt_t delay );

/**
 * @brief   try to transfer data to the message queue object,
 *          wait until given timepoint while the message queue object is full
 *
 * @param   msg        pointer to message queue object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  message data was successfully transferred to the message queue object
 * @retval  E_FAILURE  too much data in the buffer
 * @retval  E_STOPPED  message queue object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  message queue object is full and was not issued data before the specified timeout expired
 */

int msg_sendUntil( msg_t *msg, const void *data, size_t size, cnt_t time );

/**
 * @brief   try to transfer data to the message queue object,
 *          wait indefinitely while the message queue object is full
 *
 * @param   msg        pointer to message queue object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 *
 * @retval  E_SUCCESS  message data was successfully transferred to the message queue object
 * @retval  E_FAILURE  too much data in the buffer
 * @retval  E_STOPPED  message queue object was reseted
 *
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

__STATIC_INLINE
int msg_send( msg_t *msg, const void *data, size_t size ) { return msg_sendFor(msg, data, size, INFINITE); }

#if OS_ATOMICS
/** Async alias of msg_send() */
int msg_sendAsync( msg_t *msg, const void *data, size_t size );
#endif

/**
 * @brief   try to transfer data to the message queue object,
 *          remove the oldest data if the message queue object is full
 *
 * @param   msg        pointer to message queue object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 *
 * @retval  E_SUCCESS  message data was successfully transferred to the message queue object
 * @retval  E_FAILURE  too much data in the buffer
 */

int msg_push( msg_t *msg, const void *data, size_t size );

/**
 * @brief   return the amount of data contained in the message queue
 *
 * @param   msg        pointer to message queue object
 *
 * @return  amount of data contained in the message queue
 */

size_t msg_count( msg_t *msg );

/**
 * @brief   return the amount of free space in the message queue
 *
 * @param   msg        pointer to message queue object
 *
 * @return  amount of free space in the message queue
 */

size_t msg_space( msg_t *msg );

/**
 * @brief   return the size of the message queue
 *
 * @param   msg        pointer to message queue object
 *
 * @return  size of the message queue
 */

size_t msg_limit( msg_t *msg );

/**
 * @brief   return max size of a message in the message queue
 *
 * @param   msg        pointer to message queue object
 *
 * @return  max size of a message in the message queue
 */

__STATIC_INLINE
size_t msg_size( msg_t *msg ) { return msg->size - sizeof(size_t); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus)
namespace intros {

/**
 * @brief   create and initialize a message queue object
 *
 * @par     Constructor parameters
 *          - limit: size of a queue (max number of stored messages)
 *          - size: max size of a single message (in bytes)
 */

template<unsigned limit_, size_t size_>
struct MessageQueueT : public __msg
{
	/// constructor, see the constructor parameters of the class
	constexpr
	MessageQueueT(): __msg _MSG_INIT(limit_, size_, data_) {}

	~MessageQueueT() { assert(__msg::obj.queue == nullptr); }

	/// move constructor
	MessageQueueT( MessageQueueT&& _src ): __msg _MSG_INIT(limit_, size_, data_)
	{
		assert(_src.__msg::count == 0 && _src.obj.queue == nullptr);
		(void) _src;
	}

	MessageQueueT( const MessageQueueT& ) = delete;
	MessageQueueT& operator=( MessageQueueT&& ) = delete;
	MessageQueueT& operator=( const MessageQueueT& ) = delete;

	void     reset    ()                                                                   {        msg_reset    (this); }                                            ///< msg_reset()
	void     kill     ()                                                                   {        msg_kill     (this); }                                            ///< msg_kill()
	int      take     (       void *_data, size_t _size, size_t *_read = nullptr )         { return msg_take     (this, _data, _size, _read); }                       ///< msg_take()
	int      tryWait  (       void *_data, size_t _size, size_t *_read = nullptr )         { return msg_tryWait  (this, _data, _size, _read); }                       ///< msg_tryWait()
	template<typename T>
	int      waitFor  (       void *_data, size_t _size, size_t *_read,  const T& _delay ) { return msg_waitFor  (this, _data, _size, _read, Clock::count(_delay)); } ///< msg_waitFor()
	template<typename T>
	int      waitUntil(       void *_data, size_t _size, size_t *_read,  const T& _time )  { return msg_waitUntil(this, _data, _size, _read, Clock::until(_time)); }  ///< msg_waitUntil()
	int      wait     (       void *_data, size_t _size, size_t *_read = nullptr )         { return msg_wait     (this, _data, _size, _read); }                       ///< msg_wait()
	int      give     ( const void *_data, size_t _size )                                  { return msg_give     (this, _data, _size); }                              ///< msg_give()
	template<typename T>
	int      sendFor  ( const void *_data, size_t _size, const T& _delay )                 { return msg_sendFor  (this, _data, _size, Clock::count(_delay)); }        ///< msg_sendFor()
	template<typename T>
	int      sendUntil( const void *_data, size_t _size, const T& _time )                  { return msg_sendUntil(this, _data, _size, Clock::until(_time)); }         ///< msg_sendUntil()
	int      send     ( const void *_data, size_t _size )                                  { return msg_send     (this, _data, _size); }                              ///< msg_send()
	int      push     ( const void *_data, size_t _size )                                  { return msg_push     (this, _data, _size); }                              ///< msg_push()
	size_t   count    ()                                                                   { return msg_count    (this); }                                            ///< msg_count()
	size_t   space    ()                                                                   { return msg_space    (this); }                                            ///< msg_space()
	size_t   limit    ()                                                                   { return msg_limit    (this); }                                            ///< msg_limit()
	size_t   size     ()                                                                   { return msg_size     (this); }                                            ///< msg_size()
#if OS_ATOMICS
	int      takeAsync(       void *_data, size_t _size, size_t *_read = nullptr )         { return msg_takeAsync(this, _data, _size, _read); }                       ///< msg_takeAsync()
	int      waitAsync(       void *_data, size_t _size, size_t *_read = nullptr )         { return msg_waitAsync(this, _data, _size, _read); }                       ///< msg_waitAsync()
	int      giveAsync( const void *_data, size_t _size )                                  { return msg_giveAsync(this, _data, _size); }                              ///< msg_giveAsync()
	int      sendAsync( const void *_data, size_t _size )                                  { return msg_sendAsync(this, _data, _size); }                              ///< msg_sendAsync()
#endif

	private:
	char data_[limit_ * MSG_SIZE(size_)];
};

/**
 * @brief   create and initialize a message queue object
 *
 * @par     Constructor parameters
 *          - limit: size of a buffer (max number of stored objects)
 *          - C: class of a single message
 */

template<unsigned limit_, class C>
struct MessageQueueTT : public MessageQueueT<limit_, sizeof(C)>
{
	/// constructor, see the constructor parameters of the class
	constexpr
	MessageQueueTT(): MessageQueueT<limit_, sizeof(C)>() {}

	int      take     (       C *_data, size_t *_read = nullptr )         { return msg_take     (this, _data, sizeof(C), _read); }                       ///< msg_take()
	int      tryWait  (       C *_data, size_t *_read = nullptr )         { return msg_tryWait  (this, _data, sizeof(C), _read); }                       ///< msg_tryWait()
	template<typename T>
	int      waitFor  (       C *_data, size_t *_read,  const T& _delay ) { return msg_waitFor  (this, _data, sizeof(C), _read, Clock::count(_delay)); } ///< msg_waitFor()
	template<typename T>
	int      waitUntil(       C *_data, size_t *_read,  const T& _time )  { return msg_waitUntil(this, _data, sizeof(C), _read, Clock::until(_time)); }  ///< msg_waitUntil()
	int      wait     (       C *_data, size_t *_read = nullptr )         { return msg_wait     (this, _data, sizeof(C), _read); }                       ///< msg_wait()
	int      give     ( const C *_data )                                  { return msg_give     (this, _data, sizeof(C)); }                              ///< msg_give()
	template<typename T>
	int      sendFor  ( const C *_data, const T& _delay )                 { return msg_sendFor  (this, _data, sizeof(C), Clock::count(_delay)); }        ///< msg_sendFor()
	template<typename T>
	int      sendUntil( const C *_data, const T& _time )                  { return msg_sendUntil(this, _data, sizeof(C), Clock::until(_time)); }         ///< msg_sendUntil()
	int      send     ( const C *_data )                                  { return msg_send     (this, _data, sizeof(C)); }                              ///< msg_send()
	int      push     ( const C *_data )                                  { return msg_push     (this, _data, sizeof(C)); }                              ///< msg_push()
#if OS_ATOMICS
	int      takeAsync(       C *_data, size_t *_read = nullptr )         { return msg_takeAsync(this, _data, sizeof(C), _read); }                       ///< msg_takeAsync()
	int      waitAsync(       C *_data, size_t *_read = nullptr )         { return msg_waitAsync(this, _data, sizeof(C), _read); }                       ///< msg_waitAsync()
	int      giveAsync( const C *_data )                                  { return msg_giveAsync(this, _data, sizeof(C)); }                              ///< msg_giveAsync()
	int      sendAsync( const C *_data )                                  { return msg_sendAsync(this, _data, sizeof(C)); }                              ///< msg_sendAsync()
#endif
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

/** @} */

#endif//__INTROS_MSG_H
