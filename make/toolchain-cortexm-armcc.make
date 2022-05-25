ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?= # project name
ARMCC      ?= # toolchain path
GNUCC      ?= # toolchain path
OPENOCD    := openocd
STLINK     := st-link_cli
CUBE       := stm32_programmer_cli
QEMU       := qemu-system-gnuarmeclipse
OPTF       ?= 2
STDC       ?= 99
STDCXX     ?= 11
BUILD      ?= build

#----------------------------------------------------------#

iifeq ($(PROJECT),)
PROJECT    := $(firstword $(PROJECT) $(notdir $(CURDIR)))
endif

feq ($(BUILD),)
$(error Invalid BUILD definition)
endif

#----------------------------------------------------------#

AS         := $(ARMCC)armasm
CC         := $(ARMCC)armcc
CXX        := $(ARMCC)armcc
COPY       := $(ARMCC)fromelf
DUMP       := $(ARMCC)fromelf
SIZE       := $(GNUCC)arm-none-eabi-size
LD         := $(ARMCC)armlink
AR         := $(ARMCC)armar
GDB        := $(GNUCC)arm-none-eabi-gdb

RM         ?= rm -f

#----------------------------------------------------------#

ELF        := $(BUILD)/$(PROJECT).axf
LIB        := $(BUILD)/$(PROJECT).lib
BIN        := $(BUILD)/$(PROJECT).bin
HEX        := $(BUILD)/$(PROJECT).hex
FED        := $(BUILD)/$(PROJECT).fed
LSS        := $(BUILD)/$(PROJECT).lss
MAP        := $(BUILD)/$(PROJECT).map

SRCS       := $(foreach s,$(SRCS),$(realpath $s))
OBJS       := $(SRCS:%=$(BUILD)%.o)
DEPS       := $(OBJS:.o=.d)

#----------------------------------------------------------#

INCS       += $(abspath $(ARMCC)../../RV31/INC)

#----------------------------------------------------------#

ifneq (clean,$(MAKECMDGOALS))
COMMON_F   += --thumb --cpu=$(TARGET_CORE)
COMMON_F   += --md --depend=$(@:.o=.d) # --feedback=$(FED)
OPTIM_F    += --split_sections # --interleave
AS_FLAGS   +=#--list=$(@:.o=.lst)
C_FLAGS    += --c$(STDC)     --forceinline # --asm --asm_dir=$(@D) # --list --list_dir=$(@D)
CXX_FLAGS  += --cpp$(STDCXX) --forceinline # --asm --asm_dir=$(@D) # --list --list_dir=$(@D)
LD_FLAGS   += --strict
LD_FLAGS   += --map --list=$(MAP) --symbols --list_mapping_symbols --info common,sizes,totals,veneers,unused
ifneq ($(filter STDLIB,$(DEFS)),)
$(info Using stdlib)
DEFS       := $(DEFS:STDLIB=)
else
$(info Using microlib)
DEFS       += __MICROLIB
LD_FLAGS   += --library_type=microlib
endif
ifneq ($(filter DEBUG,$(DEFS)),)
$(info Using debug)
OPTIMF_F   += -O$(OPTF) --debug
DEFS       := $(DEFS:NDEBUG=)
DEFS       := $(DEFS:MINSIZE=)
else
ifneq ($(filter MINSIZE,$(DEFS)),)
$(info Using minsize)
DEFS       := $(DEFS:MINSIZE=)
OPTIMF_F   += -Ospace
else
OPTIMF_F   += -O$(OPTF)
endif
ifeq  ($(filter NDEBUG,$(DEFS)),)
DEFS       += NDEBUG
endif
endif
endif

#----------------------------------------------------------#

empty=
comma=,
space=$(empty) $(empty)
clist=$(subst $(space),$(comma),$(strip $1))

#----------------------------------------------------------#

DEFS_F     := $(DEFS:%=-D%)
INCS_F     := $(INCS:%=-I%)
DEFS_L     := $(DEFS_F:%=--pd=%)
INCS_L     := $(INCS_F:%=--pd=%)
LIB_DIRS   += $(abspath $(ARMCC)../../RV31/LIB)
LIB_DIRS_F := --libpath=$(abspath $(ARMCC)../lib/)
LIB_DIRS_F += --userlibpath=$(call clist,$(LIB_DIRS))
SCRIPT_F   := $(SCRIPT:%=--scatter=%)

