ifndef HAL_DEVICE_INCLUDE_GUARD
       HAL_DEVICE_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef TARGET_COMPILER
$(error Please define TARGET_COMPILER before including hal package)
endif

INCS += $(COMMON)/hal/$(TARGET_DEVICE)/inc

LIBS += $(COMMON)/hal/$(TARGET_DEVICE)/lib/$(TARGET_COMPILER)/$(LIBRARY_PREFIX)HAL$(LIBRARY_SUFFIX)

endif
