ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?=
GCC        ?=
OPTF       ?=
STDC       ?= 11
STDCXX     ?= 17

#----------------------------------------------------------#

ifeq ($(strip $(PROJECT)),)
PROJECT    := $(firstword $(notdir $(CURDIR)))
endif

#----------------------------------------------------------#

ifneq ($(filter CLANG,$(DEFS)),)
AS         := $(GCC)clang -x assembler-with-cpp
CC         := $(GCC)clang
CXX        := $(GCC)clang++
COPY       := $(GCC)llvm-objcopy
DUMP       := $(GCC)llvm-objdump
SIZE       := $(GCC)llvm-size
LD         := $(GCC)clang++
AR         := $(GCC)llvm-ar
else
AS         := $(GCC)gcc -x assembler-with-cpp
CC         := $(GCC)gcc
CXX        := $(GCC)g++
COPY       := $(GCC)objcopy
DUMP       := $(GCC)objdump
SIZE       := $(GCC)size
LD         := $(GCC)g++
AR         := $(GCC)ar
endif
FC         := $(GCC)gfortran
RES        := $(GCC)windres

RM         ?= rm -f

#----------------------------------------------------------#

EXE        := $(PROJECT).exe
LIB        := lib$(PROJECT).a
LSS        := $(PROJECT).lss
MAP        := $(PROJECT).map

OBJS       := $(SRCS:%=%.o)
DEPS       := $(OBJS:%.o=%.d)
LSTS       := $(OBJS:%.o=%.lst)

#----------------------------------------------------------#

GENERATED  := $(EXE) $(LIB) $(LSS) $(MAP) $(DEPS) $(LSTS) $(OBJS)

#----------------------------------------------------------#

ifneq (clean,$(MAKECMDGOALS))
COMMON_F   += -O$(OPTF)
COMMON_F   += -MD -MP
ifeq  ($(filter CLANG,$(DEFS)),)
COMMON_F   +=#-Wa,-amhls=$(@:.o=.lst)
endif
AS_FLAGS   +=
ifneq ($(filter ISO,$(DEFS)),)
$(info Using iso)
DEFS       := $(DEFS:ISO=)
C_FLAGS    += -std=c$(STDC:20=2x)
CXX_FLAGS  += -std=c++$(STDCXX:20=2a)
else
C_FLAGS    += -std=gnu$(STDC:20=2x)
CXX_FLAGS  += -std=gnu++$(STDCXX:20=2a)
endif
F_FLAGS    += -cpp
LD_FLAGS   += -Wl,--gc-sections,-Map=$(MAP)
ifneq ($(filter CLANG,$(DEFS)),)
$(info Using clang)
else
DEFS       := $(DEFS:CLANG=)
COMMON_F   += -s
ifneq ($(filter CONSOLE,$(DEFS)),)
$(info Using console)
DEFS       := $(DEFS:CONSOLE=)
COMMON_F   += -mconsole
endif
ifneq ($(filter WINDOWS,$(DEFS)),)
$(info Using windows)
DEFS       := $(DEFS:WINDOWS=)
COMMON_F   += -mwindows
endif
ifneq ($(filter STATIC,$(DEFS)),)
$(info Using static)
DEFS       := $(DEFS:STATIC=)
COMMON_F   += -static
endif
ifneq ($(filter LTO,$(DEFS)),)
$(info Using lto)
DEFS       := $(DEFS:LTO=)
COMMON_F   += -flto
endif
endif
ifneq ($(filter UNICODE,$(DEFS)),)
$(info Using unicode)
DEFS       += _UNICODE
COMMON_F   += -municode
else
ifneq ($(filter unicode,$(MAKECMDGOALS)),)
$(info Using unicode)
DEFS       += _UNICODE UNICODE
COMMON_F   += -municode
endif
endif
ifneq ($(filter NOWARNINGS,$(DEFS)),)
$(info Using nowarnings)
DEFS       := $(DEFS:NOWARNINGS=)
COMMON_F   += -Wall
else
COMMON_F   += -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wlogical-op
CXX_FLAGS  += -Wzero-as-null-pointer-constant
endif
ifneq ($(filter DEBUG,$(DEFS)),)
$(info Using debug)
COMMON_F   += -g -ggdb
else
ifeq  ($(filter NDEBUG,$(DEFS)),)
DEFS       += NDEBUG
endif
endif
endif

#----------------------------------------------------------#

DEFS_F     := $(DEFS:%=-D%)
INCS_F     := $(INCS:%=-I%/)
LIB_DIRS_F := $(LIB_DIRS:%=-L%)

AS_FLAGS   += $(COMMON_F) $(DEFS_F) $(INCS_F)
C_FLAGS    += $(COMMON_F) $(DEFS_F) $(INCS_F)
CXX_FLAGS  += $(COMMON_F) $(DEFS_F) $(INCS_F)
F_FLAGS    += $(COMMON_F) $(DEFS_F) $(INCS_F)
LD_FLAGS   += $(COMMON_F)

#----------------------------------------------------------#

$(info Using '$(MAKECMDGOALS)')

all : $(LSS) print_exe_size

unicode : all

lib : $(LIB) print_size

$(OBJS) : $(MAKEFILE_LIST)

%.s.o : %.s
	$(info $<)
	$(AS) $(AS_FLAGS) -c $< -o $@

%.c.o : %.c
	$(info $<)
	$(CC) $(C_FLAGS) -c $< -o $@

%.cc.o : %.cc
	$(info $<)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

%.cpp.o : %.cpp
	$(info $<)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

%.f.o : %.f
	$(info $<)
	$(FC) $(F_FLAGS) -c $< -o $@

%.rc.o : %.rc
	$(info $<)
	$(RES) $< $@

$(EXE) : $(OBJS)
	$(info $@)
	$(LD) $(LD_FLAGS) $(OBJS) $(LIBS) $(LIB_DIRS_F) -o $@

$(LIB) : $(OBJS)
	$(info $@)
	$(AR) -r $@ $?

$(LSS) : $(EXE)
	$(info $@)
	$(DUMP) -C -d $< > $@

print_size : $(OBJS)
	$(info Size of modules:)
	$(SIZE) -B -t --common $(OBJS)

print_exe_size : $(EXE)
	$(info Size of target file:)
	$(SIZE) -B $(EXE)

clean :
	$(info Removing all generated output files)
	$(RM) $(GENERATED)

run : all
	$(info Starting the program...)
ifneq ($(OS),Windows_NT)
	@chmod 777 ./$(EXE)
endif
	@./$(EXE)

.PHONY : all unicode lib run clean

-include $(DEPS)
