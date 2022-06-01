ifndef CMSIS_OS_INCLUDE_GUARD
       CMSIS_OS_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef CMSISOS_TARGET
$(error Please define CMSISOS_TARGET before including the cmsis-os package)
endif

ifndef CMSISOS_COMPILER
$(error Please define CMSISOS_COMPILER before including the cmsis-os package)
endif

INCS += $(COMMON)/cmsis/os/inc
INCS += $(COMMON)/cmsis/os/rtx/include

SRCS += $(COMMON)/cmsis/os/src/cmsis_os1.c
SRCS += $(COMMON)/cmsis/os/src/os_systick.c
SRCS +=#$(COMMON)/cmsis/os/rtx/config/RTX_Config.c
SRCS += $(COMMON)/cmsis/os/rtx/source/rtx_delay.c
SRCS += $(COMMON)/cmsis/os/rtx/source/rtx_evflags.c
SRCS += $(COMMON)/cmsis/os/rtx/source/rtx_evr.c
SRCS += $(COMMON)/cmsis/os/rtx/source/rtx_kernel.c
SRCS += $(COMMON)/cmsis/os/rtx/source/rtx_lib.c
SRCS += $(COMMON)/cmsis/os/rtx/source/rtx_memory.c
SRCS += $(COMMON)/cmsis/os/rtx/source/rtx_mempool.c
SRCS += $(COMMON)/cmsis/os/rtx/source/rtx_msgqueue.c
SRCS += $(COMMON)/cmsis/os/rtx/source/rtx_mutex.c
SRCS += $(COMMON)/cmsis/os/rtx/source/rtx_semaphore.c
SRCS += $(COMMON)/cmsis/os/rtx/source/rtx_system.c
SRCS += $(COMMON)/cmsis/os/rtx/source/rtx_thread.c
SRCS += $(COMMON)/cmsis/os/rtx/source/rtx_timer.c
SRCS += $(COMMON)/cmsis/os/rtx/source/$(CMSISOS_COMPILER)/irq_$(CMSISOS_TARGET).s

include $(COMMON)/cmsis/makefile

endif
