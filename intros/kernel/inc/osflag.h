/******************************************************************************

    @file    IntrOS: osflag.h
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

#ifndef __INTROS_FLG_H
#define __INTROS_FLG_H

#include "oskernel.h"
#include "osclock.h"

/**
 * @defgroup flag Flag
 * @brief    flags (any, all, new)
 * @{
 */

/* -------------------------------------------------------------------------- */

#define flgAny          0U ///< wait for any flags to be set
#define flgAll          1U ///< wait for all flags to be set
#define flgNew          2U ///< ignore flags in flag object that have been set before

#define flgAnyNew     ( flgAny | flgNew  ) ///< flgAny | flgNew
#define flgAllNew     ( flgAll | flgNew  ) ///< flgAll | flgNew

/**
 * @brief   flag
 */

typedef struct __flg flg_t;

struct __flg
{
	obj_t    obj;   ///< object header

	unsigned flags; ///< pending flags
};

typedef struct __flg flg_id [];

/******************************************************************************
 *
 * Name              : _FLG_INIT
 *
 * Description       : create and initialize a flag object
 *
 * Parameters
 *   init            : initial value of flag
 *
 * Return            : flag object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _FLG_INIT( _init ) { _OBJ_INIT(), _init }

/******************************************************************************
 *
 * Name              : _VA_FLG
 *
 * Description       : calculate initial value of flag from optional parameter
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _VA_FLG( _init ) ( _init + 0 )

/**
 * @brief   define and initialize a flag object
 *
 * @param   flg        name of a pointer to flag object
 * @param   ...        init: (optional) initial value of flag; default: 0
 */

#define             OS_FLG( flg, ... ) \
                       flg_t flg[] = { _FLG_INIT( _VA_FLG(__VA_ARGS__) ) }

/** static alias of OS_FLG() */
#define         static_FLG( flg, ... ) \
                static flg_t flg[] = { _FLG_INIT( _VA_FLG(__VA_ARGS__) ) }

/**
 * @brief   create and initialize a flag object
 *
 * @param   ...        init: (optional) initial value of flag; default: 0
 *
 * @return  flag object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                FLG_INIT( ... ) \
                      _FLG_INIT( _VA_FLG(__VA_ARGS__) )
#endif

/**
 * @brief   create and initialize a flag object
 *
 * @param   ...        init: (optional) initial value of flag; default: 0
 *
 * @return  flag object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                FLG_CREATE( ... ) \
                     { FLG_INIT  ( _VA_FLG(__VA_ARGS__) ) }
/** alias of FLG_CREATE() */
#define                FLG_NEW \
                       FLG_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize a flag object
 *
 * @param   flg        pointer to flag object
 * @param   init       initial value of flag
 */

void flg_init( flg_t *flg, unsigned init );

/**
 * @brief   reset the flag object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * @param   flg        pointer to flag object
 *
 * @note    use only in thread mode
 */

void flg_reset( flg_t *flg );

/** alias of flg_reset() */
__STATIC_INLINE
void flg_kill( flg_t *flg ) { flg_reset(flg); }

/**
 * @brief   check if required flags have been set in the flag object
 *
 * @param   flg        pointer to flag object
 * @param   flags      all flags to wait
 * @param   mode       waiting mode
 *                     - flgAny: wait for any flags to be set
 *                     - flgAll: wait for all flags to be set
 *                     - flgNew: ignore flags in flag object that have been set before
 *                     ( either flgAny or flgAll can be used with flgNew )
 *
 * @retval  0              required flags have been set
 * @retval  another value  remaining flags, try again
 */

unsigned flg_take( flg_t *flg, unsigned flags, unsigned mode );

/** alias of flg_take() */
__STATIC_INLINE
unsigned flg_tryWait( flg_t *flg, unsigned flags, unsigned mode ) { return flg_take(flg, flags, mode); }

/**
 * @brief   wait on flag object for given flags for given duration of time
 *
 * @param   flg        pointer to flag object
 * @param   flags      all flags to wait
 * @param   mode       waiting mode
 *                     - flgAny: wait for any flags to be set
 *                     - flgAll: wait for all flags to be set
 *                     - flgNew: ignore flags in flag object that have been set before
 *                     ( either flgAny or flgAll can be used with flgNew )
 * @param   delay      duration of time (maximum number of ticks to wait on flag object for given flags)
 *                     - IMMEDIATE: don't wait until required flags have been set
 *                     - INFINITE:  wait indefinitely until required flags have been set
 *
 * @retval  E_SUCCESS  required flags have been set
 * @retval  E_STOPPED  flag object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  required flags have not been set before the specified timeout expired
 */

