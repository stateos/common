/******************************************************************************

    @file    StateOS: osrwlock.h
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

#ifndef __STATEOS_RWL_H
#define __STATEOS_RWL_H

#include "oskernel.h"
#include "osclock.h"

/**
 * @defgroup rwlock Read/write lock
 * @brief    read/write locks
 * @{
 */

/* -------------------------------------------------------------------------- */

#define RDR_LIMIT    ( 0U-1 )

/**
 * @brief   read/write lock (one write, many read)
 */

typedef struct __rwl rwl_t;

struct __rwl
{
	obj_t    obj;   ///< object header

	tsk_t  * queue; ///< readers queue
	bool     write; ///< writer is active
	unsigned count; ///< number of active readers
};

typedef struct __rwl rwl_id [];

/******************************************************************************
 *
 * Name              : _RWL_INIT
 *
 * Description       : create and initialize a read/write lock object
 *
 * Parameters        : none
 *
 * Return            : read/write lock object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _RWL_INIT() { _OBJ_INIT(), NULL, false, 0 }

/**
 * @brief   define and initialize a read/write lock object
 *
 * @param   rwl        name of a pointer to read/write lock object
 */

#define             OS_RWL( rwl ) \
                       rwl_t rwl[] = { _RWL_INIT() }

/** static alias of OS_RWL() */
#define         static_RWL( rwl ) \
                static rwl_t rwl[] = { _RWL_INIT() }

/**
 * @brief   create and initialize a read/write lock object
 *
 * @return  read/write lock object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                RWL_INIT() \
                      _RWL_INIT()
#endif

/**
 * @brief   create and initialize a read/write lock object
 *
 * @return  read/write lock object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                RWL_CREATE() \
                     { RWL_INIT  () }
/** alias of RWL_CREATE() */
#define                RWL_NEW \
                       RWL_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize a read/write lock object
 *
 * @param   rwl        pointer to read/write lock object
 *
 * @note    use only in thread mode
 */

void rwl_init( rwl_t *rwl );

/**
 * @brief   create and initialize a new read/write lock object
 *
 * @return  pointer to read/write lock object
 * @retval  NULL       object not created (not enough free memory)
 *
 * @note    use only in thread mode
 */

rwl_t *rwl_create( void );

/** alias of rwl_create() */
__STATIC_INLINE
rwl_t *rwl_new( void ) { return rwl_create(); }

/**
 * @brief   reset the read/write lock object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * @param   rwl        pointer to read/write lock object
 *
 * @note    use only in thread mode
 */

void rwl_reset( rwl_t *rwl );

/** alias of rwl_reset() */
__STATIC_INLINE
void rwl_kill( rwl_t *rwl ) { rwl_reset(rwl); }

/**
 * @brief   reset the read/write lock object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * @param   rwl        pointer to read/write lock object
 *
 * @note    use only in thread mode
 */

void rwl_destroy( rwl_t *rwl );

/** alias of rwl_destroy() */
__STATIC_INLINE
void rwl_delete( rwl_t *rwl ) { rwl_destroy(rwl); }

/**
 * @brief   try to lock the reader,
 *          don't wait if the reader can't be locked immediately
 *
 * @param   rwl        pointer to read/write lock object
 *
 * @retval  E_SUCCESS  reader was successfully locked
 * @retval  E_TIMEOUT  reader can't be locked immediately, try again
 *
 * @note    use only in thread mode
 */

int rwl_takeRead( rwl_t *rwl );

/** alias of rwl_takeRead() */
__STATIC_INLINE
int rwl_tryLockRead( rwl_t *rwl ) { return rwl_takeRead(rwl); }

/**
 * @brief   try to lock the reader,
 *          wait for given duration of time if the reader can't be locked immediately
 *
 * @param   rwl        pointer to read/write lock object
 * @param   delay      duration of time (maximum number of ticks to wait for lock the reader)
 *                     - IMMEDIATE: don't wait if the reader can't be locked immediately
 *                     - INFINITE:  wait indefinitely until the reader has been locked
 *
 * @retval  E_SUCCESS  reader was successfully locked
 * @retval  E_STOPPED  reader was reseted before the specified timeout expired
 * @retval  E_DELETED  reader was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  reader was not locked before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int rwl_waitReadFor( rwl_t *rwl, cnt_t delay );

/**
 * @brief   try to lock the reader,
 *          wait until given timepoint if the reader can't be locked immediately
 *
 * @param   rwl        pointer to read/write lock object
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  reader was successfully locked
 * @retval  E_STOPPED  reader was reseted before the specified timeout expired
 * @retval  E_DELETED  reader was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  reader was not locked before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int rwl_waitReadUntil( rwl_t *rwl, cnt_t time );

/**
 * @brief   try to lock the reader,
 *          wait indefinitely if the reader can't be locked immediately
 *
 * @param   rwl        pointer to read/write lock object
 *
 * @retval  E_SUCCESS  reader was successfully locked
 * @retval  E_STOPPED  reader was reseted
 * @retval  E_DELETED  reader was deleted
 *
 * @note    use only in thread mode
 */

