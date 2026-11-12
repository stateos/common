/******************************************************************************

    @file    StateOS: osmutex.h
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

#ifndef __STATEOS_MTX_H
#define __STATEOS_MTX_H

#include "oskernel.h"
#include "osclock.h"

/**
 * @defgroup mutex Mutex
 * @brief    mutexes with configurable type, protocol and robustness
 * @{
 */

/* -------------------------------------------------------------------------- */

/////// mutex type
#define mtxNormal        0U ///< normal mutex
#define mtxErrorCheck    1U ///< error checking mutex
#define mtxRecursive     2U ///< recursive mutex
#define mtxDefault       mtxNormal ///< default mutex type
#define mtxTypeMASK    ( mtxNormal | mtxErrorCheck | mtxRecursive )

/////// mutex protocol
#define mtxPrioNone      0U ///< none
#define mtxPrioInherit   4U ///< priority inheritance mutex
#define mtxPrioProtect   8U ///< priority protected mutex (ICPP, POSIX PTHREAD_PRIO_PROTECT): the owner runs at least
                            // at the mutex priority (ceiling), a task with a higher priority cannot lock it
#define mtxPrioMASK    ( mtxPrioNone | mtxPrioInherit | mtxPrioProtect )

/////// mutex robustness
#define mtxStalled       0U ///< stalled mutex
#define mtxRobust       16U ///< robust mutex

/////// inconsistency of robust mutex
#define mtxInconsistent 32U ///< inconsistent mutex

#define mtxMASK        ( mtxTypeMASK | mtxPrioMASK | mtxRobust | mtxInconsistent )

/* -------------------------------------------------------------------------- */

#define MTX_LIMIT      ( 0U-1 )

/**
 * @brief   mutex
 */

struct __mtx
{
	obj_t    obj;   ///< object header

