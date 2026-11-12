# Porting StateOS {#porting}

The kernel does not depend on the hardware: everything that depends on the processor, the compiler or the device is in the port layer (`port` directory).
This page describes what a port must provide; the existing ports are the best reference:

| port                         | description                                                                         |
|------------------------------|-------------------------------------------------------------------------------------|
| `port/cortexm`               | ARM Cortex-M (CMSIS), compilers armcc, clang, csmcc, gnucc, iarcc, devices STM32    |
| `port/stm8`                  | STM8 (8-bit), compilers csmc, iarc, sdcc                                            |
| `port/x86`                   | PC (Linux, Windows): the system runs in a process of the host, for testing purposes |

## Files

A port consists of the files common to the processor and compiler (`compiler`) and the files specific to the device (`device`):

| file                                  | contents                                                                                |
|---------------------------------------|-----------------------------------------------------------------------------------------|
| `port/<arch>/compiler/<cc>/osdefs.h`  | definitions of the compiler (see Compiler definitions)                                     |
| `port/<arch>/compiler/<cc>/oscore.h`  | types, context of the task, critical section, inline functions of the core              |
| `port/<arch>/compiler/<cc>/oscore.c`  | context switch handler, `core_tsk_flip` (or `oscore.s` in assembler)                    |
| `port/<arch>/compiler/<cc>/oslibc.c`  | optional: support of the C library (e.g. locks of newlib)                               |
| `port/<arch>/device/<dev>/osport.h`   | frequencies and sizes of the timers, inline functions of the system timer               |
| `port/<arch>/device/<dev>/osport.c`   | `port_sys_init`, interrupt handlers of the system timer                                 |
| `port/<arch>/common/*.h`              | optional: headers common to all devices (e.g. `osmpu.h`)                                |

The build scripts select the port with the variables `STATEOS_ARCH`, `STATEOS_COMPILER` and `STATEOS_DEVICE`
(`make/<board>/makefile.<cc>`, `cmake/config-<board>-<cc>.cmake`), add the directories to the include path
and compile `oscore.c`, `oscore.s`, `oslibc.c` and `osport.c` if they exist.

The headers are included in this order: `oskernel.h` → `oscore.h` → `osbase.h` → `osport.h` → `osconfig.h` (configuration of the application, skipped with `NOCONFIG`) and `osdefs.h`.
So `osport.h` can not use the types of the kernel (e.g. `cnt_t`), `oscore.h` can.

## Compiler definitions

`osdefs.h` provides the definitions of the compiler used by the kernel.
On Cortex-M most of them come from CMSIS (`cmsis_compiler.h`), so `osdefs.h` defines only `__CONSTRUCTOR` (if the compiler supports it);
the other ports define all of them (see `port/x86/compiler/gcc/osdefs.h`):

| definition              | required | description                                                                                      |
|-------------------------|----------|--------------------------------------------------------------------------------------------------|
| `__STATIC_INLINE`       | yes      | static inline function                                                                           |
| `__NO_RETURN`           | yes      | function that never returns                                                                      |
| `__COMPILER_BARRIER()`  | yes      | compiler memory barrier (critical section)                                                       |
| `__PACKED_STRUCT`       | yes      | structure without padding (header of a message in the message queue)                             |
| `__ALIGNED(x)`          | no       | alignment of a variable to `x` bytes (task stacks with MPU, system heap); default: no alignment  |
| `__CONSTRUCTOR`         | no       | function called before `main`: `sys_init` is called automatically and the macros that start an object at its definition are available (`OS_TSK_START`, `OS_WRK_START`, `OS_TMR_START`, `OS_TMR_UNTIL`, ...); without it the application calls `sys_init` at the beginning of `main` |

`__MPU_USED` come from the header of the device (CMSIS), the default value is 0.

## Configuration

The port sets the default values of the configuration; the application can change them in `osconfig.h`:

