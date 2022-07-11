/******************************************************************************

    @file    StateOS: osrawbuffer.h
    @author  Rajmund Szymanski
    @date    11.07.2022
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

#ifndef __STATEOS_RAW_H
#define __STATEOS_RAW_H

#include "oskernel.h"
#include "osclock.h"

/******************************************************************************
 *
 * Name              : raw buffer
 *
 ******************************************************************************/

typedef struct __raw raw_t, * const raw_id;

struct __raw
{
	obj_t    obj;   // object header

	size_t   count; // size of used memory in the raw buffer (in bytes)
	size_t   limit; // size of the raw buffer (in bytes)

	size_t   head;  // first element to read from data buffer
	size_t   tail;  // first element to write into data buffer
	char *   data;  // data buffer
};

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : _RAW_INIT
 *
 * Description       : create and initialize a raw buffer object
 *
 * Parameters
 *   limit           : size of a buffer (max number of stored bytes)
 *   data            : raw buffer data
 *
 * Return            : raw buffer object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _RAW_INIT( _limit, _data ) { _OBJ_INIT(), 0, _limit, 0, 0, _data }

/******************************************************************************
 *
 * Name              : _RAW_DATA
 *
 * Description       : create a raw buffer data
 *
 * Parameters
 *   limit           : size of a buffer (max number of stored bytes)
 *
 * Return            : raw buffer data
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#ifndef __cplusplus
#define               _RAW_DATA( _limit ) (char[_limit]){ 0 }
#endif

/******************************************************************************
 *
 * Name              : _VA_RAW
 *
 * Description       : calculate buffer size from optional parameter
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _VA_RAW( _limit, _size ) ( (_size + 0) ? ((_limit) * (_size + 0)) : (_limit) )

/******************************************************************************
 *
 * Name              : OS_RAW_BUFFER
 *
 * Description       : define raw data buffer
 *
 * Parameters
 *   buf             : name of the buffer (passed to the init function)
 *   limit           : size of a buffer (max number of stored bytes / objects)
 *   size            : (optional) size of the object (in bytes); default: 1
 *
 ******************************************************************************/

#define             OS_RAW_BUFFER( buf, limit, ... ) \
                       char buf[_VA_RAW(limit, __VA_ARGS__)]

/******************************************************************************
 *
 * Name              : OS_RAW
 * Static alias      : static_RAW
 *
 * Description       : define and initialize a raw buffer object
 *
 * Parameters
 *   raw             : name of a pointer to raw buffer object
 *   limit           : size of a buffer (max number of stored bytes / objects)
 *   size            : (optional) size of the object (in bytes); default: 1
 *
 ******************************************************************************/

#define             OS_RAW( raw, limit, ... )                                                 \
                       char raw##__buf[_VA_RAW(limit, __VA_ARGS__)];                           \
                       raw_t raw##__raw = _RAW_INIT( _VA_RAW(limit, __VA_ARGS__), raw##__buf ); \
                       raw_id raw = & raw##__raw

#define         static_RAW( raw, limit, ... )                                                 \
                static char raw##__buf[_VA_RAW(limit, __VA_ARGS__)];                           \
                static raw_t raw##__raw = _RAW_INIT( _VA_RAW(limit, __VA_ARGS__), raw##__buf ); \
                static raw_id raw = & raw##__raw