	tsk_t  * owner; ///< mutex owner
	unsigned mode;  ///< mutex mode: mutex type + mutex protocol + mutex robustness
	unsigned count; ///< current value of the mutex counter
	unsigned prio;  ///< mutex priority ceiling; used only with mtxPrioProtect protocol
	mtx_t  * list;  ///< list of mutexes held by owner
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
 *   prio            : mutex priority ceiling; used only with mtxPrioProtect protocol
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

/**
 * @brief   define and initialize a mutex object
 *
 * @param   mtx        name of a pointer to mutex object
 * @param   mode       mutex mode (mutex type + mutex protocol + mutex robustness)
 *                     - type: mtxNormal or mtxErrorCheck or mtxRecursive
 *                     - protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *                     - robustness: mtxStalled or mtxRobust
 * @param   ...        prio: mutex priority ceiling; used only with mtxPrioProtect protocol
 */

#define             OS_MTX( mtx, mode, ... ) \
                       mtx_t mtx[] = { _MTX_INIT(mode, _VA_MTX(__VA_ARGS__) ) }

/** static alias of OS_MTX() */
#define         static_MTX( mtx, mode, ... ) \
                static mtx_t mtx[] = { _MTX_INIT(mode, _VA_MTX(__VA_ARGS__) ) }

/**
 * @brief   create and initialize a mutex object
 *
 * @param   mode       mutex mode (mutex type + mutex protocol + mutex robustness)
 *                     - type: mtxNormal or mtxErrorCheck or mtxRecursive
 *                     - protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *                     - robustness: mtxStalled or mtxRobust
 * @param   ...        prio: mutex priority ceiling; used only with mtxPrioProtect protocol
 *
 * @return  mutex object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                MTX_INIT( mode, ... ) \
                      _MTX_INIT( mode, _VA_MTX(__VA_ARGS__) )
#endif

/**
 * @brief   create and initialize a mutex object
 *
 * @param   mode       mutex mode (mutex type + mutex protocol + mutex robustness)
 *                     - type: mtxNormal or mtxErrorCheck or mtxRecursive
 *                     - protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *                     - robustness: mtxStalled or mtxRobust
 * @param   ...        prio: mutex priority ceiling; used only with mtxPrioProtect protocol
 *
 * @return  mutex object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                MTX_CREATE( mode, ... ) \
                     { MTX_INIT  ( mode, _VA_MTX(__VA_ARGS__) ) }
/** alias of MTX_CREATE() */
#define                MTX_NEW \
                       MTX_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize a mutex object
 *
 * @param   mtx        pointer to mutex object
 * @param   mode       mutex mode (mutex type + mutex protocol + mutex robustness)
 *                     - type: mtxNormal or mtxErrorCheck or mtxRecursive
 *                     - protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *                     - robustness: mtxStalled or mtxRobust
 * @param   prio       mutex priority ceiling; used only with mtxPrioProtect protocol
 *
 * @note    use only in thread mode
 */

void mtx_init( mtx_t *mtx, unsigned mode, unsigned prio );

/**
 * @brief   create and initialize a new mutex object
 *
 * @param   mode       mutex mode (mutex type + mutex protocol + mutex robustness)
 *                     - type: mtxNormal or mtxErrorCheck or mtxRecursive
 *                     - protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *                     - robustness: mtxStalled or mtxRobust
 * @param   prio       mutex priority ceiling; used only with mtxPrioProtect protocol
 *
 * @return  pointer to mutex object
 * @retval  NULL       object not created (not enough free memory)
 *
 * @note    use only in thread mode
 */

mtx_t *mtx_create( unsigned mode, unsigned prio );

/** alias of mtx_create() */
__STATIC_INLINE
mtx_t *mtx_new( unsigned mode, unsigned prio ) { return mtx_create(mode, prio); }

/**
 * @brief   reset the mutex object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * @param   mtx        pointer to mutex object
 *
 * @note    use only in thread mode
 */

void mtx_reset( mtx_t *mtx );

/** alias of mtx_reset() */
__STATIC_INLINE
void mtx_kill( mtx_t *mtx ) { mtx_reset(mtx); }

/**
 * @brief   reset the mutex object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * @param   mtx        pointer to mutex object
 *
 * @note    use only in thread mode
 */

void mtx_destroy( mtx_t *mtx );

/** alias of mtx_destroy() */
__STATIC_INLINE
void mtx_delete( mtx_t *mtx ) { mtx_destroy(mtx); }

/**
 * @brief   get priority of given mutex
 *
 * @param   mtx        pointer to mutex object
 *
 * @return  mutex priority
 *
 * @note    use only in thread mode
 */

unsigned mtx_prio( mtx_t *mtx );

/**
 * @brief   try to lock the mutex object,
 *          don't wait if the mutex object can't be locked immediately
 *
 * @param   mtx        pointer to mutex object
 *
 * @retval  E_SUCCESS  mutex object was successfully locked
 * @retval  OWNERDEAD  mutex object was successfully locked, previous owner has stopped
 * @retval  E_FAILURE  mutex object can't be locked (also: the task priority is above
 *                     the priority ceiling of a mtxPrioProtect mutex)
 * @retval  E_TIMEOUT  mutex object can't be locked immediately, try again
 *
 * @note    use only in thread mode
 */

int mtx_take( mtx_t *mtx );

/** alias of mtx_take() */
__STATIC_INLINE
int mtx_tryLock( mtx_t *mtx ) { return mtx_take(mtx); }

/**
 * @brief   try to lock the mutex object,
 *          wait for given duration of time if the mutex object can't be locked immediately
 *
 * @param   mtx        pointer to mutex object
 * @param   delay      duration of time (maximum number of ticks to wait for lock the mutex object)
 *                     - IMMEDIATE: don't wait if the mutex object can't be locked immediately
 *                     - INFINITE:  wait indefinitely until the mutex object has been locked
 *
 * @retval  E_SUCCESS  mutex object was successfully locked
 * @retval  OWNERDEAD  mutex object was successfully locked, previous owner has stopped
 * @retval  E_FAILURE  mutex object can't be locked (also: the task priority is above
 *                     the priority ceiling of a mtxPrioProtect mutex)
 * @retval  E_STOPPED  mutex object was reseted before the specified timeout expired
 * @retval  E_DELETED  mutex object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  mutex object was not locked before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int mtx_waitFor( mtx_t *mtx, cnt_t delay );

/**
 * @brief   try to lock the mutex object,
 *          wait until given timepoint if the mutex object can't be locked immediately
 *
 * @param   mtx        pointer to mutex object
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  mutex object was successfully locked
 * @retval  OWNERDEAD  mutex object was successfully locked, previous owner has stopped
 * @retval  E_FAILURE  mutex object can't be locked (also: the task priority is above
 *                     the priority ceiling of a mtxPrioProtect mutex)
 * @retval  E_STOPPED  mutex object was reseted before the specified timeout expired
 * @retval  E_DELETED  mutex object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  mutex object was not locked before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int mtx_waitUntil( mtx_t *mtx, cnt_t time );

/**
 * @brief   try to lock the mutex object,
 *          wait indefinitely if the mutex object can't be locked immediately
 *
 * @param   mtx        pointer to mutex object
 *
 * @retval  E_SUCCESS  mutex object was successfully locked
 * @retval  OWNERDEAD  mutex object was successfully locked, previous owner has stopped
 * @retval  E_FAILURE  mutex object can't be locked (also: the task priority is above
 *                     the priority ceiling of a mtxPrioProtect mutex)
 * @retval  E_STOPPED  mutex object was reseted
 * @retval  E_DELETED  mutex object was deleted
 *
 * @note    use only in thread mode
 */

__STATIC_INLINE
int mtx_wait( mtx_t *mtx ) { return mtx_waitFor(mtx, INFINITE); }

/** alias of mtx_wait() */
__STATIC_INLINE
int mtx_lock( mtx_t *mtx ) { return mtx_wait(mtx); }

/**
 * @brief   try to unlock the mutex object (only owner task can unlock mutex object),
 *          don't wait if the mutex object can't be unlocked
 *
 * @param   mtx        pointer to mutex object
 *
 * @retval  E_SUCCESS  mutex object was successfully unlocked
 * @retval  E_FAILURE  mutex object can't be unlocked
 *
 * @note    use only in thread mode
 */

int mtx_give( mtx_t *mtx );

/** alias of mtx_give() */
__STATIC_INLINE
int mtx_unlock( mtx_t *mtx ) { return mtx_give(mtx); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(__STATEOS_STDCXX)
namespace stateos {

/**
 * @brief   create and initialize a mutex object
 *
 * @par     Constructor parameters
 *          - mode: mutex mode (mutex type + mutex protocol + mutex robustness)
 *            - type: mtxNormal or mtxErrorCheck or mtxRecursive
 *            - protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *            - robustness: mtxStalled or mtxRobust
 *          - prio: mutex priority ceiling; used only with mtxPrioProtect protocol
 */

struct Mutex : public __mtx
{
	/// constructor, see the constructor parameters of the class
	constexpr
	Mutex(): __mtx _MTX_INIT(mtxPrioInherit|mtxErrorCheck, 0) {}
	/// constructor, see the constructor parameters of the class
	constexpr
	Mutex( const unsigned _mode, const unsigned _prio = 0 ): __mtx _MTX_INIT(_mode, _prio) {}

	~Mutex() { assert(__mtx::owner == nullptr); }

	/// move constructor
	Mutex( Mutex&& _src ): __mtx(_src)
	{
		assert(_src.owner == nullptr && _src.obj.queue == nullptr);
	}

	Mutex( const Mutex& ) = delete;
	Mutex& operator=( Mutex&& ) = delete;
	Mutex& operator=( const Mutex& ) = delete;

	void     reset    ()                  {        mtx_reset    (this); }                       ///< mtx_reset()
	void     kill     ()                  {        mtx_kill     (this); }                       ///< mtx_kill()
	void     destroy  ()                  {        mtx_destroy  (this); }                       ///< mtx_destroy()
	unsigned prio     ()                  { return mtx_prio     (this); }                       ///< mtx_prio()
	int      take     ()                  { return mtx_take     (this); }                       ///< mtx_take()
	int      tryLock  ()                  { return mtx_tryLock  (this); }                       ///< mtx_tryLock()
	template<typename T>
	int      waitFor  ( const T& _delay ) { return mtx_waitFor  (this, Clock::count(_delay)); } ///< mtx_waitFor()
	template<typename T>
	int      waitUntil( const T& _time )  { return mtx_waitUntil(this, Clock::until(_time)); }  ///< mtx_waitUntil()
	int      wait     ()                  { return mtx_wait     (this); }                       ///< mtx_wait()
	int      lock     ()                  { return mtx_lock     (this); }                       ///< mtx_lock()
	int      give     ()                  { return mtx_give     (this); }                       ///< mtx_give()
	int      unlock   ()                  { return mtx_unlock   (this); }                       ///< mtx_unlock()

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<Mutex>; ///< result type of Create()
#else
	using Ptr = Mutex *; ///< result type of Create()
#endif

/**
 * @brief   create dynamic object with manageable resources
 *
 * @param   _mode      mutex mode (mutex type + mutex protocol + mutex robustness)
 *                     - type: mtxNormal or mtxErrorCheck or mtxRecursive
 *                     - protocol: mtxPrioNone or mtxPrioInherit or mtxPrioProtect
 *                     - robustness: mtxStalled or mtxRobust
 * @param   _prio      mutex priority ceiling; used only with mtxPrioProtect protocol
 *
 * @return  std::unique_pointer / pointer to Mutex object
 *
 * @note    use only in thread mode
 */

	static
	Ptr Create( const unsigned _mode, const unsigned _prio = 0 )
	{
		auto mtx = new (std::nothrow) Mutex(_mode, _prio);
		if (mtx != nullptr)
			mtx->__mtx::obj.res = mtx;
		return Ptr(mtx);
	}

};

/**
 * @brief   create and initialize a lock guard object
 */

struct LockGuard
{
	/// constructor, see the constructor parameters of the class
	explicit
	LockGuard( Mutex& _mtx ): mtx_(&_mtx)
	{
		int result = mtx_->lock();
		assert(result == E_SUCCESS || result == OWNERDEAD || result == E_STOPPED || result == E_DELETED);
		if (result != E_SUCCESS && result != OWNERDEAD) // OWNERDEAD: the robust mutex is locked, its previous owner has stopped
			mtx_ = nullptr; // nothing to release
	}

	~LockGuard()
	{
		if (mtx_ == nullptr)
			return;

		int result = mtx_->unlock();
		assert(result == E_SUCCESS);
		(void) result;
	}

	/// move constructor
	LockGuard( LockGuard&& _src ): mtx_(_src.mtx_)
	{
		_src.mtx_ = nullptr;
	}

	/// true: the mutex is locked by the guard; false: the mutex was reset or deleted while waiting (or the guard was moved)
	bool ownsLock() const { return mtx_ != nullptr; }

	explicit
	operator bool() const { return ownsLock(); } ///< ownsLock()

	LockGuard( const LockGuard& ) = delete;
	LockGuard& operator=( LockGuard&& ) = delete;
	LockGuard& operator=( const LockGuard& ) = delete;

	private:
	Mutex *mtx_;
};

/**
 * @brief   create and initialize a unique lock object
 */

struct UniqueLock
{
	/// constructor, see the constructor parameters of the class
	explicit
	UniqueLock( Mutex& _mtx ): mtx_(&_mtx), locked_(false)
	{
		lock();
	}

	/// constructor, see the constructor parameters of the class
	template<typename T>
	UniqueLock( Mutex& _mtx, const T& _delay ): mtx_(&_mtx), locked_(false)
	{
		waitFor(_delay);
	}

	/// move constructor
	UniqueLock( UniqueLock&& _src ): mtx_(_src.mtx_), locked_(_src.locked_)
	{
		_src.mtx_ = nullptr;
		_src.locked_ = false;
	}

	~UniqueLock()
	{
		unlock();
	}

	/// default constructor
	UniqueLock() = default;
	UniqueLock( const UniqueLock& ) = delete;
	UniqueLock& operator=( const UniqueLock& ) = delete;

	/// unlock the held mutex and take over the mutex of _src
	UniqueLock& operator=( UniqueLock&& _src )
	{
		unlock();

		mtx_ = _src.mtx_;
		locked_ = _src.locked_;

		_src.mtx_ = nullptr;
		_src.locked_ = false;

		return *this;
	}

	/// try to lock the mutex, don't wait; true: the mutex is locked
	bool tryLock()
	{
		if (mtx_ == nullptr || locked_)
			return false;

		int result = mtx_->tryLock();
		assert(result == E_SUCCESS || result == OWNERDEAD || result == E_TIMEOUT);
		locked_ = result == E_SUCCESS || result == OWNERDEAD;
		return locked_;
	}

	/// try to lock the mutex, wait for given duration of time; true: the mutex is locked
	template<typename T>
	bool waitFor( const T& _delay )
	{
		if (mtx_ == nullptr || locked_)
			return false;

		int result = mtx_->waitFor(_delay);
		assert(result == E_SUCCESS || result == OWNERDEAD || result == E_TIMEOUT || result == E_STOPPED || result == E_DELETED);
		locked_ = result == E_SUCCESS || result == OWNERDEAD;
		return locked_;
	}

	/// try to lock the mutex, wait until given timepoint; true: the mutex is locked
	template<typename T>
	bool waitUntil( const T& _time )
	{
		if (mtx_ == nullptr || locked_)
			return false;

		int result = mtx_->waitUntil(_time);
		assert(result == E_SUCCESS || result == OWNERDEAD || result == E_TIMEOUT || result == E_STOPPED || result == E_DELETED);
		locked_ = result == E_SUCCESS || result == OWNERDEAD;
		return locked_;
	}

	/// lock the mutex, wait indefinitely; true: the mutex is locked
	bool lock()
	{
		if (mtx_ == nullptr || locked_)
			return false;

		int result = mtx_->lock();
		assert(result == E_SUCCESS || result == OWNERDEAD || result == E_STOPPED || result == E_DELETED);
		locked_ = result == E_SUCCESS || result == OWNERDEAD; // OWNERDEAD: the robust mutex is locked, its previous owner has stopped
		return locked_;
	}

	/// unlock the mutex; true: the mutex was unlocked
	bool unlock()
	{
		if (mtx_ == nullptr || !locked_)
			return false;

		int result = mtx_->unlock();
		assert(result == E_SUCCESS);
		locked_ = false;
		return result == E_SUCCESS;
	}

	/// true: the mutex is locked by the object
	bool ownsLock() const
	{
		return locked_;
	}

	/// ownsLock()
	explicit
	operator bool() const
	{
		return ownsLock();
	}

	/// release the mutex without unlocking it, return the mutex
	Mutex* release()
	{
		Mutex* result = mtx_;

		mtx_ = nullptr;
		locked_ = false;

		return result;
	}

	/// associated mutex
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

/** @} */

#endif//__STATEOS_MTX_H
