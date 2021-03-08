ifndef STATEOS_INCLUDE_GUARD
       STATEOS_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef STATEOS_ARCH
$(error Please define STATEOS_ARCH before including the stateos-kernel package)
endif

ifndef STATEOS_DEVICE
$(error Please define STATEOS_DEVICE before including the stateos-kernel package)
endif

ifndef STATEOS_COMPILER
$(error Please define STATEOS_COMPILER before including the stateos-kernel package)
endif

INCS += $(COMMON)/stateos/kernel
INCS += $(COMMON)/stateos/port/$(STATEOS_ARCH)/compiler/$(STATEOS_COMPILER)
INCS += $(COMMON)/stateos/port/$(STATEOS_ARCH)/device/$(STATEOS_DEVICE)
ifneq ($(wildcard $(COMMON)/stateos/port/$(STATEOS_ARCH)/common),)
INCS += $(COMMON)/stateos/port/$(STATEOS_ARCH)/common
endif

SRCS += $(COMMON)/stateos/kernel/osalloc.c
SRCS += $(COMMON)/stateos/kernel/oskernel.c
SRCS += $(COMMON)/stateos/kernel/src/osclock.c
SRCS += $(COMMON)/stateos/kernel/src/osbarrier.c
SRCS += $(COMMON)/stateos/kernel/src/osconditionvariable.c
SRCS += $(COMMON)/stateos/kernel/src/osevent.c
SRCS += $(COMMON)/stateos/kernel/src/oseventqueue.c
SRCS += $(COMMON)/stateos/kernel/src/osfastmutex.c
SRCS += $(COMMON)/stateos/kernel/src/osflag.c
SRCS += $(COMMON)/stateos/kernel/src/osjobqueue.c
SRCS += $(COMMON)/stateos/kernel/src/oslist.c
SRCS += $(COMMON)/stateos/kernel/src/osmailboxqueue.c
SRCS += $(COMMON)/stateos/kernel/src/osmemorypool.c
SRCS += $(COMMON)/stateos/kernel/src/osmessagebuffer.c
SRCS += $(COMMON)/stateos/kernel/src/osmutex.c
SRCS += $(COMMON)/stateos/kernel/src/osrwlock.c
SRCS += $(COMMON)/stateos/kernel/src/ossemaphore.c
SRCS += $(COMMON)/stateos/kernel/src/ossignal.c
SRCS += $(COMMON)/stateos/kernel/src/osstreambuffer.c
SRCS += $(COMMON)/stateos/kernel/src/ostask.c
SRCS += $(COMMON)/stateos/kernel/src/ostimer.c
ifneq ($(wildcard $(COMMON)/stateos/port/$(STATEOS_ARCH)/compiler/$(STATEOS_COMPILER)/oscore.s),)
SRCS += $(COMMON)/stateos/port/$(STATEOS_ARCH)/compiler/$(STATEOS_COMPILER)/oscore.s
endif
ifneq ($(wildcard $(COMMON)/stateos/port/$(STATEOS_ARCH)/compiler/$(STATEOS_COMPILER)/oscore.c),)
SRCS += $(COMMON)/stateos/port/$(STATEOS_ARCH)/compiler/$(STATEOS_COMPILER)/oscore.c
endif
ifneq ($(wildcard $(COMMON)/stateos/port/$(STATEOS_ARCH)/compiler/$(STATEOS_COMPILER)/oslibc.c),)
SRCS += $(COMMON)/stateos/port/$(STATEOS_ARCH)/compiler/$(STATEOS_COMPILER)/oslibc.c
endif
ifneq ($(wildcard $(COMMON)/stateos/port/$(STATEOS_ARCH)/device/$(STATEOS_DEVICE)/osport.c),)
SRCS += $(COMMON)/stateos/port/$(STATEOS_ARCH)/device/$(STATEOS_DEVICE)/osport.c
endif

endif
