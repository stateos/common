/******************************************************************************

    @file    StateOS: osmutex.h
    @author  Rajmund Szymanski
    @date    18.06.2023
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

#ifndef __STATEOS_MTX_H
#define __STATEOS_MTX_H

#include "oskernel.h"
#include "osclock.h"

/* -------------------------------------------------------------------------- */

/////// mutex type
#define mtxNormal        0U // normal mutex
#define mtxErrorCheck    1U // error checking mutex
#define mtxRecursive     2U // recursive mutex
#define mtxDefault       mtxNormal
#define mtxTypeMASK    ( mtxNormal | mtxErrorCheck | mtxRecursive )

/////// mutex protocol
#define mtxPrioNone      0U // none
#define mtxPrioInherit   4U // priority inheritance mutex
#define mtxPrioProtect   8U // priority protected mutex (OCPP)
#define mtxPrioMASK    ( mtxPrioNone | mtxPrioInherit | mtxPrioProtect )

/////// mutex robustness
#define mtxStalled       0U // stalled mutex
#define mtxRobust       16U // robust mutex

/////// inconsistency of robust mutex
#define mtxInconsistent 32U // inconsistent mutex

#define mtxMASK        ( mtxTypeMASK | mtxPrioMASK | mtxRobust | mtxInconsistent )

/* -------------------------------------------------------------------------- */

#define MTX_LIMIT      ( 0U-1 )

/******************************************************************************
 *
 * Name              : mutex
 *                     like a POSIX pthread_mutex_t
 *
 ******************************************************************************/

struct __mtx
{
	obj_t    obj;   // object header

	tsk_t  * owner; // mutex owner
	unsigned mode;  // mutex mode: mutex type + mutex protocol + mutex robustness
	unsigned count; // current value of the mutex counter
	unsigned prio;  // mutex priority; used only with mtxPrioProtect protocol
	mtx_t  * list;  // list of mutexes held by owner
};

typedef struct __mtx mtx_id [];

/******************************************************************************
 *
 * Name              : _MTX_INIT
 *
 * Description       : create and initialize a mutex object
 *
 * Parameters
 *   mode            : mutex mode (mutex type + mutex protocol + mutex robustness)
 *                           type: mtxNormal or mtxErrorCheck or mtxRecursive
 *                       protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *                     robustness: mtxStalled or mtxRobust
 *   prio            : mutex priority; used only with mtxPrioProtect protocol
 *
 * Return            : mutex object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _MTX_INIT( _mode, _prio ) { _OBJ_INIT(), NULL, _mode, 0, _prio, NULL }

/******************************************************************************
 *
 * Name              : _VA_MTX
 *
 * Description       : calculate mutex priority from optional parameter
 *                     default: 0
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _VA_MTX( _prio ) ( _prio + 0 )

/******************************************************************************
 *
 * Name              : OS_MTX
 * Static alias      : static_MTX
 *
 * Description       : define and initialize a mutex object
 *
 * Parameters
 *   mtx             : name of a pointer to mutex object
 *   mode            : mutex mode (mutex type + mutex protocol + mutex robustness)
 *                           type: mtxNormal or mtxErrorCheck or mtxRecursive
 *                       protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *                     robustness: mtxStalled or mtxRobust
 *   prio            : mutex priority; used only with mtxPrioProtect protocol
 *
 ******************************************************************************/

#define             OS_MTX( mtx, mode, ... ) \
                       mtx_t mtx[] = { _MTX_INIT(mode, _VA_MTX(__VA_ARGS__) ) }

#define         static_MTX( mtx, mode, ... ) \
                static mtx_t mtx[] = { _MTX_INIT(mode, _VA_MTX(__VA_ARGS__) ) }

