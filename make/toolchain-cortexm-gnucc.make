ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?= # project name
BUILD      ?= # build folder name
GNUCC      ?= # toolchain path
OPENOCD    := openocd
STLINK     := st-link_cli
CUBE       := stm32_programmer_cli
QEMU       := qemu-system-gnuarmeclipse
OPTF       ?=
STDC       ?= 20
STDCXX     ?= 20

#----------------------------------------------------------#

PROJECT    := $(firstword $(PROJECT) $(notdir $(CURDIR)))
BUILD      := $(firstword $(BUILD) build)

#----------------------------------------------------------#

AS         := $(GNUCC)arm-none-eabi-gcc
CC         := $(GNUCC)arm-none-eabi-gcc
CXX        := $(GNUCC)arm-none-eabi-g++
FC         := $(GNUCC)arm-none-eabi-gfortran
COPY       := $(GNUCC)arm-none-eabi-objcopy
DUMP       := $(GNUCC)arm-none-eabi-objdump
SIZE       := $(GNUCC)arm-none-eabi-size
LD         := $(GNUCC)arm-none-eabi-g++
AR         := $(GNUCC)arm-none-eabi-ar
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
COMMON_F   += -mthumb -mcpu=$(TARGET_CORE)
COMMON_F   += -ffunction-sections -fdata-sections
COMMON_F   += -MD -MP
COMMON_F   +=#-Wa,-amhls=$(@:.o=.lst)
AS_FLAGS   += -xassembler-with-cpp
C_FLAGS    += -Wlogical-op
CXX_FLAGS  += -Wlogical-op -fno-use-cxa-atexit
F_FLAGS    += -cpp
LD_FLAGS   += -Wl,-Map=$(MAP),--gc-sections
ifneq ($(filter ISO,$(DEFS)),)
$(info Using iso)
DEFS       := $(DEFS:ISO=)
C_FLAGS    += -std=c$(STDC:20=2x)
CXX_FLAGS  += -std=c++$(STDCXX:20=2a)
else
C_FLAGS    += -std=gnu$(STDC:20=2x)
CXX_FLAGS  += -std=gnu++$(STDCXX:20=2a)
endif
ifneq ($(filter main_stack_size%,$(DEFS)),)
LD_FLAGS   += -Wl,--defsym=$(filter main_stack_size%,$(DEFS))
endif
ifneq ($(filter proc_stack_size%,$(DEFS)),)
LD_FLAGS   += -Wl,--defsym=$(filter proc_stack_size%,$(DEFS))
endif
ifneq ($(filter flash,$(MAKECMDGOALS)),)
DEFS       := $(DEFS:SEMIHOST=)
endif
ifneq ($(filter monitor,$(MAKECMDGOALS)),)
DEFS       += SEMIHOST
endif
ifneq ($(filter NOSTARTFILES,$(DEFS)),)
$(info Using nostartfiles)
DEFS       := $(DEFS:NOSTARTFILES=)
LD_FLAGS   += -nostartfiles
endif
ifneq ($(filter EXCEPTIONS,$(DEFS)),)
$(info Using exceptions)
DEFS       := $(DEFS:EXCEPTIONS=)
else
CXX_FLAGS  += -fno-rtti -fno-exceptions
endif
ifneq ($(filter STDLIB,$(DEFS)),)
$(info Using stdlib)
DEFS       := $(DEFS:STDLIB=)
else
$(info Using nanolib)
LD_FLAGS   += -specs=nano.specs
endif
ifneq ($(filter NOSYS,$(DEFS)),)
$(info Using nosys)
DEFS       := $(DEFS:NOSYS=) __NOSYS
LD_FLAGS   += -specs=nosys.specs
endif
ifneq ($(filter SEMIHOST,$(DEFS)),)
$(info Using semihosting)
DEFS       := $(DEFS:SEMIHOST=) __SEMIHOST
LD_FLAGS   += -specs=rdimon.specs
endif
ifneq ($(filter LTO,$(DEFS)),)
$(info Using lto)
DEFS       := $(DEFS:LTO=)
COMMON_F   += -flto
endif
ifneq ($(filter NOWARNINGS,$(DEFS)),)
$(info Using nowarnings)
DEFS       := $(DEFS:NOWARNINGS=)
COMMON_F   += -Wall
else
COMMON_F   += -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wsign-conversion
CXX_FLAGS  += -Wzero-as-null-pointer-constant
endif
ifneq ($(filter DEBUG,$(DEFS)),)
$(info Using debug)
COMMON_F   += -O$(OPTF) -g -ggdb
DEFS       := $(DEFS:NDEBUG=)
DEFS       := $(DEFS:MINSIZE=)
else
ifeq  ($(filter NDEBUG,$(DEFS)),)
DEFS       += NDEBUG
endif
ifneq ($(filter MINSIZE,$(DEFS)),)
$(info Using minsize)
DEFS       := $(DEFS:MINSIZE=)
COMMON_F   += -Os
else
COMMON_F   += -O$(OPTF)
endif
endif
endif

#----------------------------------------------------------#

DEFS_F     := $(DEFS:%=-D%)
INCS_F     := $(INCS:%=-I%) $(PREINC:%=-include %)
LIB_DIRS_F := $(LIB_DIRS:%=-L%)
SCRIPT_F   := $(SCRIPT:%=-T%)

AS_FLAGS   += $(COMMON_F) $(DEFS_F) $(INCS_F)
C_FLAGS    += $(COMMON_F) $(DEFS_F) $(INCS_F)
CXX_FLAGS  += $(COMMON_F) $(DEFS_F) $(INCS_F)
F_FLAGS    += $(COMMON_F) $(DEFS_F) $(INCS_F)
LD_FLAGS   += $(COMMON_F) $(LIB_DIRS_F) $(SCRIPT_F)

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

$(BUILD)/%.S.o : /%.S
	$(info $<)
	mkdir -p $(dir $@)
	$(AS) $(AS_FLAGS) -c $< -o $@

$(BUILD)/%.s.o : /%.s
	$(info $<)
	mkdir -p $(dir $@)
	$(AS) $(AS_FLAGS) -c $< -o $@

$(BUILD)/%.c.o : /%.c
	$(info $<)
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD)/%.cc.o : /%.cc
	$(info $<)
	mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(BUILD)/%.cpp.o : /%.cpp
	$(info $<)
	mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(BUILD)/%.f.o : /%.f
	$(info $<)
	mkdir -p $(dir $@)
	$(FC) $(F_FLAGS) -c $< -o $@

$(ELF) : $(OBJS) $(SCRIPT)
	$(info $@)
	$(LD) $(LD_FLAGS) $(OBJS) $(LIBS) -o $@

$(LIB) : $(OBJS)
	$(info $@)
	$(AR) -r $@ $?

$(BIN) : $(ELF)
	$(info $@)
	$(COPY) -O binary $< $@

$(HEX) : $(ELF)
	$(info $@)
	$(COPY) -O ihex $< $@

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
#	$(STLINK) -Q -c SWD UR -P $(HEX) -V -HardRst

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
