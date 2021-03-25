ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?= # project name
CSMC       ?= # toolchain path (st7)
CSTM       ?= # toolchain path (stm8)
BUILD      ?= build

#----------------------------------------------------------#

ifeq ($(BUILD),)
$(error Invalid BUILD definition)
endif

#----------------------------------------------------------#

PROJECT    := $(firstword $(PROJECT) $(notdir $(CURDIR)))

#----------------------------------------------------------#

AS         := $(CSMC)cxst7
CC         := $(CSMC)cxst7
DWARF      := $(CSMC)cvdwarf
DUMP       := $(CSMC)chex
LD         := $(CSMC)clnk
AR         := $(CSTM)clib
LABS       := $(CSMC)clabs
SIZE       := size

RM         ?= rm -f

#----------------------------------------------------------#

ST7        := $(BUILD)/$(PROJECT).st7
ELF        := $(BUILD)/$(PROJECT).elf
LIB        := $(BUILD)/$(PROJECT).st7
HEX        := $(BUILD)/$(PROJECT).s19
MAP        := $(BUILD)/$(PROJECT).map

SRCS       := $(foreach s,$(SRCS),$(realpath $s))
OBJS       := $(SRCS:%.s=$(BUILD)%.o)
OBJS       := $(OBJS:%.c=$(BUILD)%.o)

#----------------------------------------------------------#

LIBS       += crtsx.st7 libisl.st7 libm.st7

#----------------------------------------------------------#

ifneq (clean,$(MAKECMDGOALS))
AS_FLAGS   += -l -ep # -ax
C_FLAGS    += -pc99 -l -pad +modsl
LD_FLAGS   += -m $(MAP) -p
ifneq ($(filter DEBUG,$(DEFS)),)
$(info Using debug)
C_FLAGS    += +debug
else
ifeq  ($(filter NDEBUG,$(DEFS)),)
DEFS       += NDEBUG
endif
endif
endif

#----------------------------------------------------------#

DEFS_F     := $(DEFS:%=-d%)
INCS_F     := $(INCS:%=-i%)
LIB_DIRS_F := $(LIB_DIRS:%=-l%)

AS_FLAGS   += $(DEFS_F)
C_FLAGS    += $(DEFS_F) $(INCS_F)
LD_FLAGS   += $(SCRIPT) $(LIB_DIRS_F)

#----------------------------------------------------------#

$(info Using '$(MAKECMDGOALS)')

all : $(ELF) print_elf_size

lib : $(OBJS)
	$(info $(LIB))
	$(AR) -c -p $(LIB) $?

$(OBJS) : $(MAKEFILE_LIST)

$(BUILD)/%.o : /%.s
	mkdir -p $(dir $@)
	$(AS) -co $(dir $@) -cl $(dir $@) $(AS_FLAGS) $<

$(BUILD)/%.o : /%.c
	mkdir -p $(dir $@)
	$(CC) -co $(dir $@) -cl $(dir $@) $(C_FLAGS) $<

$(ST7) : $(OBJS) $(SCRIPT)
	$(info $@)
	$(LD) -o $@ $(LD_FLAGS) $(OBJS) $(LIBS)

$(ELF) : $(ST7)
	$(info $@)
	$(DWARF) $<

$(HEX) : $(ST7)
	$(info $@)
	$(DUMP) -o $@ $<

print_elf_size : $(ELF)
	$(info Size of target:)
	$(SIZE) -B $<

clean :
	$(info Removing all generated output files)
	$(RM) -Rd $(BUILD)

.PHONY : all lib clean
