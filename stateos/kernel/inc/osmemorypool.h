/******************************************************************************

    @file    StateOS: osmemorypool.h
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

#ifndef __STATEOS_MEM_H
#define __STATEOS_MEM_H

#include "oskernel.h"
#include "osclock.h"
#include "oslist.h"

/* -------------------------------------------------------------------------- */

#define MEM_SIZE( size ) \
    ALIGNED_SIZE( size, sizeof(que_t) )

/******************************************************************************
 *
 * Name              : memory pool
 *
 ******************************************************************************/

typedef struct __mem mem_t;

struct __mem
{
	lst_t    lst;   // memory pool list

	unsigned limit; // size of a memory pool (depth of memory pool buffer)
	unsigned size;  // size of memory object (in sizeof(que_t) units)
	que_t  * data;  // pointer to memory pool buffer
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

/******************************************************************************
 *
 * Name              : OS_MEM_BUFFER
 *
 * Description       : define memory pool data buffer
 *
 * Parameters
 *   buf             : name of the buffer (passed to the init function)
 *   limit           : size of a buffer (max number of objects)
 *   size            : size of memory object (in bytes)
 *
 ******************************************************************************/

#define             OS_MEM_BUFFER( buf, limit, size ) \
                       que_t buf[limit * (1 + MEM_SIZE(size))]

/******************************************************************************
 *
 * Name              : OS_MEM
 * Static alias      : static_MEM
 *
 * Description       : define and initialize a memory pool object
 *
 * Parameters
 *   mem             : name of a pointer to memory pool object
 *   limit           : size of a buffer (max number of objects)
 *   size            : size of memory object (in bytes)
 *
 ******************************************************************************/

#define             OS_MEM( mem, limit, size )                        \
                static que_t mem##__buf[limit * (1 + MEM_SIZE(size))]; \
                       mem_t mem[] = { _MEM_INIT( limit, MEM_SIZE(size), mem##__buf ) }

#define         static_MEM( mem, limit, size )                        \
                static que_t mem##__buf[limit * (1 + MEM_SIZE(size))]; \
                static mem_t mem[] = { _MEM_INIT( limit, MEM_SIZE(size), mem##__buf ) }

/******************************************************************************
 *
 * Name              : MEM_INIT
 *
 * Description       : create and initialize a memory pool object
 *
 * Parameters
 *   limit           : size of a buffer (max number of objects)
 *   size            : size of memory object (in bytes)
 *
 * Return            : memory pool object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                MEM_INIT( limit, size ) \
                      _MEM_INIT( limit, MEM_SIZE(size), _MEM_DATA(limit, MEM_SIZE(size)) )
#endif

/******************************************************************************
 *
 * Name              : MEM_CREATE
 * Alias             : MEM_NEW
 *
 * Description       : create and initialize a memory pool object
 *
 * Parameters
 *   limit           : size of a buffer (max number of objects)
 *   size            : size of memory object (in bytes)
 *
 * Return            : memory pool object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                MEM_CREATE( limit, size ) \
                     { MEM_INIT  ( limit, size ) }
#define                MEM_NEW \
                       MEM_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : mem_init
 *
 * Description       : initialize a memory pool object
 *
 * Parameters
 *   mem             : pointer to memory pool object
 *   size            : size of memory object (in bytes)
 *   data            : memory pool data buffer
 *   bufsize         : size of the data buffer (in bytes)
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void mem_init( mem_t *mem, size_t size, que_t *data, size_t bufsize );

/******************************************************************************
 *
 * Name              : mem_create
 * Alias             : mem_new
 *
 * Description       : create and initialize a new memory pool object
 *
 * Parameters
 *   limit           : size of a buffer (max number of objects)
 *   size            : size of memory object (in bytes)
 *
 * Return            : pointer to memory pool object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

mem_t *mem_create( unsigned limit, size_t size );

__STATIC_INLINE
mem_t *mem_new( unsigned limit, size_t size ) { return mem_create(limit, size); }

/******************************************************************************
 *
 * Name              : mem_reset
 * Alias             : mem_kill
 *
 * Description       : reset the memory pool object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   mem             : pointer to memory pool object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void mem_reset( mem_t *mem );

__STATIC_INLINE
void mem_kill( mem_t *mem ) { mem_reset(mem); }

/******************************************************************************
 *
 * Name              : mem_destroy
 * Alias             : mem_delete
 *
 * Description       : reset the memory pool object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * Parameters
 *   mem             : pointer to memory pool object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void mem_destroy( mem_t *mem );

__STATIC_INLINE
void mem_delete( mem_t *mem ) { mem_destroy(mem); }

/******************************************************************************
 *
 * Name              : mem_take
 * Alias             : mem_tryWait
 * ISR alias         : mem_takeISR
 *
 * Description       : try to get memory object from the memory pool object,
 *                     don't wait if the memory pool object is empty
 *
 * Parameters
 *   mem             : pointer to memory pool object
 *   data            : pointer to store the pointer to the memory object
 *
 * Return
 *   E_SUCCESS       : pointer to memory object was successfully transferred to the data pointer
 *   E_TIMEOUT       : memory pool object is empty
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

int mem_take( mem_t *mem, void **data );

__STATIC_INLINE
int mem_tryWait( mem_t *mem, void **data ) { return mem_take(mem, data); }

__STATIC_INLINE
int mem_takeISR( mem_t *mem, void **data ) { return mem_take(mem, data); }

/******************************************************************************
 *
 * Name              : mem_waitFor
 *
 * Description       : try to get memory object from the memory pool object,
 *                     wait for given duration of time while the memory pool object is empty
 *
 * Parameters
 *   mem             : pointer to memory pool object
 *   data            : pointer to store the pointer to the memory object
 *   delay           : duration of time (maximum number of ticks to wait while the memory pool object is empty)
 *                     IMMEDIATE: don't wait if the memory pool object is empty
 *                     INFINITE:  wait indefinitely while the memory pool object is empty
 *
 * Return
 *   E_SUCCESS       : pointer to memory object was successfully transferred to the data pointer
 *   E_STOPPED       : memory pool object was reseted before the specified timeout expired
 *   E_DELETED       : memory pool object was deleted before the specified timeout expired
 *   E_TIMEOUT       : memory pool object is empty and was not received data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int mem_waitFor( mem_t *mem, void **data, cnt_t delay );

/******************************************************************************
 *
 * Name              : mem_waitUntil
 *
 * Description       : try to get memory object from the memory pool object,
 *                     wait until given timepoint while the memory pool object is empty
 *
 * Parameters
 *   mem             : pointer to memory pool object
 *   data            : pointer to store the pointer to the memory object
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : pointer to memory object was successfully transferred to the data pointer
 *   E_STOPPED       : memory pool object was reseted before the specified timeout expired
 *   E_DELETED       : memory pool object was deleted before the specified timeout expired
 *   E_TIMEOUT       : memory pool object is empty and was not received data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int mem_waitUntil( mem_t *mem, void **data, cnt_t time );

/******************************************************************************
 *
 * Name              : mem_wait
 *
 * Description       : try to get memory object from the memory pool object,
 *                     wait indefinitely while the memory pool object is empty
 *
 * Parameters
 *   mem             : pointer to memory pool object
 *   data            : pointer to store the pointer to the memory object
 *
 * Return
 *   E_SUCCESS       : pointer to memory object was successfully transferred to the data pointer
 *   E_STOPPED       : memory pool object was reseted
 *   E_DELETED       : memory pool object was deleted
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
int mem_wait( mem_t *mem, void **data ) { return mem_waitFor(mem, data, INFINITE); }

/******************************************************************************
 *
 * Name              : mem_give
 * ISR alias         : mem_giveISR
 *
 * Description       : transfer memory object to the memory pool object,
 *
 * Parameters
 *   mem             : pointer to memory pool object
 *   data            : pointer to memory object
 *
 * Return            : none
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

void mem_give( mem_t *mem, void *data );

__STATIC_INLINE
void mem_giveISR( mem_t *mem, void *data ) { mem_give(mem, data); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace stateos {

/******************************************************************************
 *
 * Class             : MemoryPoolT<>
 *
 * Description       : create and initialize a memory pool object
 *
 * Constructor parameters
 *   limit           : size of a buffer (max number of objects)
 *   size            : size of memory object (in bytes)
 *
 ******************************************************************************/

template<unsigned limit_, size_t size_>
struct MemoryPoolT : public __mem
{
	MemoryPoolT(): __mem _MEM_INIT(limit_, MEM_SIZE(size_), data_) {}

	~MemoryPoolT() { assert(__mem::lst.obj.queue == nullptr); }

	MemoryPoolT( MemoryPoolT&& ) = default;
	MemoryPoolT( const MemoryPoolT& ) = delete;
	MemoryPoolT& operator=( MemoryPoolT&& ) = delete;
	MemoryPoolT& operator=( const MemoryPoolT& ) = delete;

	void reset    ()                                {        mem_reset    (this); }
	void kill     ()                                {        mem_kill     (this); }
	void destroy  ()                                {        mem_destroy  (this); }
	int  take     ( void **_data )                  { return mem_take     (this, _data); }
	int  tryWait  ( void **_data )                  { return mem_tryWait  (this, _data); }
	int  takeISR  ( void **_data )                  { return mem_takeISR  (this, _data); }
	template<typename T>
	int  waitFor  ( void **_data, const T& _delay ) { return mem_waitFor  (this, _data, Clock::count(_delay)); }
	template<typename T>
	int  waitUntil( void **_data, const T& _time )  { return mem_waitUntil(this, _data, Clock::until(_time)); }
	int  wait     ( void **_data )                  { return mem_wait     (this, _data); }
	void give     ( void  *_data )                  {        mem_give     (this, _data); }
	void giveISR  ( void  *_data )                  {        mem_giveISR  (this, _data); }

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<MemoryPoolT<limit_, size_>>;
#else
	using Ptr = MemoryPoolT<limit_, size_> *;
#endif

/******************************************************************************
 *
 * Name              : MemoryPoolT<>::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters
 *   limit           : size of a buffer (max number of objects)
 *   size            : size of memory object (in bytes)
 *
 * Return            : std::unique_pointer / pointer to MemoryPoolT<> object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create()
	{
		auto mem = new (std::nothrow) MemoryPoolT<limit_, size_>();
		if (mem != nullptr)
			mem->__mem::lst.obj.res = mem;
		return Ptr(mem);
	}

	private:
	que_t data_[limit_ * (1 + MEM_SIZE(size_))];
};

/******************************************************************************
 *
 * Class             : MemoryPoolTT<>
 *
 * Description       : create and initialize a memory pool object
 *
 * Constructor parameters
 *   limit           : size of a buffer (max number of objects)
 *   C               : class of a memory object
 *
 ******************************************************************************/

template<unsigned limit_, class C>
struct MemoryPoolTT : public MemoryPoolT<limit_, sizeof(C)>
{
	MemoryPoolTT(): MemoryPoolT<limit_, sizeof(C)>() {}

	int  take     ( C **_data )                  { return mem_take     (this, reinterpret_cast<void **>(_data)); }
	int  tryWait  ( C **_data )                  { return mem_tryWait  (this, reinterpret_cast<void **>(_data)); }
	int  takeISR  ( C **_data )                  { return mem_takeISR  (this, reinterpret_cast<void **>(_data)); }
	template<typename T>
	int  waitFor  ( C **_data, const T& _delay ) { return mem_waitFor  (this, reinterpret_cast<void **>(_data), Clock::count(_delay)); }
	template<typename T>
	int  waitUntil( C **_data, const T& _time )  { return mem_waitUntil(this, reinterpret_cast<void **>(_data), Clock::until(_time)); }
	int  wait     ( C **_data )                  { return mem_wait     (this, reinterpret_cast<void **>(_data)); }

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<MemoryPoolTT<limit_, C>>;
#else
	using Ptr = MemoryPoolTT<limit_, C> *;
#endif

/******************************************************************************
 *
 * Name              : MemoryPoolTT<>::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters
 *   limit           : size of a buffer (max number of objects)
 *   C               : class of a memory object
 *
 * Return            : std::unique_pointer / pointer to MemoryPoolTT<> object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create()
	{
		auto mem = new (std::nothrow) MemoryPoolTT<limit_, C>();
		if (mem != nullptr)
			mem->__mem::lst.obj.res = mem;
		return Ptr(mem);
	}

};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__STATEOS_MEM_H
