ifndef STATEOS_STD_INCLUDE_GUARD
       STATEOS_STD_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

DEFS += _GLIBCXX_HAS_GTHREADS

INCS += $(COMMON)/stateos/stdc++

SRCS += $(COMMON)/stateos/stdc++/src/emutls.cc
SRCS += $(COMMON)/stateos/stdc++/src/chrono.cc
SRCS += $(COMMON)/stateos/stdc++/src/future.cc
SRCS += $(COMMON)/stateos/stdc++/src/mutex.cc
SRCS += $(COMMON)/stateos/stdc++/src/condition_variable.cc
SRCS += $(COMMON)/stateos/stdc++/src/thread.cc

include $(COMMON)/stateos/makefile

endif
