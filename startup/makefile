ifndef STARTUP_INCLUDE_GUARD
       STARTUP_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef TARGET_CHIP
$(error Please define TARGET_CHIP before including the startup package)
endif

include $(COMMON)/startup/$(TARGET_CHIP)/makefile

endif