| definition       | file         | description                                                                                |
|------------------|--------------|--------------------------------------------------------------------------------------------|
| `CPU_FREQUENCY`  | `osport.h`   | frequency of the processor in Hz                                                           |
| `OS_FREQUENCY`   | `osport.h`   | frequency of the system timer in Hz (ticks per second)                                     |
| `OS_TIMER_SIZE`  | `osport.h`   | bit size of the system time `cnt_t`: 16, 32 or 64                                          |
| `OS_ROBIN`       | `osport.h`   | frequency of the round-robin in Hz: 0 = cooperative mode; must not exceed `OS_FREQUENCY`   |
| `OS_STACK_SIZE`  | `oscore.h`   | default size of the task stack in bytes                                                    |
| `OS_IDLE_STACK`  | `oscore.h`   | size of the idle task stack in bytes                                                       |
| `OS_HEAP_SIZE`   | `oscore.h`   | size of the system heap in bytes: 0 = the heap of the C library (`malloc`)                |
| `OS_MAIN_PRIO`   | `oscore.h`   | priority of the main task                                                                  |
| `OS_LOCK_LEVEL`  | `oscore.h`   | optional, specific to the port: interrupts masked by the critical section                  |

The port also defines `HW_TIMER_SIZE` (internal, it must not be set in `osconfig.h`): the bit size of the hardware timer in tick-less mode, or 0 in tick mode.
The ports choose the tick-less mode for `OS_FREQUENCY` above 1000 Hz.

A port should reject the configuration that it does not support with `#error`, e.g. `OS_ATOMICS` on a core without atomic operations.

## Types and definitions

They are defined in `oscore.h`:

| definition         | description                                                                                                  |
|--------------------|--------------------------------------------------------------------------------------------------------------|
| `lck_t`            | state of the interrupts saved by `port_get_lock`                                                             |
| `stk_t`            | unit of the task stack; its size is the alignment of the stack (e.g. `uint64_t` on Cortex-M)                 |
| `ctx_t`            | context of the task saved on its stack by the context switch handler                                         |
| `_CTX_INIT(pc)`    | static initializer of `ctx_t`: the task starts the function `pc` (used for the idle task)                    |
| `MAIN_TOP`         | optional: top of the stack of the main task (e.g. the symbol of the linker script); without it the kernel reserves a stack of `OS_STACK_SIZE` bytes for `tsk_flip` called in the main task |

The kernel places the context of a new task at the top of its stack and initializes it with `port_ctx_init`.
The first context switch to the task loads this context, so the task starts `core_tsk_loop` (or `core_tsk_exec` with `OS_TASK_EXIT`) on its own stack.

## Functions of the port

The functions are usually `__STATIC_INLINE` in `oscore.h` or `osport.h`, `port_sys_init` and `port_sys_time` (when it is not inline) are in `osport.c`.

| function                     | description                                                                                       |
|------------------------------|---------------------------------------------------------------------------------------------------|
| `port_get_lock()`            | return the current state of the interrupts                                                        |
| `port_put_lock(lck)`         | restore the state of the interrupts                                                               |
| `port_set_lock()`            | mask the interrupts that use the kernel (begin of the critical section)                           |
| `port_clr_lock()`            | unmask the interrupts (end of the critical section)                                               |
| `port_isr_context()`         | true if called from an interrupt handler                                                          |
| `port_get_sp()`              | current stack pointer of the task (stack checks in `DEBUG` mode)                                  |
| `port_ctx_init(ctx, pc)`     | initialize the context `ctx` of a new task, so that it starts the function `pc`                   |
| `port_ctx_switch()`          | request the context switch: it is executed when the interrupts are unmasked (e.g. pending PendSV) |
| `port_ctx_switchNow()`       | request the context switch and execute it now (called in thread mode)                             |
| `port_ctx_reset()`           | restart the time slice of the round-robin (tick-less mode with `OS_ROBIN`), otherwise empty       |
| `port_sys_init()`            | configure the system timer, the interrupt of the context switch and the MPU; called once by `sys_init` |
| `port_sys_wait()`            | wait for an interrupt in the idle task; called with the interrupts masked by `port_set_lock`, it must not miss an interrupt pending since then |
| `port_sys_time()`            | tick-less mode: current system time read from the hardware timer                                  |
| `port_tmr_start(time)`       | tick-less mode: generate the timer interrupt when the hardware timer reaches `time`               |
| `port_tmr_stop()`            | tick-less mode: disable the timer interrupt set by `port_tmr_start`                               |
| `port_tmr_force()`           | tick-less mode: generate the timer interrupt now                                                  |
| `port_mpu_*`                 | optional: memory protection (`__MPU_USED`), see `port/cortexm/common/osmpu.h`                     |

