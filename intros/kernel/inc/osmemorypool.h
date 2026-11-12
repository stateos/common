/******************************************************************************

    @file    IntrOS: osmemorypool.h
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

#ifndef __INTROS_MEM_H
#define __INTROS_MEM_H

#include "oskernel.h"
#include "osclock.h"
#include "oslist.h"

/**
 * @defgroup memorypool Memory pool
 * @brief    memory pools (objects of a fixed size)
 * @{
 */

/* -------------------------------------------------------------------------- */

#define MEM_SIZE( size ) \
    ALIGNED_SIZE( size, sizeof(que_t) )

/**
 * @brief   memory pool
 */

typedef struct __mem mem_t;

struct __mem
{
	lst_t    lst;   ///< memory pool list

	unsigned limit; ///< size of a memory pool (depth of memory pool buffer)
	unsigned size;  ///< size of memory object (in sizeof(que_t) units)
	que_t  * data;  ///< pointer to memory pool buffer
};

typedef struct __mem mem_id [];

/******************************************************************************
 *
 * Name              : _MEM_INIT
 *
 * Description       : create and initialize a memory pool object
 *
 * Parameters
 *   limit           : size of a buffer (max number of objects)
 *   size            : size of memory object (in sizeof(que_t) units)
 *   data            : memory pool data buffer
 *
 * Return            : memory pool object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _MEM_INIT( _limit, _size, _data ) { _LST_INIT(), _limit, _size, _data }

/******************************************************************************
 *
 * Name              : _MEM_DATA
 *
 * Description       : create a memory pool data buffer
 *
 * Parameters
 *   limit           : size of a buffer (max number of objects)
 *   size            : size of memory object (in sizeof(que_t) units)
 *
 * Return            : memory pool data buffer
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#ifndef __cplusplus
#define               _MEM_DATA( _limit, _size ) (que_t[_limit * (1 + _size)]){ { NULL } }
#endif

/**
 * @brief   define memory pool data buffer
 *
 * @param   buf        name of the buffer (passed to the init function)
 * @param   limit      size of a buffer (max number of objects)
 * @param   size       size of memory object (in bytes)
 */

#define             OS_MEM_BUFFER( buf, limit, size ) \
                       que_t buf[limit * (1 + MEM_SIZE(size))]

/**
 * @brief   define and initialize a memory pool object
 *
 * @param   mem        name of a pointer to memory pool object
 * @param   limit      size of a buffer (max number of objects)
 * @param   size       size of memory object (in bytes)
 */

