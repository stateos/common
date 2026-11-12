/******************************************************************************

    @file    StateOS: osrawbuffer.h
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

#ifndef __STATEOS_RAW_H
#define __STATEOS_RAW_H

#include "oskernel.h"
#include "osclock.h"

/**
 * @defgroup rawbuffer Raw buffer
 * @brief    raw buffers (streams of bytes)
 * @{
 */

/**
 * @brief   raw buffer
 */

typedef struct __raw raw_t;

struct __raw
{
	obj_t    obj;   ///< object header

	size_t   count; ///< size of used memory in the raw buffer (in bytes)
	size_t   limit; ///< size of the raw buffer (in bytes)

	size_t   head;  ///< first element to read from data buffer
	size_t   tail;  ///< first element to write into data buffer
	char *   data;  ///< data buffer
};

typedef struct __raw raw_id [];

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

/**
 * @brief   define raw data buffer
 *
 * @param   buf        name of the buffer (passed to the init function)
 * @param   limit      size of a buffer (max number of stored bytes / objects)
 * @param   ...        size: (optional) size of the object (in bytes); default: 1
 */

#define             OS_RAW_BUFFER( buf, limit, ... ) \
                       char buf[_VA_RAW(limit, __VA_ARGS__)]

/**
 * @brief   define and initialize a raw buffer object
 *
 * @param   raw        name of a pointer to raw buffer object
 * @param   limit      size of a buffer (max number of stored bytes / objects)
 * @param   ...        size: (optional) size of the object (in bytes); default: 1
 */

