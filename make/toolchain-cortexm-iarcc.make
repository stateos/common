ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?=
IARCC      ?= /c/sys/icc/arm/bin/
GNUCC      ?=
OPENOCD    := openocd
STLINK     := st-link_cli
CUBE       := stm32_programmer_cli
QEMU       := qemu-system-gnuarmeclipse
OPTF       ?=

#----------------------------------------------------------#

ifeq ($(strip $(PROJECT)),)
PROJECT    := $(firstword $(notdir $(CURDIR)))
endif

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

BIN        := $(PROJECT).bin
ELF        := $(PROJECT).elf
HEX        := $(PROJECT).hex
LIB        := lib$(PROJECT).a
LSS        := $(PROJECT).lss
MAP        := $(PROJECT).map

OBJS       := $(SRCS:%=%.o)
DEPS       := $(OBJS:.o=.d)
LSTS       := $(OBJS:.o=.lst)

#----------------------------------------------------------#

GENERATED  := $(BIN) $(ELF) $(HEX) $(LIB) $(LSS) $(MAP) $(DEPS) $(LSTS) $(OBJS)

#----------------------------------------------------------#

ifneq (clean,$(MAKECMDGOALS))
CORE_F     += --cpu=$(TARGET_CORE)
COMMON_F   += --thumb -O$(OPTF) -e --dependencies=m $<.d
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
LIB_DIRS_F := $(LIB_DIRS:%=-L%)
SCRIPT_F   := $(SCRIPT:%=--config %)

AS_FLAGS   += $(CORE_F)             $(DEFS_F) $(INCS_F)
C_FLAGS    += $(CORE_F) $(COMMON_F) $(DEFS_F) $(INCS_F)
CXX_FLAGS  += $(CORE_F) $(COMMON_F) $(DEFS_F) $(INCS_F)
LD_FLAGS   += $(CORE_F) $(SCRIPT_F)

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

all : $(LSS) print_elf_size

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
	$(DUMP) --demangle -S $< > $@

print_size : $(OBJS)
	$(info Size of modules:)
	$(SIZE) -B -t --common $(OBJS)

print_elf_size : $(ELF)
	$(info Size of target file:)
	$(SIZE) -B $(ELF)

clean :
	$(info Removing all generated output files)
	$(RM) $(GENERATED)

flash : all $(HEX)
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
