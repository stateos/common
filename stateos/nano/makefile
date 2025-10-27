ifndef STATEOS_NANO_INCLUDE_GUARD
       STATEOS_NANO_INCLUDE_GUARD:=ON

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

ifndef STATEOS_INCLUDE_GUARD

INCS += $(COMMON)/stateos/nano/$(STATEOS_ARCH)/compiler/$(STATEOS_COMPILER)
INCS += $(COMMON)/stateos/nano/$(STATEOS_ARCH)/device/$(STATEOS_DEVICE)
ifneq ($(wildcard $(COMMON)/stateos/nano/$(STATEOS_ARCH)/common),)
INCS += $(COMMON)/stateos/nano/$(STATEOS_ARCH)/common
endif

ifneq ($(wildcard $(COMMON)/stateos/nano/$(STATEOS_ARCH)/compiler/$(STATEOS_COMPILER)/oscore.s),)
SRCS += $(COMMON)/stateos/nano/$(STATEOS_ARCH)/compiler/$(STATEOS_COMPILER)/oscore.s
endif
ifneq ($(wildcard $(COMMON)/stateos/nano/$(STATEOS_ARCH)/compiler/$(STATEOS_COMPILER)/oscore.c),)
SRCS += $(COMMON)/stateos/nano/$(STATEOS_ARCH)/compiler/$(STATEOS_COMPILER)/oscore.c
endif
ifneq ($(wildcard $(COMMON)/stateos/nano/$(STATEOS_ARCH)/compiler/$(STATEOS_COMPILER)/oslibc.c),)
SRCS += $(COMMON)/stateos/nano/$(STATEOS_ARCH)/compiler/$(STATEOS_COMPILER)/oslibc.c
endif
ifneq ($(wildcard $(COMMON)/stateos/nano/$(STATEOS_ARCH)/device/$(STATEOS_DEVICE)/osport.c),)
SRCS += $(COMMON)/stateos/nano/$(STATEOS_ARCH)/device/$(STATEOS_DEVICE)/osport.c
endif

include $(COMMON)/stateos/makefile

endif

endif
