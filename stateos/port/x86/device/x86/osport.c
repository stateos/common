/******************************************************************************

    @file    StateOS: osport.c
    @author  Rajmund Szymanski
    @date    14.10.2026
    @brief   StateOS port file for X86.

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

#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L // clock_gettime, nanosleep
#endif

#include "oskernel.h"
#include "inc/ostimer.h"

#if defined(_WIN32)
// <windows.h> is not included because of conflicting definitions (INFINITE)
__declspec(dllimport) int  __stdcall QueryPerformanceFrequency( int64_t *frq );
__declspec(dllimport) int  __stdcall QueryPerformanceCounter( int64_t *cnt );
__declspec(dllimport) void __stdcall Sleep( unsigned long ms );
__declspec(dllimport) void*__stdcall CreateWaitableTimerExW( void *attr, const void *name, unsigned long flags, unsigned long access );
__declspec(dllimport) int  __stdcall SetWaitableTimer( void *timer, const int64_t *due, long period, void *proc, void *arg, int resume );
__declspec(dllimport) unsigned long __stdcall WaitForSingleObject( void *handle, unsigned long ms );
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002UL
#define TIMER_ALL_ACCESS                      0x001F0003UL
#else
#include <time.h>
#endif

/* -------------------------------------------------------------------------- */

#define NSEC 1000000000ULL

#if defined(_WIN32)
#define WAKE  500000ULL // margin for the wake-up latency of the host (ns)
#else
#define WAKE  100000ULL // margin for the wake-up latency of the host (ns)
#endif

lck_t port_lck = 0;
bool  port_isr = false;
bool  port_ctx_pending = false;

/* -------------------------------------------------------------------------- */
// return time elapsed since the first call (in nanoseconds)

#if defined(_WIN32)

static
uint64_t priv_sys_nsec( void )
{
	static
	int64_t frq = 0;
	static
	int64_t ref = 0;
	int64_t cnt;
	uint64_t t;

	if (frq == 0)
	{
		QueryPerformanceFrequency(&frq);
		QueryPerformanceCounter(&ref);
	}

	QueryPerformanceCounter(&cnt);
	t = (uint64_t)(cnt - ref);

	return t / (uint64_t)frq * NSEC + t % (uint64_t)frq * NSEC / (uint64_t)frq;
}

// Sleep has the resolution of the system timer (15.6 ms by default),
// the high resolution waitable timer is available since Windows 10 1803

static
void priv_sys_sleep( uint64_t nsec )
{
	static
	void * timer = NULL;
	static
	bool   init = false;
	int64_t due = -(int64_t)(nsec / 100); // relative time in 100 ns units

	if (!init)
	{
		init = true;
		timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
	}

	if (timer != NULL && SetWaitableTimer(timer, &due, 0, NULL, NULL, 0))
		WaitForSingleObject(timer, 0xFFFFFFFFUL);
	else
		Sleep((unsigned long)(nsec / 1000000));
}

#else

static
uint64_t priv_sys_nsec( void )
{
	static
	struct timespec ref = { 0, 0 };
	struct timespec now;

	if (ref.tv_sec == 0 && ref.tv_nsec == 0)
		clock_gettime(CLOCK_MONOTONIC, &ref);

	clock_gettime(CLOCK_MONOTONIC, &now);

	return (uint64_t)(now.tv_sec - ref.tv_sec) * NSEC + (uint64_t)now.tv_nsec - (uint64_t)ref.tv_nsec;
}

static
void priv_sys_sleep( uint64_t nsec )
{
	struct timespec ts = { (time_t)(nsec / NSEC), (long)(nsec % NSEC) };
	nanosleep(&ts, NULL);
}

#endif

/* -------------------------------------------------------------------------- */

uint64_t port_sys_clock( void )
{
	uint64_t ns = priv_sys_nsec();

	return ns / NSEC * (OS_FREQUENCY) + ns % NSEC * (OS_FREQUENCY) / NSEC;
}

/* -------------------------------------------------------------------------- */

void port_sys_init( void )
{
	(void) priv_sys_nsec(); // start the host clock
}

/* -------------------------------------------------------------------------- */

#if HW_TIMER_SIZE == 0

/******************************************************************************
 Non-tick-less mode: number of the handled ticks of the system timer
*******************************************************************************/

static
uint64_t priv_sys_tick = 0;

#endif

/******************************************************************************
 Virtual interrupt controller: handler of the system timer and context switch
*******************************************************************************/

void port_sys_poll( void )
{
	port_isr = true;
	{
	#if HW_TIMER_SIZE == 0
		uint64_t clk = port_sys_clock();

		while (priv_sys_tick < clk)
		{
			priv_sys_tick++;
			core_sys_tick();
		}
	#else
		core_tmr_handler();
	#endif
	}
	port_isr = false;

	if (port_ctx_pending)
	{
		port_ctx_pending = false;
		port_ctx_swap();
	}
}

/******************************************************************************
 Idle task: wait for the nearest time breakpoint (no longer than 100 ms)
 The host sleeps until the breakpoint minus the wake-up latency margin,
 the rest of the time is spent on polling the clock
 Called with the virtual interrupts masked, they are handled after port_clr_lock
*******************************************************************************/

void port_sys_wait( void )
{
	tmr_t  * tmr = WAIT.hdr.next;
	uint64_t now = priv_sys_nsec();
	uint64_t end = now + NSEC / 10;

	if (tmr->delay != INFINITE)
	{
	#if HW_TIMER_SIZE == 0
		uint64_t clk = priv_sys_tick;
		cnt_t    cnt = System.cnt;
	#else
		uint64_t clk = port_sys_clock();
		cnt_t    cnt = (cnt_t) clk;
	#endif
		cnt_t    elapsed = (cnt_t)(cnt - tmr->start);
		uint64_t tick;
		uint64_t nsec;

		if (elapsed >= tmr->delay)
			end = now;
		else
		if ((uint64_t)(tmr->delay - elapsed) < (OS_FREQUENCY) / 10)
		{
			tick = clk + (tmr->delay - elapsed);
			nsec = tick / (OS_FREQUENCY) * NSEC + (tick % (OS_FREQUENCY) * NSEC + (OS_FREQUENCY) - 1) / (OS_FREQUENCY);
			if (nsec < end)
				end = nsec;
		}
	}

	if (end > now + WAKE)
		priv_sys_sleep(end - now - WAKE);
}

/******************************************************************************
 End of the functions
*******************************************************************************/

/* -------------------------------------------------------------------------- */
