/******************************************************************************

    @file    StateOS: osflag.h
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

#ifndef __STATEOS_FLG_H
#define __STATEOS_FLG_H

#include "oskernel.h"
#include "osclock.h"

/* -------------------------------------------------------------------------- */

#define flgAny          0U
#define flgAll          1U
#define flgProtect      2U
#define flgIgnore       4U
#define flgMASK         7U

#define flgAnyProtect ( flgAny | flgProtect )
#define flgAllProtect ( flgAll | flgProtect )
#define flgAnyIgnore  ( flgAny | flgIgnore  )
#define flgAllIgnore  ( flgAll | flgIgnore  )

/******************************************************************************
 *
 * Name              : flag
 *
 ******************************************************************************/

typedef struct __flg flg_t;

struct __flg
{
	obj_t    obj;   // object header

	unsigned flags; // pending flags
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

/******************************************************************************
 *
 * Name              : OS_FLG
 * Static alias      : static_FLG
 *
 * Description       : define and initialize a flag object
 *
 * Parameters
 *   flg             : name of a pointer to flag object
 *   init            : (optional) initial value of flag; default: 0
 *
 ******************************************************************************/

#define             OS_FLG( flg, ... ) \
                       flg_t flg[] = { _FLG_INIT( _VA_FLG(__VA_ARGS__) ) }

#define         static_FLG( flg, ... ) \
                static flg_t flg[] = { _FLG_INIT( _VA_FLG(__VA_ARGS__) ) }

/******************************************************************************
 *
 * Name              : FLG_INIT
 *
 * Description       : create and initialize a flag object
 *
 * Parameters
 *   init            : (optional) initial value of flag; default: 0
 *
 * Return            : flag object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                FLG_INIT( ... ) \
                      _FLG_INIT( _VA_FLG(__VA_ARGS__) )
#endif

/******************************************************************************
 *
 * Name              : FLG_CREATE
 * Alias             : FLG_NEW
 *
 * Description       : create and initialize a flag object
 *
 * Parameters
 *   init            : (optional) initial value of flag; default: 0
 *
 * Return            : flag object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                FLG_CREATE( ... ) \
                     { FLG_INIT  ( _VA_FLG(__VA_ARGS__) ) }
#define                FLG_NEW \
                       FLG_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : flg_init
 *
 * Description       : initialize a flag object
 *
 * Parameters
 *   flg             : pointer to flag object
 *   init            : initial value of flag
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void flg_init( flg_t *flg, unsigned init );

/******************************************************************************
 *
 * Name              : flg_create
 * Alias             : flg_new
 *
 * Description       : create and initialize a new flag object
 *
 * Parameters
 *   init            : initial value of flag
 *
 * Return            : pointer to flag object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

flg_t *flg_create( unsigned init );

__STATIC_INLINE
flg_t *flg_new( unsigned init ) { return flg_create(init); }

/******************************************************************************
 *
 * Name              : flg_reset
 * Alias             : flg_kill
 *
 * Description       : reset the flag object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   flg             : pointer to flag object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void flg_reset( flg_t *flg );

__STATIC_INLINE
void flg_kill( flg_t *flg ) { flg_reset(flg); }

/******************************************************************************
 *
 * Name              : flg_destroy
 * Alias             : flg_delete
 *
 * Description       : reset the flag object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * Parameters
 *   flg             : pointer to flag object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void flg_destroy( flg_t *flg );

__STATIC_INLINE
void flg_delete( flg_t *flg ) { flg_destroy(flg); }

/******************************************************************************
 *
 * Name              : flg_take
 * Alias             : flg_tryWait
 * ISR alias         : flg_takeISR
 *
 * Description       : check if required flags have been set in the flag object
 *
 * Parameters
 *   flg             : pointer to flag object
 *   flags           : all flags to wait
 *   mode            : waiting mode
 *                     flgAny:     wait for any flags to be set
 *                     flgAll:     wait for all flags to be set
 *                     flgProtect: don't clear flags in flag object
 *                     flgIgnore:  ignore flags in flag object that have been set and not accepted before
 *                     ( either flgAny or flgAll can be OR'ed with flgProtect or flgIgnore )
 *
 * Return
 *   0               : required flags have been set
 *   another value   : remaining flags, try again
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

unsigned flg_take( flg_t *flg, unsigned flags, unsigned mode );

__STATIC_INLINE
unsigned flg_tryWait( flg_t *flg, unsigned flags, unsigned mode ) { return flg_take(flg, flags, mode); }

__STATIC_INLINE
unsigned flg_takeISR( flg_t *flg, unsigned flags, unsigned mode ) { return flg_take(flg, flags, mode); }

/******************************************************************************
 *
 * Name              : flg_waitFor
 *
 * Description       : wait on flag object for given flags for given duration of time
 *
 * Parameters
 *   flg             : pointer to flag object
 *   flags           : all flags to wait
 *   mode            : waiting mode
 *                     flgAny:     wait for any flags to be set
 *                     flgAll:     wait for all flags to be set
 *                     flgProtect: don't clear flags in flag object
 *                     flgIgnore:  ignore flags in flag object that have been set and not accepted before
 *                     ( either flgAny or flgAll can be OR'ed with flgProtect or flgIgnore )
 *   delay           : duration of time (maximum number of ticks to wait on flag object for given flags)
 *                     IMMEDIATE: don't wait until required flags have been set
 *                     INFINITE:  wait indefinitely until required flags have been set
 *
 * Return
 *   E_SUCCESS       : required flags have been set
 *   E_STOPPED       : flag object was reseted before the specified timeout expired
 *   E_DELETED       : flag object was deleted before the specified timeout expired
 *   E_TIMEOUT       : required flags have not been set before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int flg_waitFor( flg_t *flg, unsigned flags, unsigned mode, cnt_t delay );

/******************************************************************************
 *
 * Name              : flg_waitUntil
 *
 * Description       : wait on flag object for given flags until given timepoint
 *
 * Parameters
 *   flg             : pointer to flag object
 *   flags           : all flags to wait
 *   mode            : waiting mode
 *                     flgAny:     wait for any flags to be set
 *                     flgAll:     wait for all flags to be set
 *                     flgProtect: don't clear flags in flag object
 *                     flgIgnore:  ignore flags in flag object that have been set and not accepted before
 *                     ( either flgAny or flgAll can be OR'ed with flgProtect or flgIgnore )
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : required flags have been set
 *   E_STOPPED       : flag object was reseted before the specified timeout expired
 *   E_DELETED       : flag object was deleted before the specified timeout expired
 *   E_TIMEOUT       : required flags have not been set before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int flg_waitUntil( flg_t *flg, unsigned flags, unsigned mode, cnt_t time );

/******************************************************************************
 *
 * Name              : flg_wait
 *
 * Description       : wait indefinitely on flag object until required flags have been set
 *
 * Parameters
 *   flg             : pointer to flag object
 *   flags           : all flags to wait
 *   mode            : waiting mode
 *                     flgAny:     wait for any flags to be set
 *                     flgAll:     wait for all flags to be set
 *                     flgProtect: don't clear flags in flag object
 *                     flgIgnore:  ignore flags in flag object that have been set and not accepted before
 *                     ( either flgAny or flgAll can be OR'ed with flgProtect or flgIgnore )
 *
 * Return
 *   E_SUCCESS       : required flags have been set
 *   E_STOPPED       : flag object was reseted
 *   E_DELETED       : flag object was deleted
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
int flg_wait( flg_t *flg, unsigned flags, unsigned mode ) { return flg_waitFor(flg, flags, mode, INFINITE); }

/******************************************************************************
 *
 * Name              : flg_give
 * Alias             : flg_set
 * ISR alias         : flg_giveISR
 *
 * Description       : set given flags in flag object
 *
 * Parameters
 *   flg             : pointer to flag object
 *   flags           : all flags to set
 *
 * Return            : flags after setting
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

unsigned flg_give( flg_t *flg, unsigned flags );

__STATIC_INLINE
unsigned flg_set( flg_t *flg, unsigned flags ) { return flg_give(flg, flags); }

__STATIC_INLINE
unsigned flg_giveISR( flg_t *flg, unsigned flags ) { return flg_give(flg, flags); }

/******************************************************************************
 *
 * Name              : flg_clear
 * ISR alias         : flg_clearISR
 *
 * Description       : clear given flags in flag object
 *
 * Parameters
 *   flg             : pointer to flag object
 *   flags           : all flags to clear
 *
 * Return            : flags before clearing
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

unsigned flg_clear( flg_t *flg, unsigned flags );

__STATIC_INLINE
unsigned flg_clearISR( flg_t *flg, unsigned flags ) { return flg_clear(flg, flags); }

/******************************************************************************
 *
 * Name              : flg_get
 * ISR alias         : flg_getISR
 *
 * Description       : get current flags from flag object
 *
 * Parameters
 *   flg             : pointer to flag object
 *
 * Return            : current flags in flag object
 *
 * Note              : can be used in both thread and handler mode (for blockable interrupts)
 *                     use ISR alias in blockable interrupt handlers
 *
 ******************************************************************************/

unsigned flg_get( flg_t *flg );

__STATIC_INLINE
unsigned flg_getISR( flg_t *flg ) { return flg_get(flg); }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)
namespace stateos {

/******************************************************************************
 *
 * Class             : Flag
 *
 * Description       : create and initialize a flag object
 *
 * Constructor parameters
 *   init            : initial value of flag
 *
 ******************************************************************************/

struct Flag : public __flg
{
	constexpr
	Flag( const unsigned _init = 0 ): __flg _FLG_INIT(_init) {}

