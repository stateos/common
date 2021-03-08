ifndef CMSIS_OS_INCLUDE_GUARD
       CMSIS_OS_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef TARGET_COMPILER
$(error Please define TARGET_COMPILER before including the cmsis-os package)
endif

INCS += $(COMMON)/cmsis/os/inc

SRCS += $(COMMON)/cmsis/os/src/os_systick.c

LIBS += $(COMMON)/cmsis/os/lib/$(TARGET_COMPILER)/$(LIBRARY_PREFIX)RTX_CM4F$(LIBRARY_SUFFIX)

include $(COMMON)/cmsis/makefile

endif
