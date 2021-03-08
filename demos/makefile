ifndef DEMOS_INCLUDE_GUARD
       DEMOS_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef DEMOS_ARCH
$(error Please define DEMOS_ARCH before including the demos-kernel package)
endif

ifndef DEMOS_DEVICE
$(error Please define DEMOS_DEVICE before including the demos-kernel package)
endif

INCS += $(COMMON)/demos/kernel
INCS += $(COMMON)/demos/port/$(DEMOS_ARCH)/$(DEMOS_DEVICE)

SRCS += $(COMMON)/demos/kernel/os.c
ifneq ($(wildcard $(COMMON)/demos/port/$(DEMOS_ARCH)/$(DEMOS_DEVICE)/osport.c),)
SRCS += $(COMMON)/demos/port/$(DEMOS_ARCH)/$(DEMOS_DEVICE)/osport.c
endif

endif
