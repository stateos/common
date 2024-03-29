/******************************************************************************

    @file    IntrOS: osmutex.h
    @author  Rajmund Szymanski
    @date    26.07.2022
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

#ifndef __INTROS_MTX_H
#define __INTROS_MTX_H

#include "oskernel.h"

/******************************************************************************
 *
 * Name              : mutex
 *
 ******************************************************************************/

typedef struct __mtx mtx_t;

struct __mtx
{
	tsk_t *  owner; // mutex owner
};

typedef struct __mtx mtx_id [];

/******************************************************************************
 *
 * Name              : _MTX_INIT
 *
 * Description       : create and initialize a mutex object
 *
 * Parameters        : none
 *
 * Return            : mutex object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _MTX_INIT() { NULL }

/******************************************************************************
 *
 * Name              : OS_MTX
 * Static alias      : static_MTX
 *
 * Description       : define and initialize a mutex object
 *
 * Parameters
 *   mtx             : name of a pointer to mutex object
 *
 ******************************************************************************/

#define             OS_MTX( mtx ) \
                       mtx_t mtx[] = { _MTX_INIT() }

#define         static_MTX( mtx ) \
                static mtx_t mtx[] = { _MTX_INIT() }

/******************************************************************************
 *
 * Name              : MTX_INIT
 *
 * Description       : create and initialize a mutex object
 *
 * Parameters        : none
 *
 * Return            : mutex object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                MTX_INIT() \
                      _MTX_INIT()
#endif

/******************************************************************************
 *
 * Name              : MTX_CREATE
 * Alias             : MTX_NEW
 *
 * Description       : create and initialize a mutex object
 *
 * Parameters        : none
 *
 * Return            : mutex object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                MTX_CREATE() \
                     { MTX_INIT  () }
#define                MTX_NEW \
                       MTX_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : mtx_init
 *
 * Description       : initialize a mutex object
 *
 * Parameters
 *   mtx             : pointer to mutex object
 *
 * Return            : none
 *
 ******************************************************************************/

void mtx_init( mtx_t *mtx );

/******************************************************************************
 *
 * Name              : mtx_take
 * Alias             : mtx_tryLock
 *
 * Description       : try to lock the mutex object,
 *                     don't wait if the mutex object can't be locked immediately
 *
 * Parameters
 *   mtx             : pointer to mutex object
 *
 * Return
 *   SUCCESS         : mutex object was successfully locked
 *   FAILURE         : mutex object can't be locked immediately
 *
 ******************************************************************************/

unsigned mtx_take( mtx_t *mtx );

__STATIC_INLINE
unsigned mtx_tryLock( mtx_t *mtx ) { return mtx_take(mtx); }

/******************************************************************************
 *
 * Name              : mtx_wait
 * Alias             : mtx_lock
 *
 * Description       : try to lock the mutex object,
 *                     wait indefinitely if the mutex object can't be locked immediately
 *
 * Parameters
 *   mtx             : pointer to mutex object
 *
 * Return            : none
 *
 ******************************************************************************/

void mtx_wait( mtx_t *mtx );

__STATIC_INLINE
void mtx_lock( mtx_t *mtx ) { mtx_wait(mtx); }

/******************************************************************************
 *
 * Name              : mtx_give
 * Alias             : mtx_unlock
 *
 * Description       : try to unlock the mutex object (only owner task can unlock mutex object),
 *                     don't wait if the mutex object can't be unlocked
 *
 * Parameters
 *   mtx             : pointer to mutex object
 *
 * Return
 *   SUCCESS         : mutex object was successfully unlocked
 *   FAILURE         : mutex object can't be unlocked
 *
 ******************************************************************************/

unsigned mtx_give( mtx_t *mtx );

__STATIC_INLINE
unsigned mtx_unlock( mtx_t *mtx ) { return mtx_give(mtx); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
namespace intros {

/******************************************************************************
 *
 * Class             : Mutex
 *
 * Description       : create and initialize a mutex object
 *
 * Constructor parameters
 *                   : none
 *
 ******************************************************************************/

struct Mutex : public __mtx
{
	constexpr
	Mutex(): __mtx _MTX_INIT() {}

	Mutex( Mutex&& ) = default;
	Mutex( const Mutex& ) = delete;
	Mutex& operator=( Mutex&& ) = delete;
	Mutex& operator=( const Mutex& ) = delete;

	~Mutex() { assert(__mtx::owner == nullptr); }

	unsigned take   () { return mtx_take   (this); }
	unsigned tryLock() { return mtx_tryLock(this); }
	void     wait   () {        mtx_wait   (this); }
	void     lock   () {        mtx_lock   (this); }
	unsigned give   () { return mtx_give   (this); }
	unsigned unlock () { return mtx_unlock (this); }
};

/******************************************************************************
 *
 * Class             : LockGuard
 *
 * Description       : create and initialize a lock guard object
 *
 ******************************************************************************/

struct LockGuard
{
	explicit
	LockGuard( Mutex& _mtx ): mtx_(_mtx)
	{
		mtx_.lock();
	}

	~LockGuard()
	{
		mtx_.unlock();
	}

	LockGuard( LockGuard&& ) = default;
	LockGuard( const LockGuard& ) = delete;
	LockGuard& operator=( LockGuard&& ) = delete;
	LockGuard& operator=( const LockGuard& ) = delete;

	private:
	Mutex& mtx_;
};

/******************************************************************************
 *
 * Class             : UniqueLock
 *
 * Description       : create and initialize a unique lock object
 *
 ******************************************************************************/

struct UniqueLock
{
	explicit
	UniqueLock( Mutex& _mtx ): mtx_(&_mtx), locked_(false)
	{
		lock();
	}

	UniqueLock( UniqueLock&& _src ): mtx_(_src.mtx_), locked_(_src.locked_)
	{
		_src.mtx_ = nullptr;
		_src.locked_ = false;
	}

	~UniqueLock()
	{
		unlock();
	}

	UniqueLock() = default;
	UniqueLock( const UniqueLock& ) = delete;
	UniqueLock& operator=( const UniqueLock& ) = delete;

	UniqueLock& operator=( UniqueLock&& _src )
	{
		unlock();

		mtx_ = _src.mtx_;
		locked_ = _src.locked_;

		_src.mtx_ = nullptr;
		_src.locked_ = false;

		return *this;
	}

	bool tryLock()
	{
		if (mtx_ == nullptr || locked_)
			return false;

		unsigned result = mtx_->tryLock();
		assert(result == SUCCESS);
		locked_ = result == SUCCESS;
		return locked_;
	}

	void lock()
	{
		if (mtx_ != nullptr && !locked_)
			mtx_->lock();
	}

	void unlock()
	{
		if (mtx_ != nullptr && locked_)
			mtx_->unlock();
	}

	bool ownsLock() const
	{
		return locked_;
	}

	explicit
	operator bool() const
	{
		return ownsLock();
	}

	Mutex* release()
	{
		Mutex* result = mtx_;

		mtx_ = nullptr;
		locked_ = false;

		return result;
	}

	Mutex* mutex() const
	{
		return mtx_;
	}

	private:
	Mutex *mtx_ = nullptr;
	bool locked_ = false;
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_MTX_H