/******************************************************************************
 *
 * Name              : RAW_INIT
 *
 * Description       : create and initialize a raw buffer object
 *
 * Parameters
 *   limit           : size of a buffer (max number of stored bytes / objects)
 *   type            : (optional) size of the object (in bytes); default: 1
 *
 * Return            : raw buffer object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                RAW_INIT( limit, ... ) \
                      _RAW_INIT( _VA_RAW(limit, __VA_ARGS__), _RAW_DATA( _VA_RAW(limit, __VA_ARGS__) ) )
#endif

/******************************************************************************
 *
 * Name              : RAW_CREATE
 * Alias             : RAW_NEW
 *
 * Description       : create and initialize a raw buffer object
 *
 * Parameters
 *   limit           : size of a buffer (max number of stored bytes / objects)
 *   type            : (optional) size of the object (in bytes); default: 1
 *
 * Return            : pointer to raw buffer object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                RAW_CREATE( limit, ... ) \
           (raw_t[]) { RAW_INIT  ( _VA_RAW(limit, __VA_ARGS__) ) }
#define                RAW_NEW \
                       RAW_CREATE
#endif

/******************************************************************************
 *
 * Name              : raw_init
 *
 * Description       : initialize a raw buffer object
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *   data            : raw buffer data
 *   bufsize         : size of the data buffer (in bytes)
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void raw_init( raw_t *raw, void *data, size_t bufsize );

/******************************************************************************
 *
 * Name              : raw_create
 * Alias             : raw_new
 *
 * Description       : create and initialize a new raw buffer object
 *
 * Parameters
 *   limit           : size of a buffer (max number of stored bytes)
 *
 * Return            : pointer to raw buffer object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

raw_t *raw_create( size_t limit );

__STATIC_INLINE
raw_t *raw_new( size_t limit ) { return raw_create(limit); }

/******************************************************************************
 *
 * Name              : raw_reset
 * Alias             : raw_kill
 *
 * Description       : reset the raw buffer object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void raw_reset( raw_t *raw );

__STATIC_INLINE
void raw_kill( raw_t *raw ) { raw_reset(raw); }

/******************************************************************************
 *
 * Name              : raw_destroy
 * Alias             : raw_delete
 *
 * Description       : reset the raw buffer object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void raw_destroy( raw_t *raw );

__STATIC_INLINE
void raw_delete( raw_t *raw ) { raw_destroy(raw); }

/******************************************************************************
 *
 * Name              : raw_take
 * Alias             : raw_tryWait
 * ISR alias         : raw_takeISR
 *
 * Description       : try to transfer data from the raw buffer object,
 *                     don't wait if the raw buffer object is empty
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *   read            : pointer to the variable getting number of read bytes
 *
 * Return
 *   E_SUCCESS       : variable 'read' contains the number of bytes read from the raw buffer
 *   E_TIMEOUT       : raw buffer object is empty, try again
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

int raw_take( raw_t *raw, void *data, size_t size, size_t *read );

__STATIC_INLINE
int raw_tryWait( raw_t *raw, void *data, size_t size, size_t *read ) { return raw_take(raw, data, size, read); }

__STATIC_INLINE
int raw_takeISR( raw_t *raw, void *data, size_t size, size_t *read ) { return raw_take(raw, data, size, read); }

/******************************************************************************
 *
 * Name              : raw_waitFor
 *
 * Description       : try to transfer data from the raw buffer object,
 *                     wait for given duration of time while the raw buffer object is empty
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *   read            : pointer to the variable getting number of read bytes
 *   delay           : duration of time (maximum number of ticks to wait while the raw buffer object is empty)
 *                     IMMEDIATE: don't wait if the raw buffer object is empty
 *                     INFINITE:  wait indefinitely while the raw buffer object is empty
 *
 * Return
 *   E_SUCCESS       : variable 'read' contains the number of bytes read from the raw buffer
 *   E_STOPPED       : raw buffer object was reseted before the specified timeout expired
 *   E_DELETED       : raw buffer object was deleted before the specified timeout expired
 *   E_TIMEOUT       : raw buffer object is empty and was not received data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int raw_waitFor( raw_t *raw, void *data, size_t size, size_t *read, cnt_t delay );

/******************************************************************************
 *
 * Name              : raw_waitUntil
 *
 * Description       : try to transfer data from the raw buffer object,
 *                     wait until given timepoint while the raw buffer object is empty
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *   read            : pointer to the variable getting number of read bytes
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : variable 'read' contains the number of bytes read from the raw buffer
 *   E_STOPPED       : raw buffer object was reseted before the specified timeout expired
 *   E_DELETED       : raw buffer object was deleted before the specified timeout expired
 *   E_TIMEOUT       : raw buffer object is empty and was not received data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int raw_waitUntil( raw_t *raw, void *data, size_t size, size_t *read, cnt_t time );

/******************************************************************************
 *
 * Name              : raw_wait
 *
 * Description       : try to transfer data from the raw buffer object,
 *                     wait indefinitely while the raw buffer object is empty
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *   read            : pointer to the variable getting number of read bytes
 *
 * Return
 *   E_SUCCESS       : variable 'read' contains the number of bytes read from the raw buffer
 *   E_STOPPED       : raw buffer object was reseted
 *   E_DELETED       : raw buffer object was deleted
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
int raw_wait( raw_t *raw, void *data, size_t size, size_t *read ) { return raw_waitFor(raw, data, size, read, INFINITE); }

/******************************************************************************
 *
 * Name              : raw_give
 * ISR alias         : raw_giveISR
 *
 * Description       : try to transfer data to the raw buffer object,
 *                     don't wait if the raw buffer object is full
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *
 * Return
 *   E_SUCCESS       : data was successfully transferred to the raw buffer object
 *   E_FAILURE       : size of the data is out of the limit
 *   E_TIMEOUT       : not enough space in the raw buffer, try again
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

int raw_give( raw_t *raw, const void *data, size_t size );

__STATIC_INLINE
int raw_giveISR( raw_t *raw, const void *data, size_t size ) { return raw_give(raw, data, size); }

/******************************************************************************
 *
 * Name              : raw_sendFor
 *
 * Description       : try to transfer data to the raw buffer object,
 *                     wait for given duration of time while the raw buffer object is full
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *   delay           : duration of time (maximum number of ticks to wait while the raw buffer object is full)
 *                     IMMEDIATE: don't wait if the raw buffer object is full
 *                     INFINITE:  wait indefinitely while the raw buffer object is full
 *
 * Return
 *   E_SUCCESS       : data was successfully transferred to the raw buffer object
 *   E_FAILURE       : size of the data is out of the limit
 *   E_STOPPED       : raw buffer object was reseted before the specified timeout expired
 *   E_DELETED       : raw buffer object was deleted before the specified timeout expired
 *   E_TIMEOUT       : raw buffer object is full and was not issued data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int raw_sendFor( raw_t *raw, const void *data, size_t size, cnt_t delay );

/******************************************************************************
 *
 * Name              : raw_sendUntil
 *
 * Description       : try to transfer data to the raw buffer object,
 *                     wait until given timepoint while the raw buffer object is full
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : data was successfully transferred to the raw buffer object
 *   E_FAILURE       : size of the data is out of the limit
 *   E_STOPPED       : raw buffer object was reseted before the specified timeout expired
 *   E_DELETED       : raw buffer object was deleted before the specified timeout expired
 *   E_TIMEOUT       : raw buffer object is full and was not issued data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int raw_sendUntil( raw_t *raw, const void *data, size_t size, cnt_t time );

/******************************************************************************
 *
 * Name              : raw_send
 *
 * Description       : try to transfer data to the raw buffer object,
 *                     wait indefinitely while the raw buffer object is full
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *
 * Return
 *   E_SUCCESS       : data was successfully transferred to the raw buffer object
 *   E_FAILURE       : size of the data is out of the limit
 *   E_STOPPED       : raw buffer object was reseted
 *   E_DELETED       : raw buffer object was deleted
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
int raw_send( raw_t *raw, const void *data, size_t size ) { return raw_sendFor(raw, data, size, INFINITE); }

/******************************************************************************
 *
 * Name              : raw_push
 * ISR alias         : raw_pushISR
 *
 * Description       : try to transfer data to the raw buffer object,
 *                     remove the oldest data if the raw buffer object is full
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *
 * Return
 *   E_SUCCESS       : data was successfully transferred to the raw buffer object
 *   E_FAILURE       : size of the data is out of the limit
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

int raw_push( raw_t *raw, const void *data, size_t size );

__STATIC_INLINE
int raw_pushISR( raw_t *raw, const void *data, size_t size ) { return raw_push(raw, data, size); }

/******************************************************************************
 *
 * Name              : raw_count
 * ISR alias         : raw_countISR
 *
 * Description       : return the amount of data contained in the raw buffer
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *
 * Return            : amount of data contained in the raw buffer
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

size_t raw_count( raw_t *raw );

__STATIC_INLINE
size_t raw_countISR( raw_t *raw ) { return raw_count(raw); }

/******************************************************************************
 *
 * Name              : raw_space
 * ISR alias         : raw_spaceISR
 *
 * Description       : return the amount of free space in the raw buffer
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *
 * Return            : amount of free space in the raw buffer
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

size_t raw_space( raw_t *raw );

__STATIC_INLINE
size_t raw_spaceISR( raw_t *raw ) { return raw_space(raw); }

/******************************************************************************
 *
 * Name              : raw_limit
 * ISR alias         : raw_limitISR
 *
 * Description       : return the size of the raw buffer
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *
 * Return            : size of the raw buffer
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

size_t raw_limit( raw_t *raw );

__STATIC_INLINE
size_t raw_limitISR( raw_t *raw ) { return raw_limit(raw); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace stateos {

/******************************************************************************
 *
 * Class             : RawBufferT<>
 *
 * Description       : create and initialize a raw buffer object
 *
 * Constructor parameters
 *   limit           : size of a buffer (max number of stored bytes)
 *
 ******************************************************************************/