__STATIC_INLINE
int rwl_waitRead( rwl_t *rwl ) { return rwl_waitReadFor(rwl, INFINITE); }

/** alias of rwl_waitRead() */
__STATIC_INLINE
int rwl_lockRead( rwl_t *rwl ) { return rwl_waitRead(rwl); }

/**
 * @brief   unlock the reader
 *
 * @param   rwl        pointer to read/write lock object
 *
 * @note    use only in thread mode
 */

void rwl_giveRead( rwl_t *rwl );

/** alias of rwl_giveRead() */
__STATIC_INLINE
void rwl_unlockRead( rwl_t *rwl ) { rwl_giveRead(rwl); }

/**
 * @brief   try to lock the writer,
 *          don't wait if the writer can't be locked immediately
 *
 * @param   rwl        pointer to read/write lock object
 *
 * @retval  E_SUCCESS  writer was successfully locked
 * @retval  E_TIMEOUT  writer can't be locked immediately, try again
 *
 * @note    use only in thread mode
 */

int rwl_takeWrite( rwl_t *rwl );

/** alias of rwl_takeWrite() */
__STATIC_INLINE
int rwl_tryLockWrite( rwl_t *rwl ) { return rwl_takeWrite(rwl); }

/**
 * @brief   try to lock the writer,
 *          wait for given duration of time if the writer can't be locked immediately
 *
 * @param   rwl        pointer to read/write lock object
 * @param   delay      duration of time (maximum number of ticks to wait for lock the writer)
 *                     - IMMEDIATE: don't wait if the writer can't be locked immediately
 *                     - INFINITE:  wait indefinitely until the writer has been locked
 *
 * @retval  E_SUCCESS  writer was successfully locked
 * @retval  E_STOPPED  writer was reseted before the specified timeout expired
 * @retval  E_DELETED  writer was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  writer was not locked before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int rwl_waitWriteFor( rwl_t *rwl, cnt_t delay );

/**
 * @brief   try to lock the writer,
 *          wait until given timepoint if the writer can't be locked immediately
 *
 * @param   rwl        pointer to read/write lock object
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  writer was successfully locked
 * @retval  E_STOPPED  writer was reseted before the specified timeout expired
 * @retval  E_DELETED  writer was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  writer was not locked before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int rwl_waitWriteUntil( rwl_t *rwl, cnt_t time );

/**
 * @brief   try to lock the writer,
 *          wait indefinitely if the writer can't be locked immediately
 *
 * @param   rwl        pointer to read/write lock object
 *
 * @retval  E_SUCCESS  writer was successfully locked
 * @retval  E_STOPPED  writer was reseted
 * @retval  E_DELETED  writer was deleted
 *
 * @note    use only in thread mode
 */

__STATIC_INLINE
int rwl_waitWrite( rwl_t *rwl ) { return rwl_waitWriteFor(rwl, INFINITE); }

/** alias of rwl_waitWrite() */
__STATIC_INLINE
int rwl_lockWrite( rwl_t *rwl ) { return rwl_waitWrite(rwl); }

/**
 * @brief   unlock the writer
 *
 * @param   rwl        pointer to read/write lock object
 *
 * @note    use only in thread mode
 */

void rwl_giveWrite( rwl_t *rwl );

