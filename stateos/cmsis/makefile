ifndef STATEOS_CMSIS_INCLUDE_GUARD
       STATEOS_CMSIS_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

INCS += $(COMMON)/stateos/cmsis/inc

SRCS += $(COMMON)/stateos/cmsis/src/cmsis_os1.c
SRCS += $(COMMON)/stateos/cmsis/src/cmsis_os2.c

include $(COMMON)/stateos/makefile

endif