template<size_t limit_>
struct RawBufferT : public __raw
{
	constexpr
	RawBufferT(): __raw _RAW_INIT(limit_, data_) {}

	~RawBufferT() { assert(__raw::obj.queue == nullptr); }

	RawBufferT( RawBufferT&& ) = delete;
	RawBufferT( const RawBufferT& ) = delete;
	RawBufferT& operator=( RawBufferT&& ) = delete;
	RawBufferT& operator=( const RawBufferT& ) = delete;

	void   reset    ()                                                                   {        raw_reset    (this); }
	void   kill     ()                                                                   {        raw_kill     (this); }
	void   destroy  ()                                                                   {        raw_destroy  (this); }
	int    take     (       void *_data, size_t _size, size_t *_read = nullptr )         { return raw_take     (this, _data, _size, _read); }
	int    tryWait  (       void *_data, size_t _size, size_t *_read = nullptr )         { return raw_tryWait  (this, _data, _size, _read); }
	int    takeISR  (       void *_data, size_t _size, size_t *_read = nullptr )         { return raw_takeISR  (this, _data, _size, _read); }
	template<typename T>
	int    waitFor  (       void *_data, size_t _size, size_t *_read,  const T& _delay ) { return raw_waitFor  (this, _data, _size, _read, _delay); }
	template<typename T>
	int    waitUntil(       void *_data, size_t _size, size_t *_read,  const T& _time )  { return raw_waitUntil(this, _data, _size, _read, _time); }
	int    wait     (       void *_data, size_t _size, size_t *_read = nullptr )         { return raw_wait     (this, _data, _size, _read); }
	int    give     ( const void *_data, size_t _size )                                  { return raw_give     (this, _data, _size); }
	int    giveISR  ( const void *_data, size_t _size )                                  { return raw_giveISR  (this, _data, _size); }
	template<typename T>
	int    sendFor  ( const void *_data, size_t _size, const T& _delay )                 { return raw_sendFor  (this, _data, _size, _delay); }
	template<typename T>
	int    sendUntil( const void *_data, size_t _size, const T& _time )                  { return raw_sendUntil(this, _data, _size, _time); }
	int    send     ( const void *_data, size_t _size )                                  { return raw_send     (this, _data, _size); }
	int    push     ( const void *_data, size_t _size )                                  { return raw_push     (this, _data, _size); }
	int    pushISR  ( const void *_data, size_t _size )                                  { return raw_pushISR  (this, _data, _size); }
	size_t count    ()                                                                   { return raw_count    (this); }
	size_t countISR ()                                                                   { return raw_countISR (this); }
	size_t space    ()                                                                   { return raw_space    (this); }
	size_t spaceISR ()                                                                   { return raw_spaceISR (this); }
	size_t limit    ()                                                                   { return raw_limit    (this); }
	size_t limitISR ()                                                                   { return raw_limitISR (this); }

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<RawBufferT<limit_>>;
#else
	using Ptr = RawBufferT<limit_> *;
#endif

/******************************************************************************
 *
 * Name              : RawBufferT<>::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters
 *   limit           : size of a buffer (max number of stored bytes)
 *
 * Return            : std::unique_pointer / pointer to RawBufferT<> object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create()
	{
		auto raw = new RawBufferT<limit_>();
		if (raw != nullptr)
			raw->__raw::obj.res = raw;
		return Ptr(raw);
	}

	private:
	char data_[limit_];
};

/******************************************************************************
 *
 * Class             : RawBufferTT<>
 *
 * Description       : create and initialize a raw buffer object
 *
 * Constructor parameters
 *   limit           : size of a buffer (max number of stored objects)
 *   C               : class of an object
 *
 ******************************************************************************/

template<unsigned limit_, class C>
struct RawBufferTT : public RawBufferT<limit_*sizeof(C)>
{
	constexpr
	RawBufferTT(): RawBufferT<limit_*sizeof(C)>() {}

