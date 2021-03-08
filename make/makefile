ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef TARGET_ARCH
$(error Please define TARGET_ARCH before including any common package)
endif

ifndef TARGET_COMPILER
$(error Please define TARGET_COMPILER before including any common package)
endif

#----------------------------------------------------------#

include $(COMMON)/make/toolchain-$(TARGET_ARCH)-$(TARGET_COMPILER).make

#----------------------------------------------------------#
