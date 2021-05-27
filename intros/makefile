ifndef INTROS_INCLUDE_GUARD
       INTROS_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef INTROS_ARCH
$(error Please define INTROS_ARCH before including the intros-kernel package)
endif

ifndef INTROS_DEVICE
$(error Please define INTROS_DEVICE before including the intros-kernel package)
endif

ifndef INTROS_COMPILER
$(error Please define INTROS_COMPILER before including the intros-kernel package)
endif

INCS += $(COMMON)/intros/kernel
INCS += $(COMMON)/intros/port/$(INTROS_ARCH)/compiler/$(INTROS_COMPILER)
INCS += $(COMMON)/intros/port/$(INTROS_ARCH)/device/$(INTROS_DEVICE)
ifneq ($(wildcard $(COMMON)/intros/port/$(INTROS_ARCH)/common),)
INCS += $(COMMON)/intros/port/$(INTROS_ARCH)/common
endif

SRCS += $(COMMON)/intros/kernel/oskernel.c
SRCS += $(COMMON)/intros/kernel/ossys.c
SRCS += $(COMMON)/intros/kernel/src/osclock.c
SRCS += $(COMMON)/intros/kernel/src/osbarrier.c
SRCS += $(COMMON)/intros/kernel/src/osconditionvariable.c
SRCS += $(COMMON)/intros/kernel/src/osevent.c
SRCS += $(COMMON)/intros/kernel/src/oseventqueue.c
SRCS += $(COMMON)/intros/kernel/src/osflag.c
SRCS += $(COMMON)/intros/kernel/src/osjobqueue.c
SRCS += $(COMMON)/intros/kernel/src/oslist.c
SRCS += $(COMMON)/intros/kernel/src/osmailboxqueue.c
SRCS += $(COMMON)/intros/kernel/src/osmemorypool.c
SRCS += $(COMMON)/intros/kernel/src/osmessagequeue.c
SRCS += $(COMMON)/intros/kernel/src/osmutex.c
SRCS += $(COMMON)/intros/kernel/src/osrwlock.c
SRCS += $(COMMON)/intros/kernel/src/osrawbuffer.c
SRCS += $(COMMON)/intros/kernel/src/ossemaphore.c
SRCS += $(COMMON)/intros/kernel/src/ossignal.c
SRCS += $(COMMON)/intros/kernel/src/ostask.c
SRCS += $(COMMON)/intros/kernel/src/ostimer.c
ifneq ($(wildcard $(COMMON)/intros/port/$(INTROS_ARCH)/compiler/$(INTROS_COMPILER)/oscore.s),)
SRCS += $(COMMON)/intros/port/$(INTROS_ARCH)/compiler/$(INTROS_COMPILER)/oscore.s
endif
ifneq ($(wildcard $(COMMON)/intros/port/$(INTROS_ARCH)/compiler/$(INTROS_COMPILER)/oscore.c),)
SRCS += $(COMMON)/intros/port/$(INTROS_ARCH)/compiler/$(INTROS_COMPILER)/oscore.c
endif
ifneq ($(wildcard $(COMMON)/intros/port/$(INTROS_ARCH)/compiler/$(INTROS_COMPILER)/oslibc.c),)
SRCS += $(COMMON)/intros/port/$(INTROS_ARCH)/compiler/$(INTROS_COMPILER)/oslibc.c
endif
ifneq ($(wildcard $(COMMON)/intros/port/$(INTROS_ARCH)/compiler/$(INTROS_COMPILER)/oslibc.cpp),)
SRCS += $(COMMON)/intros/port/$(INTROS_ARCH)/compiler/$(INTROS_COMPILER)/oslibc.cpp
endif
ifneq ($(wildcard $(COMMON)/intros/port/$(INTROS_ARCH)/device/$(INTROS_DEVICE)/osport.c),)
SRCS += $(COMMON)/intros/port/$(INTROS_ARCH)/device/$(INTROS_DEVICE)/osport.c
endif

endif
