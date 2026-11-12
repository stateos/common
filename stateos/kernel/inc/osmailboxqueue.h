/******************************************************************************

    @file    StateOS: osmailboxqueue.h
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

#ifndef __STATEOS_BOX_H
#define __STATEOS_BOX_H

#include "oskernel.h"
#include "osclock.h"

/**
 * @defgroup mailboxqueue Mailbox queue
 * @brief    mailbox queues (messages of a fixed size)
 * @{
 */

/**
 * @brief   mailbox queue
 */

typedef struct __box box_t;

struct __box
{
	obj_t    obj;   ///< object header

	size_t   count; ///< size of used memory in the mailbox buffer (in bytes)
	size_t   limit; ///< size of the mailbox buffer (in bytes)
	size_t   size;  ///< size of a single mail (in bytes)

	size_t   head;  ///< first element to read from data buffer
	size_t   tail;  ///< first element to write into data buffer
	char *   data;  ///< data buffer
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

/**
 * @brief   define mailbox queue data buffer
 *
 * @param   buf        name of the buffer (passed to the init function)
 * @param   limit      size of a queue (max number of stored mails)
 * @param   size       size of a single mail (in bytes)
 */

#define             OS_BOX_BUFFER( buf, limit, size ) \
                       char buf[limit * size]

/**
 * @brief   define and initialize a mailbox queue object
 *
 * @param   box        name of a pointer to mailbox queue object
 * @param   limit      size of a queue (max number of stored mails)
 * @param   size       size of a single mail (in bytes)
 */

#define             OS_BOX( box, limit, size )       \
                static char box##__buf[limit * size]; \
                       box_t box[] = { _BOX_INIT( limit, size, box##__buf ) }

/** static alias of OS_BOX() */
#define         static_BOX( box, limit, size )       \
                static char box##__buf[limit * size]; \
                static box_t box[] = { _BOX_INIT( limit, size, box##__buf ) }

/**
 * @brief   create and initialize a mailbox queue object
 *
 * @param   limit      size of a queue (max number of stored mails)
 * @param   size       size of a single mail (in bytes)
 *
 * @return  mailbox queue object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                BOX_INIT( limit, size ) \
                      _BOX_INIT( limit, size, _BOX_DATA( limit, size ) )
#endif

/**
 * @brief   create and initialize a mailbox queue object
 *
 * @param   limit      size of a queue (max number of stored mails)
 * @param   size       size of a single mail (in bytes)
 *
 * @return  mailbox queue object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                BOX_CREATE( limit, size ) \
                     { BOX_INIT  ( limit, size ) }
/** alias of BOX_CREATE() */
#define                BOX_NEW \
                       BOX_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize a mailbox queue object
 *
 * @param   box        pointer to mailbox queue object
 * @param   size       size of a single mail (in bytes)
 * @param   data       mailbox queue data buffer
 * @param   bufsize    size of the data buffer (in bytes)
 *
 * @note    use only in thread mode
 */

void box_init( box_t *box, size_t size, void *data, size_t bufsize );

/**
 * @brief   create and initialize a new mailbox queue object
 *
 * @param   limit      size of a queue (max number of stored mails)
 * @param   size       size of a single mail (in bytes)
 *
 * @return  pointer to mailbox queue object
 * @retval  NULL       object not created (not enough free memory)
 *
 * @note    use only in thread mode
 */

box_t *box_create( unsigned limit, size_t size );

/** alias of box_create() */
__STATIC_INLINE
box_t *box_new( unsigned limit, size_t size ) { return box_create(limit, size); }

/**
 * @brief   reset the mailbox queue object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * @param   box        pointer to mailbox queue object
 *
 * @note    use only in thread mode
 */

void box_reset( box_t *box );

/** alias of box_reset() */
__STATIC_INLINE
void box_kill( box_t *box ) { box_reset(box); }

/**
 * @brief   reset the mailbox queue object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * @param   box        pointer to mailbox queue object
 *
 * @note    use only in thread mode
 */

void box_destroy( box_t *box );

/** alias of box_destroy() */
__STATIC_INLINE
void box_delete( box_t *box ) { box_destroy(box); }

/**
 * @brief   try to transfer mailbox data from the mailbox queue object,
 *          don't wait if the mailbox queue object is empty
 *
 * @param   box        pointer to mailbox queue object
 * @param   data       pointer to store mailbox data
 *
 * @retval  E_SUCCESS  mailbox data was successfully transferred from the mailbox queue object
 * @retval  E_TIMEOUT  mailbox queue object is empty, try again
 *
 * @note    can be used in both thread and handler mode
 * @note    use ISR alias in blockable interrupt handlers
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

int box_take( box_t *box, void *data );

/** alias of box_take() */
__STATIC_INLINE
int box_tryWait( box_t *box, void *data ) { return box_take(box, data); }

/** ISR alias of box_take() */
__STATIC_INLINE
int box_takeISR( box_t *box, void *data ) { return box_take(box, data); }

#if OS_ATOMICS
/** Async alias of box_take() */
int box_takeAsync( box_t *box, void *data );
#endif

/**
 * @brief   try to transfer mailbox data from the mailbox queue object,
 *          wait for given duration of time while the mailbox queue object is empty
 *
 * @param   box        pointer to mailbox queue object
 * @param   data       pointer to store mailbox data
 * @param   delay      duration of time (maximum number of ticks to wait while the mailbox queue object is empty)
 *                     - IMMEDIATE: don't wait if the mailbox queue object is empty
 *                     - INFINITE:  wait indefinitely while the mailbox queue object is empty
 *
 * @retval  E_SUCCESS  mailbox data was successfully transferred from the mailbox queue object
 * @retval  E_STOPPED  mailbox queue object was reseted before the specified timeout expired
 * @retval  E_DELETED  mailbox queue object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  mailbox queue object is empty and was not received data before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int box_waitFor( box_t *box, void *data, cnt_t delay );

/**
 * @brief   try to transfer mailbox data from the mailbox queue object,
 *          wait until given timepoint while the mailbox queue object is empty
 *
 * @param   box        pointer to mailbox queue object
 * @param   data       pointer to store mailbox data
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  mailbox data was successfully transferred from the mailbox queue object
 * @retval  E_STOPPED  mailbox queue object was reseted before the specified timeout expired
 * @retval  E_DELETED  mailbox queue object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  mailbox queue object is empty and was not received data before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int box_waitUntil( box_t *box, void *data, cnt_t time );

/**
 * @brief   try to transfer mailbox data from the mailbox queue object,
 *          wait indefinitely while the mailbox queue object is empty
 *
 * @param   box        pointer to mailbox queue object
 * @param   data       pointer to store mailbox data
 *
 * @retval  E_SUCCESS  mailbox data was successfully transferred from the mailbox queue object
 * @retval  E_STOPPED  mailbox queue object was reseted (unavailable for async version)
 * @retval  E_DELETED  mailbox queue object was deleted (unavailable for async version)
 *
 * @note    use only in thread mode
 * @note    use Async alias for communication with unmasked interrupt handlers
 * @note    the Async alias is intended for tasks of equal priority: it waits in a loop with tsk_yield,
 *          lower priority tasks do not run meanwhile
 */

__STATIC_INLINE
int box_wait( box_t *box, void *data ) { return box_waitFor(box, data, INFINITE); }

#if OS_ATOMICS
/** Async alias of box_wait() */
int box_waitAsync( box_t *box, void *data );
#endif

/**
 * @brief   try to transfer mailbox data to the mailbox queue object,
 *          don't wait if the mailbox queue object is full
 *
 * @param   box        pointer to mailbox queue object
 * @param   data       pointer to mailbox data
 *
 * @retval  E_SUCCESS  mailbox data was successfully transferred to the mailbox queue object
 * @retval  E_TIMEOUT  mailbox queue object is full, try again
 *
 * @note    can be used in both thread and handler mode
 * @note    use ISR alias in blockable interrupt handlers
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

int box_give( box_t *box, const void *data );

/** ISR alias of box_give() */
__STATIC_INLINE
int box_giveISR( box_t *box, const void *data ) { return box_give(box, data); }

#if OS_ATOMICS
/** Async alias of box_give() */
int box_giveAsync( box_t *box, const void *data );
#endif

/**
 * @brief   try to transfer mailbox data to the mailbox queue object,
 *          wait for given duration of time while the mailbox queue object is full
 *
 * @param   box        pointer to mailbox queue object
 * @param   data       pointer to mailbox data
 * @param   delay      duration of time (maximum number of ticks to wait while the mailbox queue object is full)
 *                     - IMMEDIATE: don't wait if the mailbox queue object is full
 *                     - INFINITE:  wait indefinitely while the mailbox queue object is full
 *
 * @retval  E_SUCCESS  mailbox data was successfully transferred to the mailbox queue object
 * @retval  E_STOPPED  mailbox queue object was reseted before the specified timeout expired
 * @retval  E_DELETED  mailbox queue object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  mailbox queue object is full and was not issued data before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int box_sendFor( box_t *box, const void *data, cnt_t delay );

/**
 * @brief   try to transfer mailbox data to the mailbox queue object,
 *          wait until given timepoint while the mailbox queue object is full
 *
 * @param   box        pointer to mailbox queue object
 * @param   data       pointer to mailbox data
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  mailbox data was successfully transferred to the mailbox queue object
 * @retval  E_STOPPED  mailbox queue object was reseted before the specified timeout expired
 * @retval  E_DELETED  mailbox queue object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  mailbox queue object is full and was not issued data before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int box_sendUntil( box_t *box, const void *data, cnt_t time );

/**
 * @brief   try to transfer mailbox data to the mailbox queue object,
 *          wait indefinitely while the mailbox queue object is full
 *
 * @param   box        pointer to mailbox queue object
 * @param   data       pointer to mailbox data
 *
 * @retval  E_SUCCESS  mailbox data was successfully transferred to the mailbox queue object
 * @retval  E_STOPPED  mailbox queue object was reseted (unavailable for async version)
 * @retval  E_DELETED  mailbox queue object was deleted (unavailable for async version)
 *
 * @note    use only in thread mode
 * @note    use Async alias for communication with unmasked interrupt handlers
 * @note    the Async alias is intended for tasks of equal priority: it waits in a loop with tsk_yield,
 *          lower priority tasks do not run meanwhile
 */

__STATIC_INLINE
int box_send( box_t *box, const void *data ) { return box_sendFor(box, data, INFINITE); }

#if OS_ATOMICS
/** Async alias of box_send() */
int box_sendAsync( box_t *box, const void *data );
#endif

/**
 * @brief   transfer mailbox data to the mailbox queue object,
 *          remove the oldest mailbox data if the mailbox queue object is full
 *
 * @param   box        pointer to mailbox queue object
 * @param   data       pointer to mailbox data
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

void box_push( box_t *box, const void *data );

/** ISR alias of box_push() */
__STATIC_INLINE
void box_pushISR( box_t *box, const void *data ) { box_push(box, data); }

/**
 * @brief   return the amount of data contained in the mailbox queue
 *
 * @param   box        pointer to mailbox queue object
 *
 * @return  amount of data contained in the mailbox queue
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

unsigned box_count( box_t *box );

/** ISR alias of box_count() */
__STATIC_INLINE
unsigned box_countISR( box_t *box ) { return box_count(box); }

/**
 * @brief   return the amount of free space in the mailbox queue
 *
 * @param   box        pointer to mailbox queue object
 *
 * @return  amount of free space in the mailbox queue
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

unsigned box_space( box_t *box );

/** ISR alias of box_space() */
__STATIC_INLINE
unsigned box_spaceISR( box_t *box ) { return box_space(box); }

/**
 * @brief   return the size of the mailbox queue
 *
 * @param   box        pointer to mailbox queue object
 *
 * @return  size of the mailbox queue
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

unsigned box_limit( box_t *box );

/** ISR alias of box_limit() */
__STATIC_INLINE
unsigned box_limitISR( box_t *box ) { return box_limit(box); }

/**
 * @brief   return size of a single mail
 *
 * @param   box        pointer to mailbox queue object
 *
 * @return  size of a single mail in the mailbox queue
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

__STATIC_INLINE
size_t box_size( box_t *box ) { return box->size; }

/** ISR alias of box_size() */
__STATIC_INLINE
size_t box_sizeISR( box_t *box ) { return box_size(box); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(__STATEOS_STDCXX)
namespace stateos {

/**
 * @brief   create and initialize a mailbox queue object
 *
 * @par     Constructor parameters
 *          - limit: size of a queue (max number of stored mails)
 *          - size: size of a single mail (in bytes)
 */

template<unsigned limit_, size_t size_>
struct MailBoxQueueT : public __box
{
	/// constructor, see the constructor parameters of the class
	constexpr
	MailBoxQueueT(): __box _BOX_INIT(limit_, size_, data_) {}

	~MailBoxQueueT() { assert(__box::obj.queue == nullptr); }

	/// move constructor
	MailBoxQueueT( MailBoxQueueT&& _src ): __box _BOX_INIT(limit_, size_, data_)
	{
		assert(_src.__box::count == 0 && _src.obj.queue == nullptr);
		(void) _src;
	}

	MailBoxQueueT( const MailBoxQueueT& ) = delete;
	MailBoxQueueT& operator=( MailBoxQueueT&& ) = delete;
	MailBoxQueueT& operator=( const MailBoxQueueT& ) = delete;

	void     reset    ()                                     {        box_reset    (this); }                              ///< box_reset()
	void     kill     ()                                     {        box_kill     (this); }                              ///< box_kill()
	void     destroy  ()                                     {        box_destroy  (this); }                              ///< box_destroy()
	int      take     (       void *_data )                  { return box_take     (this, _data); }                       ///< box_take()
	int      tryWait  (       void *_data )                  { return box_tryWait  (this, _data); }                       ///< box_tryWait()
	int      takeISR  (       void *_data )                  { return box_takeISR  (this, _data); }                       ///< box_takeISR()
	template<typename T>
	int      waitFor  (       void *_data, const T& _delay ) { return box_waitFor  (this, _data, Clock::count(_delay)); } ///< box_waitFor()
	template<typename T>
	int      waitUntil(       void *_data, const T& _time )  { return box_waitUntil(this, _data, Clock::until(_time)); }  ///< box_waitUntil()
	int      wait     (       void *_data )                  { return box_wait     (this, _data); }                       ///< box_wait()
	int      give     ( const void *_data )                  { return box_give     (this, _data); }                       ///< box_give()
	int      giveISR  ( const void *_data )                  { return box_giveISR  (this, _data); }                       ///< box_giveISR()
	template<typename T>
	int      sendFor  ( const void *_data, const T& _delay ) { return box_sendFor  (this, _data, Clock::count(_delay)); } ///< box_sendFor()
	template<typename T>
	int      sendUntil( const void *_data, const T& _time )  { return box_sendUntil(this, _data, Clock::until(_time)); }  ///< box_sendUntil()
	int      send     ( const void *_data )                  { return box_send     (this, _data); }                       ///< box_send()
	void     push     ( const void *_data )                  {        box_push     (this, _data); }                       ///< box_push()
	void     pushISR  ( const void *_data )                  {        box_pushISR  (this, _data); }                       ///< box_pushISR()
	unsigned count    ()                                     { return box_count    (this); }                              ///< box_count()
	unsigned countISR ()                                     { return box_countISR (this); }                              ///< box_countISR()
	unsigned space    ()                                     { return box_space    (this); }                              ///< box_space()
	unsigned spaceISR ()                                     { return box_spaceISR (this); }                              ///< box_spaceISR()
	unsigned limit    ()                                     { return box_limit    (this); }                              ///< box_limit()
	unsigned limitISR ()                                     { return box_limitISR (this); }                              ///< box_limitISR()
	size_t   size     ()                                     { return box_size     (this); }                              ///< box_size()
	size_t   sizeISR  ()                                     { return box_sizeISR  (this); }                              ///< box_sizeISR()
#if OS_ATOMICS
	int      takeAsync(       void *_data )                  { return box_takeAsync(this, _data); }                       ///< box_takeAsync()
	int      waitAsync(       void *_data )                  { return box_waitAsync(this, _data); }                       ///< box_waitAsync()
	int      giveAsync( const void *_data )                  { return box_giveAsync(this, _data); }                       ///< box_giveAsync()
	int      sendAsync( const void *_data )                  { return box_sendAsync(this, _data); }                       ///< box_sendAsync()
#endif

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<MailBoxQueueT<limit_, size_>>; ///< result type of Create()
#else
	using Ptr = MailBoxQueueT<limit_, size_> *; ///< result type of Create()
#endif

/**
 * @brief   create dynamic object with manageable resources
 *
 * @par     Parameters
 *          - limit: size of a queue (max number of stored mails)
 *          - size: size of a single mail (in bytes)
 *
 * @return  std::unique_pointer / pointer to MailBoxQueueT<> object
 *
 * @note    use only in thread mode
 */

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

/**
 * @brief   create and initialize a mailbox queue object
 *
 * @par     Constructor parameters
 *          - limit: size of a queue (max number of stored mails)
 *          - C: class of a single mail
 */

template<unsigned limit_, class C>
struct MailBoxQueueTT : public MailBoxQueueT<limit_, sizeof(C)>
{
	/// constructor, see the constructor parameters of the class
	constexpr
	MailBoxQueueTT(): MailBoxQueueT<limit_, sizeof(C)>() {}

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<MailBoxQueueTT<limit_, C>>; ///< result type of Create()
#else
	using Ptr = MailBoxQueueTT<limit_, C> *; ///< result type of Create()
#endif

/**
 * @brief   create dynamic object with manageable resources
 *
 * @par     Parameters
 *          - limit: size of a queue (max number of stored mails)
 *          - C: class of a single mail
 *
 * @return  std::unique_pointer / pointer to MailBoxQueueTT<> object
 *
 * @note    use only in thread mode
 */

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

/** @} */

#endif//__STATEOS_BOX_H
