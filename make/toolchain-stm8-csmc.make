ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?= # project name
CSMC       ?= # toolchain path
STVP       := stvp_cmdline
BUILD      ?= build

#----------------------------------------------------------#

ifeq ($(BUILD),)
$(error Invalid BUILD definition)
endif

#----------------------------------------------------------#

PROJECT    := $(firstword $(PROJECT) $(notdir $(CURDIR)))

#----------------------------------------------------------#

AS         := $(CSMC)cxstm8
CC         := $(CSMC)cxstm8
DWARF      := $(CSMC)cvdwarf
DUMP       := $(CSMC)chex
LD         := $(CSMC)clnk
AR         := $(CSMC)clib
LABS       := $(CSMC)clabs
SIZE       := size

RM         ?= rm -f

#----------------------------------------------------------#

SM8        := $(BUILD)/$(PROJECT).sm8
ELF        := $(BUILD)/$(PROJECT).elf
LIB        := $(BUILD)/$(PROJECT).sm8
HEX        := $(BUILD)/$(PROJECT).s19
MAP        := $(BUILD)/$(PROJECT).map

SRCS       := $(foreach s,$(SRCS),$(realpath $s))
OBJS       := $(SRCS:%.s=$(BUILD)%.o)
OBJS       := $(OBJS:%.c=$(BUILD)%.o)

#----------------------------------------------------------#

ifneq ($(filter far,$(MAKECMDGOALS)),)
LIBS       += crtsx.sm8 libfsl.sm8 libisl.sm8 libm.sm8
else
LIBS       += crtsx0.sm8 libfsl0.sm8 libisl0.sm8 libm0.sm8
endif

#----------------------------------------------------------#

ifneq (clean,$(MAKECMDGOALS))
AS_FLAGS   += -l -ep # -ax
C_FLAGS    += -pc99 -l -pad $(if $(filter far,$(MAKECMDGOALS)),+modsl,+modsl0)
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
LD_FLAGS   += $(SCRIPT)

#----------------------------------------------------------#

$(info Using '$(MAKECMDGOALS)')

all : $(ELF) print_elf_size

far : all

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

$(SM8) : $(OBJS) $(SCRIPT)
	$(info $@)
	$(LD) -o $@ $(LD_FLAGS) $(OBJS) $(LIBS) $(LIB_DIRS_F)

$(ELF) : $(SM8)
	$(info $@)
	$(DWARF) $<

$(HEX) : $(SM8)
	$(info $@)
	$(DUMP) -o $@ $<

print_elf_size : $(ELF)
	$(info Size of target file:)
	$(SIZE) -B $(ELF)

clean :
	$(info Removing all generated output files)
	$(RM) -Rd $(BUILD)

flash : all $(HEX)
	$(info Programing device...)
	$(STVP) -BoardName=ST-LINK -Port=USB -ProgMode=SWIM -Device=$(TARGET_CHIP) -verif -no_loop -no_log -FileProg=$(HEX)

.PHONY : all far lib clean flash
