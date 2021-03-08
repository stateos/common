ifndef STARTUP_CHIP_INCLUDE_GUARD
       STARTUP_CHIP_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef TARGET_COMPILER
$(error Please define TARGET_COMPILER before including the startup package)
endif

SRCS += $(COMMON)/startup/$(TARGET_CHIP)/system.c
SRCS += $(COMMON)/startup/$(TARGET_CHIP)/startup.c

include $(COMMON)/startup/$(TARGET_CHIP)/$(TARGET_COMPILER)/makefile

endif