	~Flag() { assert(__flg::obj.queue == nullptr); }

	Flag( Flag&& ) = default;
	Flag( const Flag& ) = delete;
	Flag& operator=( Flag&& ) = delete;
	Flag& operator=( const Flag& ) = delete;

	void     reset    ()                                               {        flg_reset    (this); }
	void     kill     ()                                               {        flg_kill     (this); }
	void     destroy  ()                                               {        flg_destroy  (this); }
	unsigned take     ( unsigned _flags, char _mode = flgAll )         { return flg_take     (this, _flags, _mode); }
	unsigned tryWait  ( unsigned _flags, char _mode = flgAll )         { return flg_tryWait  (this, _flags, _mode); }
	unsigned takeISR  ( unsigned _flags, char _mode = flgAll )         { return flg_takeISR  (this, _flags, _mode); }
	template<typename T>
	int      waitFor  ( unsigned _flags, char _mode, const T& _delay ) { return flg_waitFor  (this, _flags, _mode, Clock::count(_delay)); }
	template<typename T>
	int      waitUntil( unsigned _flags, char _mode, const T& _time )  { return flg_waitUntil(this, _flags, _mode, Clock::until(_time)); }
	int      wait     ( unsigned _flags, char _mode = flgAll )         { return flg_wait     (this, _flags, _mode); }
	unsigned give     ( unsigned _flags )                              { return flg_give     (this, _flags); }
	unsigned set      ( unsigned _flags )                              { return flg_set      (this, _flags); }
	unsigned giveISR  ( unsigned _flags )                              { return flg_giveISR  (this, _flags); }
	unsigned clear    ( unsigned _flags )                              { return flg_clear    (this, _flags); }
	unsigned clearISR ( unsigned _flags )                              { return flg_clearISR (this, _flags); }
	unsigned get      ()                                               { return flg_get      (this); }
	unsigned getISR   ()                                               { return flg_getISR   (this); }

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<Flag>;
#else
	using Ptr = Flag *;
#endif

/******************************************************************************
 *
 * Name              : Flag::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters
 *   init            : initial value of flag
 *
 * Return            : std::unique_pointer / pointer to Flag object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create( const unsigned _init = 0 )
	{
		auto flg = new (std::nothrow) Flag(_init);
		if (flg != nullptr)
			flg->__flg::obj.res = flg;
		return Ptr(flg);
	}

};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__STATEOS_FLG_H