#define             OS_MEM( mem, limit, size )                        \
                static que_t mem##__buf[limit * (1 + MEM_SIZE(size))]; \
                       mem_t mem[] = { _MEM_INIT( limit, MEM_SIZE(size), mem##__buf ) }

/** static alias of OS_MEM() */
#define         static_MEM( mem, limit, size )                        \
                static que_t mem##__buf[limit * (1 + MEM_SIZE(size))]; \
                static mem_t mem[] = { _MEM_INIT( limit, MEM_SIZE(size), mem##__buf ) }

/**
 * @brief   create and initialize a memory pool object
 *
 * @param   limit      size of a buffer (max number of objects)
 * @param   size       size of memory object (in bytes)
 *
 * @return  memory pool object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                MEM_INIT( limit, size ) \
                      _MEM_INIT( limit, MEM_SIZE(size), _MEM_DATA(limit, MEM_SIZE(size)) )
#endif

/**
 * @brief   create and initialize a memory pool object
 *
 * @param   limit      size of a buffer (max number of objects)
 * @param   size       size of memory object (in bytes)
 *
 * @return  memory pool object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                MEM_CREATE( limit, size ) \
                     { MEM_INIT  ( limit, size ) }
/** alias of MEM_CREATE() */
#define                MEM_NEW \
                       MEM_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize a memory pool object
 *
 * @param   mem        pointer to memory pool object
 * @param   size       size of memory object (in bytes)
 * @param   data       memory pool data buffer
 * @param   bufsize    size of the data buffer (in bytes)
 */

void mem_init( mem_t *mem, size_t size, que_t *data, size_t bufsize );

/**
 * @brief   reset the memory pool object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * @param   mem        pointer to memory pool object
 *
 * @note    use only in thread mode
 */

void mem_reset( mem_t *mem );

/** alias of mem_reset() */
__STATIC_INLINE
void mem_kill( mem_t *mem ) { mem_reset(mem); }

/**
 * @brief   try to get memory object from the memory pool object,
 *          don't wait if the memory pool object is empty
 *
 * @param   mem        pointer to memory pool object
 * @param   data       pointer to store the pointer to the memory object
 *
 * @retval  E_SUCCESS  pointer to memory object was successfully transferred to the data pointer
 * @retval  E_TIMEOUT  memory pool object is empty
 */

int mem_take( mem_t *mem, void **data );

/** alias of mem_take() */
__STATIC_INLINE
int mem_tryWait( mem_t *mem, void **data ) { return mem_take(mem, data); }

/**
 * @brief   try to get memory object from the memory pool object,
 *          wait for given duration of time while the memory pool object is empty
 *
 * @param   mem        pointer to memory pool object
 * @param   data       pointer to store the pointer to the memory object
 * @param   delay      duration of time (maximum number of ticks to wait while the memory pool object is empty)
 *                     - IMMEDIATE: don't wait if the memory pool object is empty
 *                     - INFINITE:  wait indefinitely while the memory pool object is empty
 *
 * @retval  E_SUCCESS  pointer to memory object was successfully transferred to the data pointer
 * @retval  E_STOPPED  memory pool object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  memory pool object is empty and was not received data before the specified timeout expired
 */

int mem_waitFor( mem_t *mem, void **data, cnt_t delay );

/**
 * @brief   try to get memory object from the memory pool object,
 *          wait until given timepoint while the memory pool object is empty
 *
 * @param   mem        pointer to memory pool object
 * @param   data       pointer to store the pointer to the memory object
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  pointer to memory object was successfully transferred to the data pointer
 * @retval  E_STOPPED  memory pool object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  memory pool object is empty and was not received data before the specified timeout expired
 */

int mem_waitUntil( mem_t *mem, void **data, cnt_t time );

/**
 * @brief   try to get memory object from the memory pool object,
 *          wait indefinitely while the memory pool object is empty
 *
 * @param   mem        pointer to memory pool object
 * @param   data       pointer to store the pointer to the memory object
 *
 * @retval  E_SUCCESS  pointer to memory object was successfully transferred to the data pointer
 * @retval  E_STOPPED  memory pool object was reseted
 */

__STATIC_INLINE
int mem_wait( mem_t *mem, void **data ) { return mem_waitFor(mem, data, INFINITE); }

/**
 * @brief   transfer memory object to the memory pool object,
 *
 * @param   mem        pointer to memory pool object
 * @param   data       pointer to memory object
 */

void mem_give( mem_t *mem, void *data );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus)
namespace intros {

/**
 * @brief   create and initialize a memory pool object
 *
 * @par     Constructor parameters
 *          - limit: size of a buffer (max number of objects)
 *          - size: size of memory object (in bytes)
 */

template<unsigned limit_, size_t size_>
struct MemoryPoolT : public __mem
{
	/// constructor, see the constructor parameters of the class
	MemoryPoolT(): __mem _MEM_INIT(limit_, MEM_SIZE(size_), data_) {}

	~MemoryPoolT() { assert(__mem::lst.obj.queue == nullptr); }

	/// move constructor
	MemoryPoolT( MemoryPoolT&& _src ): __mem _MEM_INIT(limit_, MEM_SIZE(size_), data_)
	{
		assert(_src.limit == limit_ && _src.lst.obj.queue == nullptr); // the pool has not been used (the buffer is bound at the first use)
		(void) _src;
	}

	MemoryPoolT( const MemoryPoolT& ) = delete;
	MemoryPoolT& operator=( MemoryPoolT&& ) = delete;
	MemoryPoolT& operator=( const MemoryPoolT& ) = delete;

	void reset    ()                                {        mem_reset    (this); }                              ///< mem_reset()
	void kill     ()                                {        mem_kill     (this); }                              ///< mem_kill()
	int  take     ( void **_data )                  { return mem_take     (this, _data); }                       ///< mem_take()
	int  tryWait  ( void **_data )                  { return mem_tryWait  (this, _data); }                       ///< mem_tryWait()
	template<typename T>
	int  waitFor  ( void **_data, const T& _delay ) { return mem_waitFor  (this, _data, Clock::count(_delay)); } ///< mem_waitFor()
	template<typename T>
	int  waitUntil( void **_data, const T& _time )  { return mem_waitUntil(this, _data, Clock::until(_time)); }  ///< mem_waitUntil()
	int  wait     ( void **_data )                  { return mem_wait     (this, _data); }                       ///< mem_wait()
	void give     ( void  *_data )                  {        mem_give     (this, _data); }                       ///< mem_give()

	private:
	que_t data_[limit_ * (1 + MEM_SIZE(size_))];
};

/**
 * @brief   create and initialize a memory pool object
 *
 * @par     Constructor parameters
 *          - limit: size of a buffer (max number of objects)
 *          - C: class of a memory object
 */

template<unsigned limit_, class C>
struct MemoryPoolTT : public MemoryPoolT<limit_, sizeof(C)>
{
	/// constructor, see the constructor parameters of the class
	MemoryPoolTT(): MemoryPoolT<limit_, sizeof(C)>() {}

	int  take     ( C **_data )                  { return mem_take     (this, reinterpret_cast<void **>(_data)); }                       ///< mem_take()
	int  tryWait  ( C **_data )                  { return mem_tryWait  (this, reinterpret_cast<void **>(_data)); }                       ///< mem_tryWait()
	template<typename T>
	int  waitFor  ( C **_data, const T& _delay ) { return mem_waitFor  (this, reinterpret_cast<void **>(_data), Clock::count(_delay)); } ///< mem_waitFor()
	template<typename T>
	int  waitUntil( C **_data, const T& _time )  { return mem_waitUntil(this, reinterpret_cast<void **>(_data), Clock::until(_time)); }  ///< mem_waitUntil()
	int  wait     ( C **_data )                  { return mem_wait     (this, reinterpret_cast<void **>(_data)); }                       ///< mem_wait()
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

/** @} */

#endif//__INTROS_MEM_H
