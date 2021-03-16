ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?=
IARC       ?=
STVP       := stvp_cmdline
OPTF       ?=
BUILD      ?= build

#----------------------------------------------------------#

PROJECT    := $(firstword $(PROJECT) $(notdir $(CURDIR)))

#----------------------------------------------------------#

AS         := $(IARC)iasmstm8
CC         := $(IARC)iccstm8
CXX        := $(IARC)iccstm8
COPY       := $(IARC)ielftool
DUMP       := $(IARC)ielfdumpstm8
LD         := $(IARC)ilinkstm8
AR         := $(IARC)iarchive
SIZE       := size

RM         ?= rm -f

#----------------------------------------------------------#

ELF        := $(if $(BUILD),$(BUILD)/,)$(PROJECT).elf
LIB        := $(if $(BUILD),$(BUILD)/,)lib$(PROJECT).a
BIN        := $(if $(BUILD),$(BUILD)/,)$(PROJECT).bin
HEX        := $(if $(BUILD),$(BUILD)/,)$(PROJECT).hex
LSS        := $(if $(BUILD),$(BUILD)/,)$(PROJECT).lss
MAP        := $(if $(BUILD),$(BUILD)/,)$(PROJECT).map

SRCS       := $(foreach s,$(SRCS),$(realpath $s))
OBJS       := $(SRCS:%=$(BUILD)%.o)
DEPS       := $(OBJS:.o=.d)
LSTS       := $(OBJS:.o=.lst)

#----------------------------------------------------------#

GENERATED  := $(ELF) $(LIB) $(BIN) $(HEX) $(LSS) $(MAP)
GENERATED  += $(OBJS) $(DEPS) $(LSTS)

#----------------------------------------------------------#

ifneq (clean,$(MAKECMDGOALS))
CORE_F     += --core=stm8 --code_model=$(if $(filter far,$(MAKECMDGOALS)),large,small)
COMMON_F   += -O$(OPTF) --dependencies=m $(BUILD)$<.d
AS_FLAGS   += --silent
C_FLAGS    += --silent -e
CXX_FLAGS  += --silent -e --ec++
LD_FLAGS   += --silent --config $(SCRIPT) --map $(MAP)
ifneq ($(filter stack_size%,$(DEFS)),)
LD_FLAGS   += --config_def $(filter stack_size%,$(DEFS))
endif
ifneq ($(filter DEBUG,$(DEFS)),)
$(info Using debug)
COMMON_F   += --debug
else
ifeq  ($(filter NDEBUG,$(DEFS)),)
DEFS       += NDEBUG
endif
endif
endif

#----------------------------------------------------------#

DEFS_F     := $(DEFS:%=-D%)
INCS_F     := $(INCS:%=-I%/)
LIB_DIRS_F := $(LIB_DIRS:%=-L %)

AS_FLAGS   += $(CORE_F)             $(DEFS_F) $(INCS_F)
C_FLAGS    += $(CORE_F) $(COMMON_F) $(DEFS_F) $(INCS_F)
CXX_FLAGS  += $(CORE_F) $(COMMON_F) $(DEFS_F) $(INCS_F)
LD_FLAGS   +=

#----------------------------------------------------------#

$(info Using '$(MAKECMDGOALS)')

all : $(ELF) $(LSS) print_elf_size

far : all

lib : $(LIB) print_size

$(OBJS) : $(MAKEFILE_LIST)

$(BUILD)/%.s.o : /%.s
	$(info $<)
	mkdir -p $(dir $@)
	$(AS) $(AS_FLAGS) $< -o $@

$(BUILD)/%.c.o : /%.c
	$(info $<)
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) $< -o $@

$(BUILD)/%.cpp.o : /%.cpp
	$(info $<)
	mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) $< -o $@

$(ELF) : $(OBJS) $(SCRIPT)
	$(info $@)
	$(LD) $(LD_FLAGS) $(OBJS) $(LIBS) $(LIB_DIRS_F) -o $@

$(LIB) : $(OBJS)
	$(info $@)
	$(AR) --create $@ $?

$(BIN) : $(ELF)
	$(info $@)
	$(COPY) --silent --strip --bin $^ $@

$(HEX) : $(ELF)
	$(info $@)
	$(COPY) --silent --strip --ihex $^ $@

$(LSS) : $(ELF)
	$(info $@)
	$(DUMP) --all $< -o $@

print_size : $(OBJS)
	$(info Size of modules:)
	$(SIZE) -B -t --common $(OBJS)

print_elf_size : $(ELF)
	$(info Size of target file:)
	$(SIZE) -B $(ELF)

clean :
	$(info Removing all generated output files)
	$(RM) $(if $(BUILD),-Rd $(BUILD),$(GENERATED))

flash : all $(HEX)
	$(info Programing device...)
	$(STVP) -BoardName=ST-LINK -Port=USB -ProgMode=SWIM -Device=$(TARGET_CHIP) -verif -no_loop -no_log -FileProg=$(HEX)

.PHONY : all far lib clean flash

-include $(DEPS)