## Functions of the kernel used by the port

| function                     | caller                                                                                           |
|------------------------------|--------------------------------------------------------------------------------------------------|
| `core_tsk_switch(sp)`        | context switch handler: it saves the context of the current task on its stack, passes the stack pointer to `core_tsk_switch` and loads the context from the returned stack pointer of the next task |
| `core_sys_tick()`            | tick mode: interrupt of the system timer with frequency `OS_FREQUENCY`; tick-less mode with `HW_TIMER_SIZE < OS_TIMER_SIZE`: overflow of the hardware timer |
| `core_tmr_handler()`         | tick-less mode: interrupt generated by `port_tmr_start` or `port_tmr_force`                      |
| `core_ctx_switch()`          | tick-less mode with `OS_ROBIN`: interrupt of the round-robin timer with frequency `OS_ROBIN`     |
| `core_tsk_loop()`, `core_tsk_exec()` | `core_tsk_flip`                                                                          |

`core_tsk_flip(sp)` is implemented by the port (usually in assembler): it sets the stack pointer to `sp` and jumps to `core_tsk_loop` (`core_tsk_exec` with `OS_TASK_EXIT`).

## System time

- tick mode (`HW_TIMER_SIZE == 0`): the system timer generates interrupts with frequency `OS_FREQUENCY`, the handler calls `core_sys_tick`
- tick-less mode, the hardware timer at least as wide as the system time (`HW_TIMER_SIZE >= OS_TIMER_SIZE`): `port_sys_time` returns the counter of the hardware timer (the ports with a wider timer set its period to `CNT_MAX`)
- tick-less mode, the hardware timer narrower than the system time (`HW_TIMER_SIZE < OS_TIMER_SIZE`): the overflow interrupt calls `core_sys_tick` (it adds the period of the hardware timer to `System.cnt`) and `port_sys_time` returns `System.cnt` plus the counter, taking into account the overflow not yet handled

In tick-less mode the hardware timer counts with frequency `OS_FREQUENCY` and the compare interrupt (`port_tmr_start`) calls `core_tmr_handler`.

## Interrupts

- the interrupt of the context switch has the lowest priority, so it is executed after all other interrupts
- the interrupts of the system timer must be masked by `port_set_lock`
- the interrupt handlers that call the functions of the kernel must be masked by `port_set_lock`; with `OS_LOCK_LEVEL` the higher priority interrupts stay unmasked and can use only the Async functions

## Steps

1. copy the most similar port and rename the device / compiler directories
2. `osdefs.h`, `oscore.h`: types, critical section, `port_isr_context`, `port_get_sp`
3. `oscore.c` or `oscore.s`: context switch handler and `core_tsk_flip`; `port_ctx_init` and `_CTX_INIT` must match the layout of the context saved by the handler
4. `osport.h`, `osport.c`: tick mode (`HW_TIMER_SIZE == 0`) and cooperative mode (`OS_ROBIN == 0`) first, then preemption and the tick-less mode
5. configuration of the board for the build scripts: `make/<board>/makefile.<cc>`, `cmake/config-<board>-<cc>.cmake`

## Testing

- examples and unit tests (directory `test`) of the [StateOS repository](https://github.com/stateos/StateOS), built for the new board
- cooperative and preemptive mode (`OS_ROBIN`), tick and tick-less mode (`OS_FREQUENCY`)
- `OS_TIMER_SIZE` 16 (the system time wraps around quickly), 32 and 64
- `DEBUG`: checks of the integrity of the task stacks at every context switch
