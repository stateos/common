ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?=
SDCC       ?=
STVP       := stvp_cmdline
STDC       ?= 11
BUILD      ?= build

#----------------------------------------------------------#

ifeq ($(BUILD),)
$(error Invalid BUILD definition)
endif

#----------------------------------------------------------#

PROJECT    := $(firstword $(PROJECT) $(notdir $(CURDIR)))

#----------------------------------------------------------#

AS         := $(SDCC)sdasstm8
CC         := $(SDCC)sdcc
LD         := $(SDCC)sdld
AR         := $(SDCC)sdar
COPY       := $(SDCC)sdobjcopy
DBG        := $(SDCC)sdcdb
SIM        := $(SDCC)sstm8
SIZE       := size

RM         ?= rm -f

#----------------------------------------------------------#

ELF        := $(BUILD)/$(PROJECT).elf
LIB        := $(BUILD)/$(PROJECT).lib
HEX        := $(BUILD)/$(PROJECT).hex
MAP        := $(BUILD)/$(PROJECT).map

SRCS       := $(foreach s,$(SRCS),$(realpath $s))
OBJS       := $(SRCS:%=$(BUILD)%.rel)
DEPS       := $(OBJS:.rel=.d)

#----------------------------------------------------------#

ifneq (clean,$(MAKECMDGOALS))
COMMON_F   += -m$(TARGET_CORE) $(if $(filter far,$(MAKECMDGOALS)),--model-large,)
AS_FLAGS   += -l -o -s
ifneq ($(filter ISO,$(DEFS)),)
$(info Using iso)
DEFS       := $(DEFS:ISO=)
C_FLAGS    += --std-c$(STDC:20=2x)
else
C_FLAGS    += --std-sdcc$(STDC:20=2x)
endif
C_FLAGS    += -MD
LD_FLAGS   +=
ifneq ($(filter NOWARNINGS,$(DEFS)),)
$(info Using nowarnings)
DEFS       := $(DEFS:NOWARNINGS=)
COMMON_F   += --less-pedantic
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
INCS_F     := $(INCS:%=-I%)
SRC_DIRS   := $(sort $(dir $(SRCS)))
SRC_DIRS_F := $(SRC_DIRS:%/=--directory=%)
LIB_DIRS_F := $(LIB_DIRS:%/=-L%)

C_FLAGS    += $(COMMON_F) $(DEFS_F) $(INCS_F)
LD_FLAGS   += $(COMMON_F)

#----------------------------------------------------------#

$(info Using '$(MAKECMDGOALS)')

all : $(ELF) print_elf_size

far : all

lib : $(LIB)

$(OBJS) : $(MAKEFILE_LIST)

$(BUILD)/%.s.rel : /%.s
	$(info $<)
	mkdir -p $(dir $@)
	$(AS) $(AS_FLAGS) $@ $<

$(BUILD)/%.c.rel : /%.c
	$(info $<)
	mkdir -p $(dir $@)
	$(CC) -c $(C_FLAGS) $< -o $@

$(ELF) : $(OBJS)
	$(info $@)
	$(CC) --out-fmt-elf $(LD_FLAGS) $(OBJS) $(LIBS) $(LIB_DIRS_F) -o $@

$(LIB) : $(OBJS)
	$(info $@)
	$(AR) -r $@ $?

$(HEX) : $(OBJS)
	$(info $@)
	$(CC) $(LD_FLAGS) $(OBJS) -o $@

print_elf_size : $(ELF)
	$(info Size of target file:)
	$(SIZE) -B $(ELF)

clean :
	$(info Removing all generated output files)
	$(RM) -Rd $(BUILD)

flash : all $(HEX)
	$(info Programing device...)
	$(STVP) -BoardName=ST-LINK -Port=USB -ProgMode=SWIM -Device=$(TARGET_CHIP) -verif -no_loop -no_log -FileProg=$(HEX)

debug : all $(HEX)
	$(info Debugging device...)
	$(DBG) $(PROJECT) $(SRC_DIRS_F)
#	$(SIM) $(HEX)

.PHONY : all far lib clean flash debug

-include $(DEPS)