/******************************************************************************
 *
 * Name              : MTX_INIT
 *
 * Description       : create and initialize a mutex object
 *
 * Parameters
 *   mode            : mutex mode (mutex type + mutex protocol + mutex robustness)
 *                           type: mtxNormal or mtxErrorCheck or mtxRecursive
 *                       protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *                     robustness: mtxStalled or mtxRobust
 *   prio            : mutex priority; used only with mtxPrioProtect protocol
 *
 * Return            : mutex object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                MTX_INIT( mode, ... ) \
                      _MTX_INIT( mode, _VA_MTX(__VA_ARGS__) )
#endif

/******************************************************************************
 *
 * Name              : MTX_CREATE
 * Alias             : MTX_NEW
 *
 * Description       : create and initialize a mutex object
 *
 * Parameters
 *   mode            : mutex mode (mutex type + mutex protocol + mutex robustness)
 *                           type: mtxNormal or mtxErrorCheck or mtxRecursive
 *                       protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *                     robustness: mtxStalled or mtxRobust
 *   prio            : mutex priority; used only with mtxPrioProtect protocol
 *
 * Return            : mutex object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                MTX_CREATE( mode, ... ) \
                     { MTX_INIT  ( mode, _VA_MTX(__VA_ARGS__) ) }
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
 *   mode            : mutex mode (mutex type + mutex protocol + mutex robustness)
 *                           type: mtxNormal or mtxErrorCheck or mtxRecursive
 *                       protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *                     robustness: mtxStalled or mtxRobust
 *   prio            : mutex priority; used only with mtxPrioProtect protocol
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void mtx_init( mtx_t *mtx, unsigned mode, unsigned prio );

/******************************************************************************
 *
 * Name              : mtx_create
 * Alias             : mtx_new
 *
 * Description       : create and initialize a new mutex object
 *
 * Parameters
 *   mode            : mutex mode (mutex type + mutex protocol + mutex robustness)
 *                           type: mtxNormal or mtxErrorCheck or mtxRecursive
 *                       protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *                     robustness: mtxStalled or mtxRobust
 *   prio            : mutex priority; used only with mtxPrioProtect protocol
 *
 * Return            : pointer to mutex object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

mtx_t *mtx_create( unsigned mode, unsigned prio );

__STATIC_INLINE
mtx_t *mtx_new( unsigned mode, unsigned prio ) { return mtx_create(mode, prio); }

/******************************************************************************
 *
 * Name              : mtx_reset
 * Alias             : mtx_kill
 *
 * Description       : reset the mutex object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   mtx             : pointer to mutex object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void mtx_reset( mtx_t *mtx );

__STATIC_INLINE
void mtx_kill( mtx_t *mtx ) { mtx_reset(mtx); }

/******************************************************************************
 *
 * Name              : mtx_destroy
 * Alias             : mtx_delete
 *
 * Description       : reset the mutex object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * Parameters
 *   mtx             : pointer to mutex object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void mtx_destroy( mtx_t *mtx );

__STATIC_INLINE
void mtx_delete( mtx_t *mtx ) { mtx_destroy(mtx); }

/******************************************************************************
 *
 * Name              : mtx_prio
 *
 * Description       : get priority of given mutex
 *
 * Parameters
 *   mtx             : pointer to mutex object
 *
 * Return            : mutex priority
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

unsigned mtx_prio( mtx_t *mtx );

/******************************************************************************
 *
 * Name              : mtx_take
 * Alias             : mtx_tryLock
 * Async alias       : mtx_takeAsync
 *
 * Description       : try to lock the mutex object,
 *                     don't wait if the mutex object can't be locked immediately
 *
 * Parameters
 *   mtx             : pointer to mutex object
 *
 * Return
 *   E_SUCCESS       : mutex object was successfully locked
 *   OWNERDEAD       : mutex object was successfully locked, previous owner was reseted
 *   E_FAILURE       : mutex object can't be locked
 *   E_TIMEOUT       : mutex object can't be locked immediately, try again
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int mtx_take( mtx_t *mtx );

__STATIC_INLINE
int mtx_tryLock( mtx_t *mtx ) { return mtx_take(mtx); }

#if OS_ATOMICS
int mtx_takeAsync( mtx_t *mtx );
#endif

/******************************************************************************
 *
 * Name              : mtx_waitFor
 *
 * Description       : try to lock the mutex object,
 *                     wait for given duration of time if the mutex object can't be locked immediately
 *
 * Parameters
 *   mtx             : pointer to mutex object
 *   delay           : duration of time (maximum number of ticks to wait for lock the mutex object)
 *                     IMMEDIATE: don't wait if the mutex object can't be locked immediately
 *                     INFINITE:  wait indefinitely until the mutex object has been locked
 *
 * Return
 *   E_SUCCESS       : mutex object was successfully locked
 *   OWNERDEAD       : mutex object was successfully locked, previous owner was reseted
 *   E_FAILURE       : mutex object can't be locked
 *   E_STOPPED       : mutex object was reseted before the specified timeout expired
 *   E_DELETED       : mutex object was deleted before the specified timeout expired
 *   E_TIMEOUT       : mutex object was not locked before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int mtx_waitFor( mtx_t *mtx, cnt_t delay );

/******************************************************************************
 *
 * Name              : mtx_waitUntil
 *
 * Description       : try to lock the mutex object,
 *                     wait until given timepoint if the mutex object can't be locked immediately
 *
 * Parameters
 *   mtx             : pointer to mutex object
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : mutex object was successfully locked
 *   OWNERDEAD       : mutex object was successfully locked, previous owner was reseted
 *   E_FAILURE       : mutex object can't be locked
 *   E_STOPPED       : mutex object was reseted before the specified timeout expired
 *   E_DELETED       : mutex object was deleted before the specified timeout expired
 *   E_TIMEOUT       : mutex object was not locked before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int mtx_waitUntil( mtx_t *mtx, cnt_t time );

/******************************************************************************
 *
 * Name              : mtx_wait
 * Alias             : mtx_lock
 * Async alias       : mtx_waitAsync
 *
 * Description       : try to lock the mutex object,
 *                     wait indefinitely if the mutex object can't be locked immediately
 *
 * Parameters
 *   mtx             : pointer to mutex object
 *
 * Return
 *   E_SUCCESS       : mutex object was successfully locked
 *   OWNERDEAD       : mutex object was successfully locked, previous owner was reseted
 *   E_FAILURE       : mutex object can't be locked
 *   E_STOPPED       : mutex object was reseted (unavailable for async version)
 *   E_DELETED       : mutex object was deleted (unavailable for async version)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
int mtx_wait( mtx_t *mtx ) { return mtx_waitFor(mtx, INFINITE); }

__STATIC_INLINE
int mtx_lock( mtx_t *mtx ) { return mtx_wait(mtx); }

#if OS_ATOMICS
int mtx_waitAsync( mtx_t *mtx );
#endif

/******************************************************************************
 *
 * Name              : mtx_give
 * Alias             : mtx_unlock
 * Async alias       : mtx_giveAsync
 *
 * Description       : try to unlock the mutex object (only owner task can unlock mutex object),
 *                     don't wait if the mutex object can't be unlocked
 *
 * Parameters
 *   mtx             : pointer to mutex object
 *
 * Return
 *   E_SUCCESS       : mutex object was successfully unlocked
 *   E_FAILURE       : mutex object can't be unlocked
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int mtx_give( mtx_t *mtx );

__STATIC_INLINE
int mtx_unlock( mtx_t *mtx ) { return mtx_give(mtx); }

#if OS_ATOMICS
int mtx_giveAsync( mtx_t *mtx );
#endif

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace stateos {

/******************************************************************************
 *
 * Class             : Mutex
 *
 * Description       : create and initialize a mutex object
 *
 * Constructor parameters
 *   mode            : mutex mode (mutex type + mutex protocol + mutex robustness)
 *                           type: mtxNormal or mtxErrorCheck or mtxRecursive
 *                       protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *                     robustness: mtxStalled or mtxRobust
 *   prio            : mutex priority; used only with mtxPrioProtect protocol
 *
 ******************************************************************************/

