# StateOS API

StateOS is free, extremely simple and very fast real-time operating system (RTOS) designed for deeply embedded applications.
The API is available in C (functions, macros and types described in the topics below) and in C++ (classes in the namespace stateos, e.g. stateos::Semaphore).

## Topics

- system: @ref basic, @ref sys, @ref alloc, @ref clock, @ref criticalsection
- synchronization: @ref spinlock, @ref onceflag, @ref event, @ref signal, @ref flag, @ref barrier, @ref semaphore, @ref mutex, @ref conditionvariable, @ref rwlock
- communication: @ref list, @ref memorypool, @ref rawbuffer, @ref messagequeue, @ref mailboxqueue, @ref eventqueue, @ref jobqueue
- tasks: @ref timer, @ref task, @ref statemachine

## Conventions

- every object has its own prefix, e.g. semaphore: type sem_t, functions sem_*, macros OS_SEM, static_SEM, SEM_INIT and SEM_CREATE
- waitFor functions wait for given duration of time, waitUntil functions until given time point, wait functions indefinitely; time is counted in ticks of the system timer (USEC, MSEC, SEC, ...)
- the functions return the error codes described in @ref basic (E_SUCCESS, E_TIMEOUT, ...)
- ISR aliases are intended for blockable interrupt handlers, Async aliases for communication with unmasked interrupt handlers

Sources, examples and templates: https://github.com/stateos
