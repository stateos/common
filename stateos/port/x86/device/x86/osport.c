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
#define _POSIX_C_SOURCE 200809L // clock_gettime, nanosleep, sigaction, timer_create
#endif

#include "oskernel.h"
#include "inc/ostimer.h"
#if OS_ROBIN
#include <errno.h>
#endif

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
#if OS_ROBIN
__declspec(dllimport) void*__stdcall GetCurrentProcess( void );
__declspec(dllimport) void*__stdcall GetCurrentThread( void );
__declspec(dllimport) int  __stdcall DuplicateHandle( void *sproc, void *src, void *tproc, void **dst, unsigned long access, int inherit, unsigned long options );
__declspec(dllimport) void*__stdcall CreateThread( void *attr, size_t stack, unsigned long (__stdcall *proc)( void * ), void *arg, unsigned long flags, unsigned long *id );
__declspec(dllimport) int  __stdcall SetThreadPriority( void *thread, int prio );
__declspec(dllimport) int  __stdcall QueueUserAPC2( void (__stdcall *proc)( uintptr_t ), void *thread, uintptr_t arg, int flags );
__declspec(dllimport) unsigned long __stdcall GetLastError( void );
__declspec(dllimport) void __stdcall SetLastError( unsigned long err );
#define DUPLICATE_SAME_ACCESS                 0x00000002UL
#define THREAD_PRIORITY_TIME_CRITICAL         15
#define QUEUE_USER_APC_FLAGS_SPECIAL_USER_APC 0x00000001
#endif
#else
#include <time.h>
#if OS_ROBIN
#include <signal.h>
#endif
#endif

/* -------------------------------------------------------------------------- */

#define NSEC 1000000000ULL

#if defined(_WIN32)
#define WAKE  500000ULL // margin for the wake-up latency of the host (ns)
#else
#define WAKE  100000ULL // margin for the wake-up latency of the host (ns)
#endif

#define TMIN   10000ULL // minimum time of the host timer (ns): no interrupt during the return from its handler

volatile lck_t port_lck = 0;
volatile bool  port_isr = false;
volatile bool  port_irq = false;
volatile bool  port_ctx_pending = false;

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
// return the time of the beginning of the tick (in nanoseconds)

static
uint64_t priv_sys_nsecOf( uint64_t tick )
{
	return tick / (OS_FREQUENCY) * NSEC + (tick % (OS_FREQUENCY) * NSEC + (OS_FREQUENCY) - 1) / (OS_FREQUENCY);
}

/* -------------------------------------------------------------------------- */

#if OS_ROBIN

/******************************************************************************
 Preemptive mode: the host timer interrupts the running task
 Tick mode: periodic, with the frequency OS_FREQUENCY
 Tick-less mode: one-shot, set to the nearest time breakpoint or the end of the time slice
*******************************************************************************/

#if HW_TIMER_SIZE

static
uint64_t priv_slice = UINT64_MAX; // end of the time slice

static volatile
uint64_t priv_due = UINT64_MAX;   // expiration time of the host timer (UINT64_MAX: expired or not set)

#endif

static
void priv_tmr_interrupt( void )
{
	#if HW_TIMER_SIZE
	priv_due = UINT64_MAX; // expired (on windows maybe too early): priv_tmr_update sets the timer again
	#endif
	if (port_lck || port_isr)
		port_irq = true; // the interrupt is handled when the virtual interrupts are unmasked
	else
		port_sys_poll();
}

#if defined(_WIN32)

// the timer thread interrupts the system thread with a special user APC (windows 11)

static void *priv_main;           // thread of the system
static void *priv_timer;          // waitable timer
static volatile bool priv_queued; // the APC is queued

static
void __stdcall priv_tmr_apc( uintptr_t arg )
{
	unsigned long error = GetLastError();
	int err = errno;
	(void) arg;
	priv_queued = false;
	priv_tmr_interrupt();
	errno = err;
	SetLastError(error);
}

static
unsigned long __stdcall priv_tmr_thread( void *arg )
{
	(void) arg;
	for (;;)
	{
		WaitForSingleObject(priv_timer, 0xFFFFFFFFUL);
		if (!priv_queued)
		{
			priv_queued = true;
			QueueUserAPC2(priv_tmr_apc, priv_main, 0, QUEUE_USER_APC_FLAGS_SPECIAL_USER_APC);
		}
	}
	return 0;
}