struct Mutex : public __mtx
{
	constexpr
	Mutex( const unsigned _mode, const unsigned _prio = 0 ): __mtx _MTX_INIT(_mode, _prio) {}

	~Mutex() { assert(__mtx::owner == nullptr); }

	Mutex( Mutex&& ) = default;
	Mutex( const Mutex& ) = delete;
	Mutex& operator=( Mutex&& ) = delete;
	Mutex& operator=( const Mutex& ) = delete;

	void     reset    ()                  {        mtx_reset    (this); }
	void     kill     ()                  {        mtx_kill     (this); }
	void     destroy  ()                  {        mtx_destroy  (this); }
	unsigned prio     ()                  { return mtx_prio     (this); }
	int      take     ()                  { return mtx_take     (this); }
	int      tryLock  ()                  { return mtx_tryLock  (this); }
	template<typename T>
	int      waitFor  ( const T& _delay ) { return mtx_waitFor  (this, Clock::count(_delay)); }
	template<typename T>
	int      waitUntil( const T& _time )  { return mtx_waitUntil(this, Clock::until(_time)); }
	int      wait     ()                  { return mtx_wait     (this); }
	int      lock     ()                  { return mtx_lock     (this); }
	int      give     ()                  { return mtx_give     (this); }
	int      unlock   ()                  { return mtx_unlock   (this); }
#if OS_ATOMICS
	int      takeAsync ()                 { return mtx_takeAsync (this); }
	int      waitAsync ()                 { return mtx_waitAsync (this); }
	int      giveAsync ()                 { return mtx_giveAsync (this); }
#endif

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<Mutex>;
#else
	using Ptr = Mutex *;
#endif

/******************************************************************************
 *
 * Name              : Mutex::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters
 *   mode            : mutex mode (mutex type + mutex protocol + mutex robustness)
 *                           type: mtxNormal or mtxErrorCheck or mtxRecursive
 *                       protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *                     robustness: mtxStalled or mtxRobust
 *   prio            : mutex priority; used only with mtxPrioProtect protocol
 *
 * Return            : std::unique_pointer / pointer to Mutex object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create( const unsigned _mode, const unsigned _prio = 0 )
	{
		auto mtx = new (std::nothrow) Mutex(_mode, _prio);
		if (mtx != nullptr)
			mtx->__mtx::obj.res = mtx;
		return Ptr(mtx);
	}

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
		int result = mtx_.lock();
		assert(result == E_SUCCESS);
		(void) result;
	}

	~LockGuard()
	{
		int result = mtx_.unlock();
		assert(result == E_SUCCESS);
		(void) result;
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

	template<typename T>
	UniqueLock( Mutex& _mtx, const T& _delay ): mtx_(&_mtx), locked_(false)
	{
		waitFor(_delay);
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

		int result = mtx_->tryLock();
		assert(result == E_SUCCESS || result == E_TIMEOUT);
		locked_ = result == E_SUCCESS;
		return locked_;
	}

	template<typename T>
	bool waitFor( const T& _delay )
	{
		if (mtx_ == nullptr || locked_)
			return false;

		int result = mtx_->waitFor(_delay);
		assert(result == E_SUCCESS || result == E_TIMEOUT);
		locked_ = result == E_SUCCESS;
		return locked_;
	}

	template<typename T>
	bool waitUntil( const T& _time )
	{
		if (mtx_ == nullptr || locked_)
			return false;

		int result = mtx_->waitUntil(_time);
		assert(result == E_SUCCESS || result == E_TIMEOUT);
		locked_ = result == E_SUCCESS;
		return locked_;
	}

	bool lock()
	{
		if (mtx_ == nullptr || locked_)
			return false;

		int result = mtx_->lock();
		assert(result == E_SUCCESS);
		locked_ = result == E_SUCCESS;
		return locked_;
	}

	bool unlock()
	{
		if (mtx_ == nullptr || !locked_)
			return false;

		int result = mtx_->unlock();
		assert(result == E_SUCCESS);
		locked_ = false;
		return result == E_SUCCESS;
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

#endif//__STATEOS_MTX_H
