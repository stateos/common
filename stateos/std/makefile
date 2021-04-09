ifndef STATEOS_STD_INCLUDE_GUARD
       STATEOS_STD_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

DEFS += _GLIBCXX_HAS_GTHREADS

INCS += $(COMMON)/stateos/std

SRCS += $(COMMON)/stateos/std/src/chrono.cc
SRCS += $(COMMON)/stateos/std/src/future.cc
SRCS += $(COMMON)/stateos/std/src/mutex.cc
SRCS += $(COMMON)/stateos/std/src/condition_variable.cc
SRCS += $(COMMON)/stateos/std/src/thread.cc

include $(COMMON)/stateos/makefile

endif
