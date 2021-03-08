ifndef DEVICE_INCLUDE_GUARD
       DEVICE_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef TARGET_PLATFORM
$(error Please define TARGET_PLATFORM before including the device package)
endif

-include $(COMMON)/device/$(TARGET_PLATFORM)/makefile

endif
