ifndef CMSIS_INCLUDE_GUARD
       CMSIS_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef TARGET_DEVICE
$(error Please define TARGET_DEVICE before including cmsis-core package)
endif

INCS += $(COMMON)/cmsis/include
INCS += $(COMMON)/cmsis/$(TARGET_DEVICE)

endif
