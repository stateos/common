/******************************************************************************

    @file    IntrOS: osrawbuffer.h
    @author  Rajmund Szymanski
    @date    18.05.2021
    @brief   This file contains definitions for IntrOS.

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

#ifndef __INTROS_RAW_H
#define __INTROS_RAW_H

#include "oskernel.h"

/******************************************************************************
 *
 * Name              : raw buffer
 *
 ******************************************************************************/

typedef struct __raw raw_t, * const raw_id;

struct __raw
{
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

#define               _RAW_INIT( _limit, _data ) { 0, _limit, 0, 0, _data }

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
 ******************************************************************************/

void raw_init( raw_t *raw, void *data, size_t bufsize );

/******************************************************************************
 *
 * Name              : raw_take
 * Alias             : raw_tryWait
 *
 * Description       : try to transfer data from the raw buffer object,
 *                     don't wait if the raw buffer object is empty
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *   data            : pointer to the buffer
 *   size            : size of the buffer
 *
 * Return            : number of read bytes
 *   0               : raw buffer object is empty
 *
 ******************************************************************************/

size_t raw_take( raw_t *raw, void *data, size_t size );

__STATIC_INLINE
size_t raw_tryWait( raw_t *raw, void *data, size_t size ) { return raw_take(raw, data, size); }

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
 *
 * Return            : number of read bytes
 *
 ******************************************************************************/

size_t raw_wait( raw_t *raw, void *data, size_t size );

/******************************************************************************
 *
 * Name              : raw_give
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
 *   SUCCESS         : data was successfully transferred to the raw buffer object
 *   FAILURE         : not enough space in the raw buffer
 *
 ******************************************************************************/

unsigned raw_give( raw_t *raw, const void *data, size_t size );

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
 *   SUCCESS         : data was successfully transferred to the raw buffer object
 *   FAILURE         : size of the raw data is out of the limit
 *
 ******************************************************************************/

unsigned raw_send( raw_t *raw, const void *data, size_t size );

/******************************************************************************
 *
 * Name              : raw_push
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
 *   SUCCESS         : data was successfully transferred to the raw buffer object
 *   FAILURE         : size of the data is out of the limit
 *
 ******************************************************************************/

unsigned raw_push( raw_t *raw, const void *data, size_t size );

/******************************************************************************
 *
 * Name              : raw_count
 *
 * Description       : return the amount of data contained in the raw buffer
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *
 * Return            : amount of data contained in the raw buffer
 *
 ******************************************************************************/

size_t raw_count( raw_t *raw );

/******************************************************************************
 *
 * Name              : raw_space
 *
 * Description       : return the amount of free space in the raw buffer
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *
 * Return            : amount of free space in the raw buffer
 *
 ******************************************************************************/

size_t raw_space( raw_t *raw );

/******************************************************************************
 *
 * Name              : raw_limit
 *
 * Description       : return the size of the raw buffer
 *
 * Parameters
 *   raw             : pointer to raw buffer object
 *
 * Return            : size of the raw buffer
 *
 ******************************************************************************/

size_t raw_limit( raw_t *raw );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
namespace intros {

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

	RawBufferT( RawBufferT&& ) = default;
	RawBufferT( const RawBufferT& ) = delete;
	RawBufferT& operator=( RawBufferT&& ) = delete;
	RawBufferT& operator=( const RawBufferT& ) = delete;

	size_t   take   (       void *_data, size_t _size ) { return raw_take   (this, _data, _size); }
	size_t   tryWait(       void *_data, size_t _size ) { return raw_tryWait(this, _data, _size); }
	size_t   wait   (       void *_data, size_t _size ) { return raw_wait   (this, _data, _size); }
	unsigned give   ( const void *_data, size_t _size ) { return raw_give   (this, _data, _size); }
	unsigned send   ( const void *_data, size_t _size ) { return raw_send   (this, _data, _size); }
	unsigned push   ( const void *_data, size_t _size ) { return raw_push   (this, _data, _size); }
	size_t   count  ()                                  { return raw_count  (this); }
	size_t   space  ()                                  { return raw_space  (this); }
	size_t   limit  ()                                  { return raw_limit  (this); }

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

	unsigned take   (       C *_data ) { return raw_take   (this, _data, sizeof(C)) == 0 ? FAILURE : SUCCESS; }
	unsigned tryWait(       C *_data ) { return raw_tryWait(this, _data, sizeof(C)) == 0 ? FAILURE : SUCCESS; }
	void     wait   (       C *_data ) {        raw_wait   (this, _data, sizeof(C)); }
	unsigned give   ( const C *_data ) { return raw_give   (this, _data, sizeof(C)); }
	unsigned send   ( const C *_data ) { return raw_send   (this, _data, sizeof(C)); }
	unsigned push   ( const C *_data ) { return raw_push   (this, _data, sizeof(C)); }
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_RAW_H
