ifndef DEVICE_SEMIHOST_INCLUDE_GUARD
       DEVICE_SEMIHOST_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef TARGET_COMPILER
$(error Please define TARGET_COMPILER before including the device-semihost package)
endif

INCS += $(COMMON)/device/semihost

SRCS += $(COMMON)/device/semihost/semihost.c
ifneq ($(wildcard $(COMMON)/device/semihost/$(TARGET_COMPILER)),)
SRCS += $(COMMON)/device/semihost/$(TARGET_COMPILER)/retarget.c
endif

include $(COMMON)/device/nosys/makefile

endif