AS_FLAGS   +=#$(foreach d,$(DEFS),$(if $(findstring =,$d),--pd="$(subst =, SETA ,$d)",--pd="$d SETA 1"))
AS_FLAGS   += $(COMMON_F) --cpreproc --cpreproc_opts=$(call clist,$(DEFS_F) $(INCS_F))
C_FLAGS    += $(COMMON_F) $(DEFS_F) $(INCS_F) $(OPTIM_F)
CXX_FLAGS  += $(COMMON_F) $(DEFS_F) $(INCS_F) $(OPTIM_F)
LD_FLAGS   += $(SCRIPT_F) $(DEFS_L) $(INCS_L) $(LIB_DIRS_F)

#----------------------------------------------------------#

#openocd command-line
#interface and board/target settings (using the OOCD target-library here)
OOCD_INIT  := -f board/$(OOCD_BOARD).cfg
OOCD_INIT  += -c init
OOCD_INIT  += -c targets
#commands to enable semihosting
OOCD_DEBG  := -c "arm semihosting enable"
#commands to prepare flash-write
OOCD_SAVE  := -c "reset halt"
#flash-write and -verify
OOCD_SAVE  += -c "flash write_image erase $(ELF)"
OOCD_SAVE  += -c "verify_image $(ELF)"
#reset target
OOCD_EXEC  := -c "reset run"
#terminate OOCD after programming
OOCD_EXIT  := -c shutdown

#gdb command line
DEBUG_CMD  := -ex "target remote localhost:3333"
DEBUG_CMD  += -ex "mon reset halt"
DEBUG_CMD  += -ex "tbreak main"
DEBUG_CMD  += -ex "c"

#----------------------------------------------------------#

$(info Using '$(MAKECMDGOALS)')

all : $(ELF) $(LSS) print_elf_size

lib : $(LIB) print_size

$(OBJS) : $(MAKEFILE_LIST)

$(BUILD)/%.s.o : /%.s
	$(info $<)
	mkdir -p $(dir $@)
	$(AS) $(AS_FLAGS) $< -o $@

$(BUILD)/%.c.o : /%.c
	$(info $<)
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD)/%.cpp.o : /%.cpp
	$(info $<)
	mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(ELF) : $(OBJS) $(SCRIPT)
	$(info $@)
	$(LD) $(LD_FLAGS) $(OBJS) $(LIBS) -o $@

$(LIB) : $(OBJS)
	$(info $@)
	$(AR) -r $@ $?

$(BIN) : $(ELF)
	$(info $@)
	$(COPY) $< --bincombined --output $@

$(HEX) : $(ELF)
	$(info $@)
	$(COPY) $< --i32combined --output $@

$(LSS) : $(ELF)
	$(info $@)
	$(DUMP) $< --text -c -z --output $@

print_size : $(OBJS)
	$(info Size of modules:)
	$(SIZE) -B -t --common $?

print_elf_size : $(ELF)
	$(info Size of target:)
	$(SIZE) -B $<

clean :
	$(info Removing all generated output files)
	$(RM) -Rd $(BUILD)

flash : all
	$(info Programing device...)
	$(OPENOCD) $(OOCD_INIT) $(OOCD_SAVE) $(OOCD_EXEC) $(OOCD_EXIT)
#	$(CUBE) -q -c port=SWD mode=UR -w $(ELF) -v -hardRst
#	$(STLINK) -Q -c SWD UR -P $(HEX) -V -Rst

server : all
	$(info Starting server...)
	$(OPENOCD) $(OOCD_INIT) $(OOCD_SAVE)

debug : all
	$(info Debugging device...)
	$(GDB) --nx $(DEBUG_CMD) $(ELF)

monitor : all
	$(info Monitoring device...)
	$(OPENOCD) $(OOCD_INIT) $(OOCD_SAVE) $(OOCD_DEBG) $(OOCD_EXEC)

qemu : all
	$(info Emulating device...)
	$(QEMU) -semihosting -board $(QEMU_BOARD) -image $(ELF)

reset :
	$(info Reseting device...)
	$(OPENOCD) $(OOCD_INIT) $(OOCD_EXEC) $(OOCD_EXIT)
#	$(CUBE) -q -c port=SWD mode=UR -hardRst
#	$(STLINK) -Q -c SWD UR -HardRst

.PHONY : all lib clean flash server debug monitor qemu reset

-include $(DEPS)
