ifndef HAL_INCLUDE_GUARD
       HAL_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef TARGET_DEVICE
$(error Please define TARGET_DEVICE before including hal package)
endif

DEFS += USE_HAL_DRIVER

include $(COMMON)/hal/$(TARGET_DEVICE)/makefile

endif