	int take     (       C *_data )                  { return raw_take     (this, _data, sizeof(C), nullptr); }
	int tryWait  (       C *_data )                  { return raw_tryWait  (this, _data, sizeof(C), nullptr); }
	int takeISR  (       C *_data )                  { return raw_takeISR  (this, _data, sizeof(C), nullptr); }
	template<typename T>                               
	int waitFor  (       C *_data, const T& _delay ) { return raw_waitFor  (this, _data, sizeof(C), nullptr, _delay); }
	template<typename T>                               
	int waitUntil(       C *_data, const T& _time )  { return raw_waitUntil(this, _data, sizeof(C), nullptr, _time); }
	int wait     (       C *_data )                  { return raw_wait     (this, _data, sizeof(C), nullptr); }
	int give     ( const C *_data )                  { return raw_give     (this, _data, sizeof(C)); }
	int giveISR  ( const C *_data )                  { return raw_giveISR  (this, _data, sizeof(C)); }
	template<typename T>                               
	int sendFor  ( const C *_data, const T& _delay ) { return raw_sendFor  (this, _data, sizeof(C), _delay); }
	template<typename T>                               
	int sendUntil( const C *_data, const T& _time )  { return raw_sendUntil(this, _data, sizeof(C), _time); }
	int send     ( const C *_data )                  { return raw_send     (this, _data, sizeof(C)); }
	int push     ( const C *_data )                  { return raw_push     (this, _data, sizeof(C)); }
	int pushISR  ( const C *_data )                  { return raw_pushISR  (this, _data, sizeof(C)); }

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<RawBufferTT<limit_, C>>;
#else
	using Ptr = RawBufferTT<limit_, C> *;
#endif

/******************************************************************************
 *
 * Name              : RawBufferTT<>::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters
 *   limit           : size of a buffer (max number of stored objects)
 *   C               : class of an object
 *
 * Return            : std::unique_pointer / pointer to RawBufferTT<> object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create()
	{
		auto raw = new RawBufferTT<limit_, C>();
		if (raw != nullptr)
			raw->__raw::obj.res = raw;
		return Ptr(raw);
	}

};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__STATEOS_RAW_H
