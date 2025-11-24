ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?= # project name (default is folder name)
VERSION    ?= # project version
BUILD      ?= # build folder name (default is 'build')
DEFS       ?= # used definitions
INCS       ?= # include directories
SRCS       ?= # source files, modules first
LIBS       ?= # used libraries
PREINC     ?=
LIB_DIRS   ?=

#----------------------------------------------------------#

GCC        ?= # toolchain path
OPTF       ?= # optimization level (0..3, s, fast, g)
STDC       ?= 23 # c dialect
STDCXX     ?= 23 # c++ dialect

############################################################

PROJECT    := $(firstword $(PROJECT) $(notdir $(CURDIR)))
VERSION    := $(firstword $(VERSION))
BUILD      := $(firstword $(BUILD) build)
ifneq ($(VERSION),)
PROJECT    := $(PROJECT)-v$(VERSION)
endif # version

#----------------------------------------------------------#

ifneq ($(filter CLANG,$(DEFS)),)
AS         := $(GCC)clang
CC         := $(GCC)clang
CXX        := $(GCC)clang++
COPY       := $(GCC)llvm-objcopy
DUMP       := $(GCC)llvm-objdump
SIZE       := $(GCC)llvm-size
LD         := $(GCC)clang++
AR         := $(GCC)llvm-ar
else
AS         := $(GCC)gcc
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

ifneq ($(OS),Windows_NT)
ELF        := $(BUILD)/$(PROJECT)
else
ELF        := $(BUILD)/$(PROJECT).exe
endif
LIB        := $(BUILD)/lib$(PROJECT).a
DMP        := $(BUILD)/$(PROJECT).dmp
LSS        := $(BUILD)/$(PROJECT).lss
MAP        := $(BUILD)/$(PROJECT).map

SRCS       := $(foreach s,$(SRCS),$(realpath $s))
OBJS       := $(SRCS:%=$(BUILD)%.o)
DEPS       := $(OBJS:.o=.d)
LIBS       := $(LIBS:%=-l%)

#----------------------------------------------------------#

ifneq (clean,$(MAKECMDGOALS))
COMMON_F   += -MD -MP
ifeq  ($(filter CLANG,$(DEFS)),)
COMMON_F   +=#-Wa,-amhls=$(@:.o=.lst)
endif # clang
ASFLAGS    += -Xassembler-with-cpp
CFLAGS     += -Wlogical-op
CXXFLAGS   += -Wlogical-op -fconcepts -fmodules-ts
ifneq ($(filter ISO,$(DEFS)),)
$(info Using iso)
DEFS       := $(DEFS:ISO=)
ifneq ($(STDC),)
CFLAGS     += -std=c$(STDC:20=2x)
endif # stdc
ifneq ($(STDCXX),)
CXXFLAGS   += -std=c++$(STDCXX:20=2a)
endif # stdcxx
else  # iso
ifneq ($(STDC),)
CFLAGS     += -std=gnu$(STDC:20=2x)
endif # stdc
ifneq ($(STDCXX),)
CXXFLAGS   += -std=gnu++$(STDCXX:20=2a)
endif # stdcxx
endif # iso
FFLAGS     += -cpp
LDFLAGS    += -Wl,-Map=$(MAP),--gc-sections
ifneq ($(filter CLANG,$(DEFS)),)
$(info Using clang)
else  # clang
DEFS       := $(DEFS:CLANG=)
COMMON_F   += -s
ifneq ($(filter CONSOLE,$(DEFS)),)
$(info Using console)
DEFS       := $(DEFS:CONSOLE=)
COMMON_F   += -mconsole
endif # console
ifneq ($(filter WINDOWS,$(DEFS)),)
$(info Using windows)
DEFS       := $(DEFS:WINDOWS=)
COMMON_F   += -mwindows
endif # windows
ifneq ($(filter STATIC,$(DEFS)),)
$(info Using static)
DEFS       := $(DEFS:STATIC=)
COMMON_F   += -static
endif # static
ifneq ($(filter LTO,$(DEFS)),)
$(info Using lto)
DEFS       := $(DEFS:LTO=)
COMMON_F   += -flto
endif # lto
endif # clang
ifneq ($(filter UNICODE,$(DEFS)),)
$(info Using unicode)
DEFS       += _UNICODE
COMMON_F   += -municode
else  # unicode
ifneq ($(filter unicode,$(MAKECMDGOALS)),)
$(info Using unicode)
DEFS       += _UNICODE UNICODE
COMMON_F   += -municode
endif # unicode
endif # unicode
ifneq ($(filter NOWARNINGS,$(DEFS)),)
$(info Using nowarnings)
DEFS       := $(DEFS:NOWARNINGS=)
COMMON_F   += -Wall
else  # nowarnings
COMMON_F   += -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wsign-conversion
CXXFLAGS   += -Wzero-as-null-pointer-constant
endif # nowarnings
ifneq ($(filter DEBUG,$(DEFS)),)
$(info Using debug)
COMMON_F   += -Og -g -ggdb
DEFS       := $(DEFS:NDEBUG=)
DEFS       := $(DEFS:MINSIZE=)
DEFS       := $(DEFS:MAXSPEED=)
else  # debug
ifeq  ($(filter NDEBUG,$(DEFS)),)
DEFS       += NDEBUG
endif # ndebug
ifneq ($(filter MINSIZE,$(DEFS)),)
$(info Using minsize)
DEFS       := $(DEFS:MINSIZE=)
DEFS       := $(DEFS:MAXSPEED=)
COMMON_F   += -Os
else  # minsize
ifneq ($(filter MAXSPEED,$(DEFS)),)
$(info Using maxspeed)
DEFS       := $(DEFS:MINSIZE=)
DEFS       := $(DEFS:MAXSPEED=)
COMMON_F   += -Ofast
else  # maxspeed
COMMON_F   += -O$(OPTF)
endif # maxspeed
endif # minsize
endif # debug
endif # makecmdgoals