/** alias of rwl_giveWrite() */
__STATIC_INLINE
void rwl_unlockWrite( rwl_t *rwl ) { rwl_giveWrite(rwl); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(__STATEOS_STDCXX)
namespace stateos {

/**
 * @brief   create and initialize a read/write lock object
 */

struct RWLock : public __rwl
{
	/// constructor, see the constructor parameters of the class
	constexpr
	RWLock(): __rwl _RWL_INIT() {}

	~RWLock() { assert(__rwl::write == false && __rwl::count == 0); }

	/// move constructor
	RWLock( RWLock&& _src ): __rwl(_src)
	{
		assert(_src.write == false && _src.count == 0 && _src.obj.queue == nullptr && _src.queue == nullptr);
	}

	RWLock( const RWLock& ) = delete;
	RWLock& operator=( RWLock&& ) = delete;
	RWLock& operator=( const RWLock& ) = delete;

	void reset         ()                  {        rwl_reset         (this); }                       ///< rwl_reset()
	void kill          ()                  {        rwl_kill          (this); }                       ///< rwl_kill()
	void destroy       ()                  {        rwl_destroy       (this); }                       ///< rwl_destroy()
	int  takeRead      ()                  { return rwl_takeRead      (this); }                       ///< rwl_takeRead()
	int  tryLockRead   ()                  { return rwl_tryLockRead   (this); }                       ///< rwl_tryLockRead()
	template<typename T>
	int  waitReadFor   ( const T& _delay ) { return rwl_waitReadFor   (this, Clock::count(_delay)); } ///< rwl_waitReadFor()
	template<typename T>
	int  waitReadUntil ( const T& _time )  { return rwl_waitReadUntil (this, Clock::until(_time)); }  ///< rwl_waitReadUntil()
	int  waitRead      ()                  { return rwl_waitRead      (this); }                       ///< rwl_waitRead()
	int  lockRead      ()                  { return rwl_lockRead      (this); }                       ///< rwl_lockRead()
	void giveRead      ()                  {        rwl_giveRead      (this); }                       ///< rwl_giveRead()
	void unlockRead    ()                  {        rwl_unlockRead    (this); }                       ///< rwl_unlockRead()
	int  takeWrite     ()                  { return rwl_takeWrite     (this); }                       ///< rwl_takeWrite()
	int  tryLockWrite  ()                  { return rwl_tryLockWrite  (this); }                       ///< rwl_tryLockWrite()
	template<typename T>
	int  waitWriteFor  ( const T& _delay ) { return rwl_waitWriteFor  (this, Clock::count(_delay)); } ///< rwl_waitWriteFor()
	template<typename T>
	int  waitWriteUntil( const T& _time )  { return rwl_waitWriteUntil(this, Clock::until(_time)); }  ///< rwl_waitWriteUntil()
	int  waitWrite     ()                  { return rwl_waitWrite     (this); }                       ///< rwl_waitWrite()
	int  lockWrite     ()                  { return rwl_lockWrite     (this); }                       ///< rwl_lockWrite()
	void giveWrite     ()                  {        rwl_giveWrite     (this); }                       ///< rwl_giveWrite()
	void unlockWrite   ()                  {        rwl_unlockWrite   (this); }                       ///< rwl_unlockWrite()

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<RWLock>; ///< result type of Create()
#else
	using Ptr = RWLock *; ///< result type of Create()
#endif

/**
 * @brief   create dynamic object with manageable resources
 *
 * @return  std::unique_pointer / pointer to RWLock object
 *
 * @note    use only in thread mode
 */

	static
	Ptr Create()
	{
		auto rwl = new (std::nothrow) RWLock();
		if (rwl != nullptr)
			rwl->__rwl::obj.res = rwl;
		return Ptr(rwl);
	}

};

/**
 * @brief   create and initialize a read guard object
 *
 * @par     Constructor parameters
 *          - rwl: RWLock object
 */

struct ReadLock
{
	/// constructor, see the constructor parameters of the class
	explicit
	ReadLock( RWLock& _rwl ): lck_(&_rwl)
	{
		int result = lck_->lockRead();
		assert(result == E_SUCCESS || result == E_STOPPED || result == E_DELETED);
		if (result != E_SUCCESS)
			lck_ = nullptr; // nothing to release
	}

	~ReadLock()
	{
		if (lck_ != nullptr)
			lck_->unlockRead();
	}

	/// move constructor
	ReadLock( ReadLock&& _src ): lck_(_src.lck_)
	{
		_src.lck_ = nullptr;
	}

	/// true: the lock is held by the guard; false: the lock was reset or deleted while waiting (or the guard was moved)
	bool ownsLock() const { return lck_ != nullptr; }

	explicit
	operator bool() const { return ownsLock(); } ///< ownsLock()

	ReadLock( const ReadLock& ) = delete;
	ReadLock& operator=( ReadLock&& ) = delete;
	ReadLock& operator=( const ReadLock& ) = delete;

	private:
	RWLock *lck_;
};

/**
 * @brief   create and initialize a write guard object
 *
 * @par     Constructor parameters
 *          - rwl: RWLock object
 */

struct WriteLock
{
	/// constructor, see the constructor parameters of the class
	explicit
	WriteLock( RWLock& _rwl ): lck_(&_rwl)
	{
		int result = lck_->lockWrite();
		assert(result == E_SUCCESS || result == E_STOPPED || result == E_DELETED);
		if (result != E_SUCCESS)
			lck_ = nullptr; // nothing to release
	}

	~WriteLock()
	{
		if (lck_ != nullptr)
			lck_->unlockWrite();
	}

	/// move constructor
	WriteLock( WriteLock&& _src ): lck_(_src.lck_)
	{
		_src.lck_ = nullptr;
	}

	/// true: the lock is held by the guard; false: the lock was reset or deleted while waiting (or the guard was moved)
	bool ownsLock() const { return lck_ != nullptr; }

	explicit
	operator bool() const { return ownsLock(); } ///< ownsLock()

	WriteLock( const WriteLock& ) = delete;
	WriteLock& operator=( WriteLock&& ) = delete;
	WriteLock& operator=( const WriteLock& ) = delete;

	private:
	RWLock *lck_;
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

/** @} */

#endif//__STATEOS_RWL_H
