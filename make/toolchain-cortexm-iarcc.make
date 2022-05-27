ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?= # project name
BUILD      ?= # build folder name
IARCC      ?= # toolchain path
GNUCC      ?= # toolchain path
OPENOCD    := openocd
STLINK     := st-link_cli
CUBE       := stm32_programmer_cli
QEMU       := qemu-system-gnuarmeclipse
OPTF       ?=

#----------------------------------------------------------#

PROJECT    := $(firstword $(PROJECT) $(notdir $(CURDIR)))
BUILD      := $(firstword $(BUILD) build)

#----------------------------------------------------------#

AS         := $(IARCC)iasmarm
CC         := $(IARCC)iccarm
CXX        := $(IARCC)iccarm
COPY       := $(IARCC)ielftool
DUMP       := $(GNUCC)arm-none-eabi-objdump
SIZE       := $(GNUCC)arm-none-eabi-size
LD         := $(IARCC)ilinkarm
AR         := $(IARCC)iarchive
GDB        := $(GNUCC)arm-none-eabi-gdb

RM         ?= rm -f

#----------------------------------------------------------#

ELF        := $(BUILD)/$(PROJECT).elf
LIB        := $(BUILD)/lib$(PROJECT).a
BIN        := $(BUILD)/$(PROJECT).bin
HEX        := $(BUILD)/$(PROJECT).hex
DMP        := $(BUILD)/$(PROJECT).dmp
LSS        := $(BUILD)/$(PROJECT).lss
MAP        := $(BUILD)/$(PROJECT).map

SRCS       := $(foreach s,$(SRCS),$(realpath $s))
OBJS       := $(SRCS:%=$(BUILD)%.o)
DEPS       := $(OBJS:.o=.d)

#----------------------------------------------------------#

ifneq (clean,$(MAKECMDGOALS))
CORE_F     += --cpu $(TARGET_CORE)
COMMON_F   += --thumb -e --dependencies=m $(BUILD)$<.d
AS_FLAGS   += -S -s+ -w+
C_FLAGS    += --silent
CXX_FLAGS  += --silent --c++ --enable_restrict --no_rtti
LD_FLAGS   += --silent --map $(MAP)
ifneq ($(filter main_stack_size%,$(DEFS)),)
LD_FLAGS   += --config_def $(filter main_stack_size%,$(DEFS))
endif
ifneq ($(filter proc_stack_size%,$(DEFS)),)
LD_FLAGS   += --config_def $(filter proc_stack_size%,$(DEFS))
endif
ifneq ($(filter EXCEPTIONS,$(DEFS)),)
$(info Using exceptions)
DEFS       := $(DEFS:EXCEPTIONS=)
else
CXX_FLAGS  += --no_exceptions
LD_FLAGS   += --no_exceptions
endif
ifneq ($(filter SEMIHOST,$(DEFS)),)
$(info Using semihosting)
DEFS       := $(DEFS:SEMIHOST=) __SEMIHOST
LD_FLAGS   += --semihosting
endif
ifneq ($(filter DEBUG,$(DEFS)),)
$(info Using debug)
COMMON_F   += -O$(OPTF) --debug
DEFS       := $(DEFS:NDEBUG=)
DEFS       := $(DEFS:MINSIZE=)
else
ifneq ($(filter MINSIZE,$(DEFS)),)
$(info Using minsize)
DEFS       := $(DEFS:MINSIZE=)
COMMON_F   += -Ohz
else
COMMON_F   += -O$(OPTF)
endif
ifeq  ($(filter NDEBUG,$(DEFS)),)
DEFS       += NDEBUG
endif
endif
endif

#----------------------------------------------------------#

DEFS_F     := $(DEFS:%=-D%)
INCS_F     := $(INCS:%=-I%)
LIB_DIRS_F := $(LIB_DIRS:%=-L%)
SCRIPT_F   := $(SCRIPT:%=--config %)

AS_FLAGS   += $(CORE_F)             $(DEFS_F) $(INCS_F)
C_FLAGS    += $(CORE_F) $(COMMON_F) $(DEFS_F) $(INCS_F)
CXX_FLAGS  += $(CORE_F) $(COMMON_F) $(DEFS_F) $(INCS_F)
LD_FLAGS   += $(CORE_F) $(SCRIPT_F) $(LIB_DIRS_F)

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

all : $(ELF) $(DMP) $(LSS) print_elf_size

lib : $(LIB) print_size

$(OBJS) : $(MAKEFILE_LIST)

$(BUILD)/%.s.o : /%.s
	$(info $<)
	mkdir -p $(dir $@)
	$(AS) $(AS_FLAGS) -c $< -o $@

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
	$(AR) --create $@ $?

$(BIN) : $(ELF)
	$(info $@)
	$(COPY) --silent --strip --bin $^ $@

$(HEX) : $(ELF)
	$(info $@)
	$(COPY) --silent --strip --ihex $^ $@

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