int flg_waitFor( flg_t *flg, unsigned flags, unsigned mode, cnt_t delay );

/**
 * @brief   wait on flag object for given flags until given timepoint
 *
 * @param   flg        pointer to flag object
 * @param   flags      all flags to wait
 * @param   mode       waiting mode
 *                     - flgAny: wait for any flags to be set
 *                     - flgAll: wait for all flags to be set
 *                     - flgNew: ignore flags in flag object that have been set before
 *                     ( either flgAny or flgAll can be used with flgNew )
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  required flags have been set
 * @retval  E_STOPPED  flag object was reseted before the specified timeout expired
 * @retval  E_TIMEOUT  required flags have not been set before the specified timeout expired
 */

int flg_waitUntil( flg_t *flg, unsigned flags, unsigned mode, cnt_t time );

/**
 * @brief   wait indefinitely on flag object until required flags have been set
 *
 * @param   flg        pointer to flag object
 * @param   flags      all flags to wait
 * @param   mode       waiting mode
 *                     - flgAny: wait for any flags to be set
 *                     - flgAll: wait for all flags to be set
 *                     - flgNew: ignore flags in flag object that have been set before
 *                     ( either flgAny or flgAll can be used with flgNew )
 *
 * @retval  E_SUCCESS  required flags have been set
 * @retval  E_STOPPED  flag object was reseted
 */

__STATIC_INLINE
int flg_wait( flg_t *flg, unsigned flags, unsigned mode ) { return flg_waitFor(flg, flags, mode, INFINITE); }

/**
 * @brief   set given flags in flag object
 *
 * @param   flg        pointer to flag object
 * @param   flags      all flags to set
 *
 * @return  flags after setting
 */

unsigned flg_give( flg_t *flg, unsigned flags );

/** alias of flg_give() */
__STATIC_INLINE
unsigned flg_set( flg_t *flg, unsigned flags ) { return flg_give(flg, flags); }

/**
 * @brief   clear given flags in flag object
 *
 * @param   flg        pointer to flag object
 * @param   flags      all flags to clear
 *
 * @return  flags before clearing
 */

unsigned flg_clear( flg_t *flg, unsigned flags );

/**
 * @brief   get current flags from flag object
 *
 * @param   flg        pointer to flag object
 *
 * @return  current flags in flag object
 */

unsigned flg_get( flg_t *flg );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus)
namespace intros {

/**
 * @brief   create and initialize a flag object
 *
 * @par     Constructor parameters
 *          - init: initial value of flag
 */

struct Flag : public __flg
{
	/// constructor, see the constructor parameters of the class
	constexpr
	Flag( const unsigned _init = 0 ): __flg _FLG_INIT(_init) {}

	~Flag() { assert(__flg::obj.queue == nullptr); }

	/// move constructor
	Flag( Flag&& _src ): __flg(_src)
	{
		assert(_src.obj.queue == nullptr);
	}

	Flag( const Flag& ) = delete;
	Flag& operator=( Flag&& ) = delete;
	Flag& operator=( const Flag& ) = delete;

	void     reset    ()                                                   {        flg_reset    (this); }                                      ///< flg_reset()
	void     kill     ()                                                   {        flg_kill     (this); }                                      ///< flg_kill()
	unsigned take     ( unsigned _flags, unsigned _mode = flgAll )         { return flg_take     (this, _flags, _mode); }                       ///< flg_take()
	unsigned tryWait  ( unsigned _flags, unsigned _mode = flgAll )         { return flg_tryWait  (this, _flags, _mode); }                       ///< flg_tryWait()
	template<typename T>
	int      waitFor  ( unsigned _flags, unsigned _mode, const T& _delay ) { return flg_waitFor  (this, _flags, _mode, Clock::count(_delay)); } ///< flg_waitFor()
	template<typename T>
	int      waitUntil( unsigned _flags, unsigned _mode, const T& _time )  { return flg_waitUntil(this, _flags, _mode, Clock::until(_time)); }  ///< flg_waitUntil()
	int      wait     ( unsigned _flags, unsigned _mode = flgAll )         { return flg_wait     (this, _flags, _mode); }                       ///< flg_wait()
	unsigned give     ( unsigned _flags )                                  { return flg_give     (this, _flags); }                              ///< flg_give()
	unsigned set      ( unsigned _flags )                                  { return flg_set      (this, _flags); }                              ///< flg_set()
	unsigned clear    ( unsigned _flags )                                  { return flg_clear    (this, _flags); }                              ///< flg_clear()
	unsigned get      ()                                                   { return flg_get      (this); }                                      ///< flg_get()
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

/** @} */

#endif//__INTROS_FLG_H
