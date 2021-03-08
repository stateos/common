ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

SRCS += $(COMMON)/startup/$(TARGET_CHIP)/$(TARGET_COMPILER)/vectab.c

LIBS += $(COMMON)/startup/$(TARGET_CHIP)/$(TARGET_COMPILER)/vectors.sm8
LIBS += $(COMMON)/startup/$(TARGET_CHIP)/$(TARGET_COMPILER)/vectors.far.sm8

SCRIPT += $(COMMON)/startup/$(TARGET_CHIP)/$(TARGET_COMPILER)/script.lkf
