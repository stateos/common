#ifndef _GLIBCXX_GCC_GTHR_STATEOS_H
#define _GLIBCXX_GCC_GTHR_STATEOS_H

#include "os.h"

//-----------------------------------------------------------------------------

#if     OS_TASK_EXIT == 0
#error  osconfig.h: Invalid OS_TASK_EXIT value! It must be a value other than 0.
#endif

//-----------------------------------------------------------------------------

typedef int   __gthread_key_t;
typedef one_t __gthread_once_t;
typedef mtx_t __gthread_mutex_t;
typedef mtx_t __gthread_recursive_mutex_t;
typedef cnd_t __gthread_cond_t;
typedef cnt_t __gthread_time_t;

//-----------------------------------------------------------------------------

struct ostask_t : public tsk_t
{
	ostask_t() noexcept = default;

	~ostask_t() noexcept = default;

	ostask_t( ostask_t&& ) = delete;
	ostask_t( const ostask_t& ) = delete;
	ostask_t& operator=( ostask_t&& ) = delete;
	ostask_t& operator=( const ostask_t& ) = delete;
	
	static
	void handler()
	{
		ostask_t *task = reinterpret_cast<ostask_t *>(tsk_this());
		task->fun(task->arg);
	}

	void (*fun)(void *);
	void  *arg;
	stk_t  stk[];
};

typedef ostask_t *__gthread_t;

//-----------------------------------------------------------------------------

#define __GTHREADS 1
#define __GTHREADS_CXX0X 1
#define __GTHREAD_HAS_COND
#define  _GLIBCXX_USE_SCHED_YIELD

#define __GTHREAD_ONCE_INIT            _ONE_INIT()
#define __GTHREAD_MUTEX_INIT           _MTX_INIT(mtxPrioInherit|mtxErrorCheck, 0)
#define __GTHREAD_RECURSIVE_MUTEX_INIT _MTX_INIT(mtxPrioInherit|mtxRecursive, 0)
#define __GTHREAD_COND_INIT            _CND_INIT()

static inline
void __GTHREAD_MUTEX_INIT_FUNCTION(__gthread_mutex_t *mutex)
{
	mtx_init(mutex, mtxPrioInherit | mtxErrorCheck, 0);
}

static inline
void __GTHREAD_RECURSIVE_MUTEX_INIT_FUNCTION(__gthread_recursive_mutex_t *mutex)
{
	mtx_init(mutex, mtxPrioInherit | mtxRecursive, 0);
}

static inline
void __GTHREAD_COND_INIT_FUNCTION(__gthread_cond_t *cond)
{
	cnd_init(cond);
}

//-----------------------------------------------------------------------------

static inline
int __gthread_active_p()
{
	return 1;
}

static inline
int __gthread_once(__gthread_once_t *once, void(*func)())
{
	one_call(once, func);
	return 0;
}

static inline
int __gthread_key_create(__gthread_key_t *keyp, void(*dtor)(void *))
{
	return 0;
}

static inline
int __gthread_key_delete(__gthread_key_t key)
{
	return 0;
}

static inline
void *__gthread_getspecific(__gthread_key_t key)
{
	return nullptr;
}

static inline
int __gthread_setspecific(__gthread_key_t key, const void *ptr)
{
	return 0;
}

static inline
int __gthread_mutex_destroy(__gthread_mutex_t *mutex)
{
	mtx_destroy(mutex);
	return 0;
}

static inline
int __gthread_recursive_mutex_destroy(__gthread_recursive_mutex_t *mutex)
{
	mtx_destroy(mutex);
	return 0;
}

static inline
int __gthread_mutex_lock(__gthread_mutex_t *mutex)
{
	// EINVAL, EAGAIN, EBUSY, EINVAL, EDEADLK(may)
	return mtx_lock(mutex);
}

static inline
int __gthread_mutex_trylock(__gthread_mutex_t *mutex)
{
	// XXX EINVAL, EAGAIN, EBUSY
	return mtx_tryLock(mutex);
}

static inline
int __gthread_mutex_timedlock(__gthread_mutex_t *m, const __gthread_time_t *abs_timeout)
{
	return mtx_waitUntil(m, *abs_timeout);
}

static inline
int __gthread_mutex_unlock(__gthread_mutex_t *mutex)
{
	// XXX EINVAL, EAGAIN, EBUSY
	return mtx_unlock(mutex);
}

static inline
int __gthread_recursive_mutex_lock(__gthread_recursive_mutex_t *mutex)
{
	// EINVAL, EAGAIN, EBUSY, EINVAL, EDEADLK(may)
	return mtx_lock(mutex);
}

static inline
int __gthread_recursive_mutex_trylock(__gthread_recursive_mutex_t *mutex)
{
	// XXX EINVAL, EAGAIN, EBUSY
	return mtx_tryLock(mutex);
}

static inline
int __gthread_recursive_mutex_timedlock(__gthread_recursive_mutex_t *m, const __gthread_time_t *abs_timeout)
{
	return mtx_waitUntil(m, *abs_timeout);
}

static inline
int __gthread_recursive_mutex_unlock(__gthread_recursive_mutex_t *mutex)
{
	// XXX EINVAL, EAGAIN, EBUSY
	return mtx_unlock(mutex);
}

static inline
int __gthread_cond_signal(__gthread_cond_t *cond)
{
	cnd_notifyOne(cond);
}

static inline
int __gthread_cond_broadcast(__gthread_cond_t *cond)
{
	cnd_notifyAll(cond);
}

static inline
int __gthread_cond_wait(__gthread_cond_t *cond, __gthread_mutex_t *mutex)
{
	cnd_wait(cond, mutex);
}

static inline
int __gthread_cond_wait_recursive(__gthread_cond_t *cond, __gthread_recursive_mutex_t *mutex)
{
	cnd_wait(cond, mutex);
}

static inline
int __gthread_cond_timedwait(__gthread_cond_t *cond, __gthread_mutex_t *mutex, const __gthread_time_t *abs_timeout)
{
	cnd_waitUntil(cond, mutex, *abs_timeout);
}

static inline
int __gthread_create(__gthread_t *thread, void (*func)(void *), void *args)
{
	ostask_t *task = static_cast<ostask_t *>(wrk_create(OS_MAIN_PRIO, ostask_t::handler, OS_STACK_SIZE, false, false));
	if (task != nullptr)
	{
		task->fun = func;
		task->arg = args;
		tsk_start(task);
		*thread = task;
	}
	return task != nullptr ? 0 : 1;
}

static inline
int __gthread_join(__gthread_t thread, void **/*value_ptr*/)
{
	return tsk_join(thread);
}

static inline
int __gthread_detach(__gthread_t thread)
{
	return tsk_detach(thread);
}

static inline
int __gthread_equal(const __gthread_t t1, const __gthread_t t2)
{
	return t1 == t2 ? 0 : 1;
}

static inline
__gthread_t __gthread_self()
{
	return reinterpret_cast<ostask_t *>(tsk_this());
}

static inline
int __gthread_yield()
{
	tsk_yield();
	return 0;
}

//-----------------------------------------------------------------------------

#include "inc/chrono.hpp"
#include "inc/critical_section.hpp"
#include "inc/semaphore.hpp"

//-----------------------------------------------------------------------------

#endif //_GLIBCXX_GCC_GTHR_STATEOS_H