static
void priv_tmr_init( void )
{
	void *thread;

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &priv_main, 0, 0, DUPLICATE_SAME_ACCESS);
	priv_timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
	thread = CreateThread(NULL, 0, priv_tmr_thread, NULL, 0, NULL);
	SetThreadPriority(thread, THREAD_PRIORITY_TIME_CRITICAL);
}

// expire after nsec nanoseconds; periodic: period in milliseconds (the resolution of the host)
static
void priv_tmr_set( uint64_t nsec, bool periodic )
{
	int64_t due = -(int64_t)(nsec / 100 + 1);
	long period = periodic ? (long)((nsec + 999999) / 1000000) : 0;

	SetWaitableTimer(priv_timer, &due, period, NULL, NULL, 0);
}

#else

// the posix timer interrupts the system thread with the signal SIGALRM

static timer_t priv_timer;

static
void priv_tmr_handler( int signo )
{
	int err = errno;
	(void) signo;
	priv_tmr_interrupt();
	errno = err;
}

static
void priv_tmr_init( void )
{
	struct sigaction sa = { 0 };
	struct sigevent  se = { 0 };

	sa.sa_handler = priv_tmr_handler;
	sa.sa_flags   = SA_NODEFER | SA_RESTART; // the handler can switch the task: the signal stays unmasked
	sigemptyset(&sa.sa_mask);
	sigaction(SIGALRM, &sa, NULL);

	se.sigev_notify = SIGEV_SIGNAL;
	se.sigev_signo  = SIGALRM;
	timer_create(CLOCK_MONOTONIC, &se, &priv_timer);
}

// expire after nsec nanoseconds; periodic: with the period nsec
static
void priv_tmr_set( uint64_t nsec, bool periodic )
{
	struct itimerspec it = { { 0, 0 }, { (time_t)(nsec / NSEC), (long)(nsec % NSEC) } };

	if (periodic)
		it.it_interval = it.it_value;

	timer_settime(priv_timer, 0, &it, NULL);
}

#endif

#if HW_TIMER_SIZE

/******************************************************************************
 Tick-less mode: new time slice, setting of the host timer
*******************************************************************************/

void port_ctx_reset( void )
{
	priv_slice = port_sys_clock() + (OS_FREQUENCY)/(OS_ROBIN);
}

// set the host timer to the beginning of the tick of the nearest time breakpoint or the end of the time slice;
// also after each expiry with the same time: the timer could have expired too early

static
void priv_tmr_update( void )
{
	tmr_t  * tmr = WAIT.hdr.next;
	uint64_t clk = port_sys_clock();
	uint64_t due = priv_slice;
	uint64_t now, end;

	if (tmr->delay != INFINITE)
	{
		cnt_t elapsed = (cnt_t)((cnt_t) clk - tmr->start);
		end = clk + (elapsed < tmr->delay ? (cnt_t)(tmr->delay - elapsed) : 0);
		if (end < due)
			due = end;
	}

	if (due != priv_due)
	{
		now = priv_sys_nsec();
		end = priv_sys_nsecOf(due);
		if (end <= now)                  // the time has come: handled in the next round of port_sys_poll
			port_irq = true;
		else
		{
			priv_due = due;
			priv_tmr_set(end - now > TMIN ? end - now : TMIN, false);
		}
	}
}

#endif

#endif//OS_ROBIN

/* -------------------------------------------------------------------------- */

void port_sys_init( void )
{
	(void) priv_sys_nsec(); // start the host clock
#if OS_ROBIN
	priv_tmr_init();
	#if HW_TIMER_SIZE == 0
	priv_tmr_set(NSEC / (OS_FREQUENCY), true);
	#endif
#endif
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
 The virtual interrupts stay masked (port_isr) during the context switch,
 they are unmasked by the next task (here or in port_ctx_enter)
 Preemptive mode: repeated, if the host timer interrupted the handler
*******************************************************************************/

void port_sys_poll( void )
{
	do
	{
		port_isr = true;
		port_irq = false;
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
			#if OS_ROBIN
			if (port_sys_clock() >= priv_slice)
				core_ctx_switch();       // end of the time slice
			#endif
		#endif
		}

		if (port_ctx_pending)
		{
			port_ctx_pending = false;
			port_ctx_swap();
		}

		#if OS_ROBIN && HW_TIMER_SIZE
		priv_tmr_update();
		#endif
		port_isr = false;
	}
	while (port_irq);
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
			nsec = priv_sys_nsecOf(tick);
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