#define             OS_RAW( raw, limit, ... )                       \
                static char raw##__buf[_VA_RAW(limit, __VA_ARGS__)]; \
                       raw_t raw[] = { _RAW_INIT( _VA_RAW(limit, __VA_ARGS__), raw##__buf ) }

/** static alias of OS_RAW() */
#define         static_RAW( raw, limit, ... )                       \
                static char raw##__buf[_VA_RAW(limit, __VA_ARGS__)]; \
                static raw_t raw[] = { _RAW_INIT( _VA_RAW(limit, __VA_ARGS__), raw##__buf ) }

/**
 * @brief   create and initialize a raw buffer object
 *
 * @param   limit      size of a buffer (max number of stored bytes / objects)
 * @param   ...        type: (optional) size of the object (in bytes); default: 1
 *
 * @return  raw buffer object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                RAW_INIT( limit, ... ) \
                      _RAW_INIT( _VA_RAW(limit, __VA_ARGS__), _RAW_DATA( _VA_RAW(limit, __VA_ARGS__) ) )
#endif

/**
 * @brief   create and initialize a raw buffer object
 *
 * @param   limit      size of a buffer (max number of stored bytes / objects)
 * @param   ...        type: (optional) size of the object (in bytes); default: 1
 *
 * @return  raw buffer object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                RAW_CREATE( limit, ... ) \
                     { RAW_INIT  ( _VA_RAW(limit, __VA_ARGS__) ) }
/** alias of RAW_CREATE() */
#define                RAW_NEW \
                       RAW_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize a raw buffer object
 *
 * @param   raw        pointer to raw buffer object
 * @param   data       raw buffer data
 * @param   bufsize    size of the data buffer (in bytes)
 *
 * @note    use only in thread mode
 */

void raw_init( raw_t *raw, void *data, size_t bufsize );

/**
 * @brief   create and initialize a new raw buffer object
 *
 * @param   limit      size of a buffer (max number of stored bytes)
 *
 * @return  pointer to raw buffer object
 * @retval  NULL       object not created (not enough free memory)
 *
 * @note    use only in thread mode
 */

raw_t *raw_create( size_t limit );

/** alias of raw_create() */
__STATIC_INLINE
raw_t *raw_new( size_t limit ) { return raw_create(limit); }

/**
 * @brief   reset the raw buffer object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * @param   raw        pointer to raw buffer object
 *
 * @note    use only in thread mode
 */

void raw_reset( raw_t *raw );

/** alias of raw_reset() */
__STATIC_INLINE
void raw_kill( raw_t *raw ) { raw_reset(raw); }

/**
 * @brief   reset the raw buffer object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * @param   raw        pointer to raw buffer object
 *
 * @note    use only in thread mode
 */

void raw_destroy( raw_t *raw );

/** alias of raw_destroy() */
__STATIC_INLINE
void raw_delete( raw_t *raw ) { raw_destroy(raw); }

/**
 * @brief   try to transfer data from the raw buffer object,
 *          don't wait if the raw buffer object is empty
 *
 * @param   raw        pointer to raw buffer object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 * @param   read       pointer to the variable getting number of read bytes
 *
 * @retval  E_SUCCESS  variable 'read' contains the number of bytes read from the raw buffer
 * @retval  E_TIMEOUT  raw buffer object is empty, try again
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

int raw_take( raw_t *raw, void *data, size_t size, size_t *read );

/** alias of raw_take() */
__STATIC_INLINE
int raw_tryWait( raw_t *raw, void *data, size_t size, size_t *read ) { return raw_take(raw, data, size, read); }

/** ISR alias of raw_take() */
__STATIC_INLINE
int raw_takeISR( raw_t *raw, void *data, size_t size, size_t *read ) { return raw_take(raw, data, size, read); }

/**
 * @brief   try to transfer data from the raw buffer object,
 *          wait for given duration of time while the raw buffer object is empty
 *
 * @param   raw        pointer to raw buffer object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 * @param   read       pointer to the variable getting number of read bytes
 * @param   delay      duration of time (maximum number of ticks to wait while the raw buffer object is empty)
 *                     - IMMEDIATE: don't wait if the raw buffer object is empty
 *                     - INFINITE:  wait indefinitely while the raw buffer object is empty
 *
 * @retval  E_SUCCESS  variable 'read' contains the number of bytes read from the raw buffer
 * @retval  E_STOPPED  raw buffer object was reseted before the specified timeout expired
 * @retval  E_DELETED  raw buffer object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  raw buffer object is empty and was not received data before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int raw_waitFor( raw_t *raw, void *data, size_t size, size_t *read, cnt_t delay );

/**
 * @brief   try to transfer data from the raw buffer object,
 *          wait until given timepoint while the raw buffer object is empty
 *
 * @param   raw        pointer to raw buffer object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 * @param   read       pointer to the variable getting number of read bytes
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  variable 'read' contains the number of bytes read from the raw buffer
 * @retval  E_STOPPED  raw buffer object was reseted before the specified timeout expired
 * @retval  E_DELETED  raw buffer object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  raw buffer object is empty and was not received data before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int raw_waitUntil( raw_t *raw, void *data, size_t size, size_t *read, cnt_t time );

/**
 * @brief   try to transfer data from the raw buffer object,
 *          wait indefinitely while the raw buffer object is empty
 *
 * @param   raw        pointer to raw buffer object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 * @param   read       pointer to the variable getting number of read bytes
 *
 * @retval  E_SUCCESS  variable 'read' contains the number of bytes read from the raw buffer
 * @retval  E_STOPPED  raw buffer object was reseted
 * @retval  E_DELETED  raw buffer object was deleted
 *
 * @note    use only in thread mode
 */

__STATIC_INLINE
int raw_wait( raw_t *raw, void *data, size_t size, size_t *read ) { return raw_waitFor(raw, data, size, read, INFINITE); }

/**
 * @brief   try to transfer data to the raw buffer object,
 *          don't wait if the raw buffer object is full
 *
 * @param   raw        pointer to raw buffer object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 *
 * @retval  E_SUCCESS  data was successfully transferred to the raw buffer object
 * @retval  E_FAILURE  size of the data is out of the limit
 * @retval  E_TIMEOUT  not enough space in the raw buffer, try again
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

int raw_give( raw_t *raw, const void *data, size_t size );

/** ISR alias of raw_give() */
__STATIC_INLINE
int raw_giveISR( raw_t *raw, const void *data, size_t size ) { return raw_give(raw, data, size); }

/**
 * @brief   try to transfer data to the raw buffer object,
 *          wait for given duration of time while the raw buffer object is full
 *
 * @param   raw        pointer to raw buffer object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 * @param   delay      duration of time (maximum number of ticks to wait while the raw buffer object is full)
 *                     - IMMEDIATE: don't wait if the raw buffer object is full
 *                     - INFINITE:  wait indefinitely while the raw buffer object is full
 *
 * @retval  E_SUCCESS  data was successfully transferred to the raw buffer object
 * @retval  E_FAILURE  size of the data is out of the limit
 * @retval  E_STOPPED  raw buffer object was reseted before the specified timeout expired
 * @retval  E_DELETED  raw buffer object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  raw buffer object is full and was not issued data before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int raw_sendFor( raw_t *raw, const void *data, size_t size, cnt_t delay );

/**
 * @brief   try to transfer data to the raw buffer object,
 *          wait until given timepoint while the raw buffer object is full
 *
 * @param   raw        pointer to raw buffer object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  data was successfully transferred to the raw buffer object
 * @retval  E_FAILURE  size of the data is out of the limit
 * @retval  E_STOPPED  raw buffer object was reseted before the specified timeout expired
 * @retval  E_DELETED  raw buffer object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  raw buffer object is full and was not issued data before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int raw_sendUntil( raw_t *raw, const void *data, size_t size, cnt_t time );

/**
 * @brief   try to transfer data to the raw buffer object,
 *          wait indefinitely while the raw buffer object is full
 *
 * @param   raw        pointer to raw buffer object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 *
 * @retval  E_SUCCESS  data was successfully transferred to the raw buffer object
 * @retval  E_FAILURE  size of the data is out of the limit
 * @retval  E_STOPPED  raw buffer object was reseted
 * @retval  E_DELETED  raw buffer object was deleted
 *
 * @note    use only in thread mode
 */

__STATIC_INLINE
int raw_send( raw_t *raw, const void *data, size_t size ) { return raw_sendFor(raw, data, size, INFINITE); }

/**
 * @brief   try to transfer data to the raw buffer object,
 *          remove the oldest data if the raw buffer object is full
 *
 * @param   raw        pointer to raw buffer object
 * @param   data       pointer to the buffer
 * @param   size       size of the buffer
 *
 * @retval  E_SUCCESS  data was successfully transferred to the raw buffer object
 * @retval  E_FAILURE  size of the data is out of the limit
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

int raw_push( raw_t *raw, const void *data, size_t size );

/** ISR alias of raw_push() */
__STATIC_INLINE
int raw_pushISR( raw_t *raw, const void *data, size_t size ) { return raw_push(raw, data, size); }

/**
 * @brief   return the amount of data contained in the raw buffer
 *
 * @param   raw        pointer to raw buffer object
 *
 * @return  amount of data contained in the raw buffer
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

size_t raw_count( raw_t *raw );

/** ISR alias of raw_count() */
__STATIC_INLINE
size_t raw_countISR( raw_t *raw ) { return raw_count(raw); }

/**
 * @brief   return the amount of free space in the raw buffer
 *
 * @param   raw        pointer to raw buffer object
 *
 * @return  amount of free space in the raw buffer
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

size_t raw_space( raw_t *raw );

/** ISR alias of raw_space() */
__STATIC_INLINE
size_t raw_spaceISR( raw_t *raw ) { return raw_space(raw); }

/**
 * @brief   return the size of the raw buffer
 *
 * @param   raw        pointer to raw buffer object
 *
 * @return  size of the raw buffer
 *
 * @note    can be used in both thread and handler mode (for blockable interrupts)
 * @note    use ISR alias in blockable interrupt handlers
 */

size_t raw_limit( raw_t *raw );

/** ISR alias of raw_limit() */
__STATIC_INLINE
size_t raw_limitISR( raw_t *raw ) { return raw_limit(raw); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(__STATEOS_STDCXX)
namespace stateos {

/**
 * @brief   create and initialize a raw buffer object
 *
 * @par     Constructor parameters
 *          - limit: size of a buffer (max number of stored bytes)
 */

template<size_t limit_>
struct RawBufferT : public __raw
{
	/// constructor, see the constructor parameters of the class
	constexpr
	RawBufferT(): __raw _RAW_INIT(limit_, data_) {}

	~RawBufferT() { assert(__raw::obj.queue == nullptr); }

	/// move constructor
	RawBufferT( RawBufferT&& _src ): __raw _RAW_INIT(limit_, data_)
	{
		assert(_src.__raw::count == 0 && _src.obj.queue == nullptr);
		(void) _src;
	}

	RawBufferT( const RawBufferT& ) = delete;
	RawBufferT& operator=( RawBufferT&& ) = delete;
	RawBufferT& operator=( const RawBufferT& ) = delete;

	void   reset    ()                                                                   {        raw_reset    (this); }                                            ///< raw_reset()
	void   kill     ()                                                                   {        raw_kill     (this); }                                            ///< raw_kill()
	void   destroy  ()                                                                   {        raw_destroy  (this); }                                            ///< raw_destroy()
	int    take     (       void *_data, size_t _size, size_t *_read = nullptr )         { return raw_take     (this, _data, _size, _read); }                       ///< raw_take()
	int    tryWait  (       void *_data, size_t _size, size_t *_read = nullptr )         { return raw_tryWait  (this, _data, _size, _read); }                       ///< raw_tryWait()
	int    takeISR  (       void *_data, size_t _size, size_t *_read = nullptr )         { return raw_takeISR  (this, _data, _size, _read); }                       ///< raw_takeISR()
	template<typename T>
	int    waitFor  (       void *_data, size_t _size, size_t *_read,  const T& _delay ) { return raw_waitFor  (this, _data, _size, _read, Clock::count(_delay)); } ///< raw_waitFor()
	template<typename T>
	int    waitUntil(       void *_data, size_t _size, size_t *_read,  const T& _time )  { return raw_waitUntil(this, _data, _size, _read, Clock::until(_time)); }  ///< raw_waitUntil()
	int    wait     (       void *_data, size_t _size, size_t *_read = nullptr )         { return raw_wait     (this, _data, _size, _read); }                       ///< raw_wait()
	int    give     ( const void *_data, size_t _size )                                  { return raw_give     (this, _data, _size); }                              ///< raw_give()
	int    giveISR  ( const void *_data, size_t _size )                                  { return raw_giveISR  (this, _data, _size); }                              ///< raw_giveISR()
	template<typename T>
	int    sendFor  ( const void *_data, size_t _size, const T& _delay )                 { return raw_sendFor  (this, _data, _size, Clock::count(_delay)); }        ///< raw_sendFor()
	template<typename T>
	int    sendUntil( const void *_data, size_t _size, const T& _time )                  { return raw_sendUntil(this, _data, _size, Clock::until(_time)); }         ///< raw_sendUntil()
	int    send     ( const void *_data, size_t _size )                                  { return raw_send     (this, _data, _size); }                              ///< raw_send()
	int    push     ( const void *_data, size_t _size )                                  { return raw_push     (this, _data, _size); }                              ///< raw_push()
	int    pushISR  ( const void *_data, size_t _size )                                  { return raw_pushISR  (this, _data, _size); }                              ///< raw_pushISR()
	size_t count    ()                                                                   { return raw_count    (this); }                                            ///< raw_count()
	size_t countISR ()                                                                   { return raw_countISR (this); }                                            ///< raw_countISR()
	size_t space    ()                                                                   { return raw_space    (this); }                                            ///< raw_space()
	size_t spaceISR ()                                                                   { return raw_spaceISR (this); }                                            ///< raw_spaceISR()
	size_t limit    ()                                                                   { return raw_limit    (this); }                                            ///< raw_limit()
	size_t limitISR ()                                                                   { return raw_limitISR (this); }                                            ///< raw_limitISR()

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<RawBufferT<limit_>>; ///< result type of Create()
#else
	using Ptr = RawBufferT<limit_> *; ///< result type of Create()
#endif

/**
 * @brief   create dynamic object with manageable resources
 *
 * @par     Parameters
 *          - limit: size of a buffer (max number of stored bytes)
 *
 * @return  std::unique_pointer / pointer to RawBufferT<> object
 *
 * @note    use only in thread mode
 */

	static
	Ptr Create()
	{
		auto raw = new (std::nothrow) RawBufferT<limit_>();
		if (raw != nullptr)
			raw->__raw::obj.res = raw;
		return Ptr(raw);
	}

	private:
	char data_[limit_];
};

/**
 * @brief   create and initialize a raw buffer object
 *
 * @par     Constructor parameters
 *          - limit: size of a buffer (max number of stored objects)
 *          - C: class of an object
 */

template<unsigned limit_, class C>
struct RawBufferTT : public RawBufferT<limit_*sizeof(C)>
{
	/// constructor, see the constructor parameters of the class
	constexpr
	RawBufferTT(): RawBufferT<limit_*sizeof(C)>() {}

	int take     (       C *_data )                  { return raw_take     (this, _data, sizeof(C), nullptr); }                       ///< raw_take()
	int tryWait  (       C *_data )                  { return raw_tryWait  (this, _data, sizeof(C), nullptr); }                       ///< raw_tryWait()
	int takeISR  (       C *_data )                  { return raw_takeISR  (this, _data, sizeof(C), nullptr); }                       ///< raw_takeISR()
	template<typename T>                               
	int waitFor  (       C *_data, const T& _delay ) { return raw_waitFor  (this, _data, sizeof(C), nullptr, Clock::count(_delay)); } ///< raw_waitFor()
	template<typename T>                               
	int waitUntil(       C *_data, const T& _time )  { return raw_waitUntil(this, _data, sizeof(C), nullptr, Clock::until(_time)); }  ///< raw_waitUntil()
	int wait     (       C *_data )                  { return raw_wait     (this, _data, sizeof(C), nullptr); }                       ///< raw_wait()
	int give     ( const C *_data )                  { return raw_give     (this, _data, sizeof(C)); }                                ///< raw_give()
	int giveISR  ( const C *_data )                  { return raw_giveISR  (this, _data, sizeof(C)); }                                ///< raw_giveISR()
	template<typename T>                               
	int sendFor  ( const C *_data, const T& _delay ) { return raw_sendFor  (this, _data, sizeof(C), Clock::count(_delay)); }          ///< raw_sendFor()
	template<typename T>                               
	int sendUntil( const C *_data, const T& _time )  { return raw_sendUntil(this, _data, sizeof(C), Clock::until(_time)); }           ///< raw_sendUntil()
	int send     ( const C *_data )                  { return raw_send     (this, _data, sizeof(C)); }                                ///< raw_send()
	int push     ( const C *_data )                  { return raw_push     (this, _data, sizeof(C)); }                                ///< raw_push()
	int pushISR  ( const C *_data )                  { return raw_pushISR  (this, _data, sizeof(C)); }                                ///< raw_pushISR()

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<RawBufferTT<limit_, C>>; ///< result type of Create()
#else
	using Ptr = RawBufferTT<limit_, C> *; ///< result type of Create()
#endif

/**
 * @brief   create dynamic object with manageable resources
 *
 * @par     Parameters
 *          - limit: size of a buffer (max number of stored objects)
 *          - C: class of an object
 *
 * @return  std::unique_pointer / pointer to RawBufferTT<> object
 *
 * @note    use only in thread mode
 */

	static
	Ptr Create()
	{
		auto raw = new (std::nothrow) RawBufferTT<limit_, C>();
		if (raw != nullptr)
			raw->__raw::obj.res = raw;
		return Ptr(raw);
	}

};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

/** @} */

#endif//__STATEOS_RAW_H
