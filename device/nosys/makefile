ifndef DEVICE_NOSYS_INCLUDE_GUARD
       DEVICE_NOSYS_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef TARGET_ARCH
$(error Please define TARGET_ARCH before including the device-nosys package)
endif

ifndef TARGET_COMPILER
$(error Please define TARGET_COMPILER before including the device-nosys package)
endif

ifneq ($(wildcard $(COMMON)/device/nosys/$(TARGET_ARCH)/$(TARGET_COMPILER)/retarget.c),)
SRCS += $(COMMON)/device/nosys/$(TARGET_ARCH)/$(TARGET_COMPILER)/retarget.c
endif

ifneq ($(wildcard $(COMMON)/device/nosys/$(TARGET_ARCH)/$(TARGET_COMPILER)/$(LIBRARY_PREFIX)retarget$(LIBRARY_SUFFIX)),)
LIBS += $(COMMON)/device/nosys/$(TARGET_ARCH)/$(TARGET_COMPILER)/$(LIBRARY_PREFIX)retarget$(LIBRARY_SUFFIX)
endif

include $(COMMON)/device/makefile

endif