#----------------------------------------------------------#

DEFS_F     := $(DEFS:%=-D%)
INCS_F     := $(INCS:%=-I%) $(PREINC:%=-include %)
LIB_DIRS_F := $(LIB_DIRS:%=-L%)

ASFLAGS    += $(COMMON_F) $(DEFS_F) $(INCS_F)
CFLAGS     += $(COMMON_F) $(DEFS_F) $(INCS_F)
CXXFLAGS   += $(COMMON_F) $(DEFS_F) $(INCS_F)
FFLAGS     += $(COMMON_F) $(DEFS_F) $(INCS_F)
LDFLAGS    += $(COMMON_F) $(LIB_DIRS_F)

#----------------------------------------------------------#

$(info Goal: $(MAKECMDGOALS))

all : $(ELF) $(DMP) $(LSS) print_elf_size

unicode : all

lib : $(LIB) print_size

$(OBJS) : $(MAKEFILE_LIST)

$(BUILD)/%.S.o : /%.S
	$(info $<)
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD)/%.s.o : /%.s
	$(info $<)
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD)/%.c.o : /%.c
	$(info $<)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.cc.o : /%.cc
	$(info $<)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.cpp.o : /%.cpp
	$(info $<)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.f.o : /%.f
	$(info $<)
	mkdir -p $(dir $@)
	$(FC) $(FFLAGS) -c $< -o $@

$(BUILD)/%.rc.o : /%.rc
	$(info $<)
	mkdir -p $(dir $@)
	$(RES) $< $@

$(ELF) : $(OBJS)
	$(info $@)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
ifneq ($(OS),Windows_NT)
	@chmod 777 $@
endif

$(LIB) : $(OBJS)
	$(info $@)
	$(AR) -r $@ $?

$(DMP) : $(ELF)
	$(info $@)
	$(DUMP) -Ctx $< > $@

$(LSS) : $(ELF)
	$(info $@)
	$(DUMP) -CS $< > $@

print_size : $(OBJS)
	$(info Size of modules:)
	$(SIZE) -B -t --common $?

print_elf_size : $(ELF)
	$(info Size of target:)
	$(SIZE) -B $<

clean :
	$(info Removing all generated output files)
	$(RM) -Rd $(BUILD)
	$(RM) -Rd gcm.cache

run : all
	$(info Running the target...)
	@$(ELF)

.PHONY : all unicode lib clean run

-include $(DEPS)
