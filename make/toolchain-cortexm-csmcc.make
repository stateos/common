ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?=
CSMCC      ?= /c/sys/cosmic/cxstm32/
GNUCC      ?=
OPENOCD    := openocd
STLINK     := st-link_cli
CUBE       := stm32_programmer_cli
QEMU       := qemu-system-gnuarmeclipse

#----------------------------------------------------------#

ifeq ($(strip $(PROJECT)),)
PROJECT    := $(firstword $(notdir $(CURDIR)))
endif

#----------------------------------------------------------#

AS         := $(CSMCC)cxcorm
CC         := $(CSMCC)cxcorm
DWARF      := $(CSMCC)cvdwarf
COPY       := $(GNUCC)arm-none-eabi-objcopy
DUMP       := $(GNUCC)arm-none-eabi-objdump
LABS       := $(CSMCC)clabs
SIZE       := $(GNUCC)arm-none-eabi-size
LD         := $(CSMCC)clnk
AR         := $(CSMCC)clib
GDB        := $(GNUCC)arm-none-eabi-gdb

RM         ?= rm -f

#----------------------------------------------------------#

BIN        := $(PROJECT).bin
CXM        := $(PROJECT).cxm
ELF        := $(PROJECT).elf
HEX        := $(PROJECT).hex
LIB        := $(PROJECT).cxm
MAP        := $(PROJECT).map

OBJS       := $(SRCS:.s=.o)
OBJS       := $(OBJS:.c=.o)
LSTS       := $(OBJS:.o=.ls)
TXTS       := $(OBJS:.o=.la)

#----------------------------------------------------------#

GENERATED  := $(BIN) $(CXM) $(ELF) $(HEX) $(LIB) $(MAP) $(LSTS) $(OBJS) $(TXTS)

#----------------------------------------------------------#

DEFS       += __ARM__
LIBS       += crtsi.cxm libilc.cxm libm.cxm

#----------------------------------------------------------#

ifneq (clean,$(MAKECMDGOALS))
AS_FLAGS   += -l -ep#-ax
C_FLAGS    += -pc99 -l -pad +modlc
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
INCS_F     := $(INCS:%=-i%/)
LIB_DIRS_F := $(LIB_DIRS:%=-l%)

AS_FLAGS   += $(DEFS_F)
C_FLAGS    += $(DEFS_F) $(INCS_F)
LD_FLAGS   += $(SCRIPT)

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

all : $(ELF) print_elf_size

lib : $(OBJS)
	$(info $(LIB))
	$(AR) -c -p $(LIB) $?

$(OBJS) : $(MAKEFILE_LIST)

%.o : %.s
	$(info $<)
	$(AS) $(AS_FLAGS) $<

%.o : %.c
	$(info $<)
	$(CC) $(C_FLAGS) $<

$(CXM) : $(OBJS) $(SCRIPT)
	$(info $@)
	$(LD) -o $@ $(LD_FLAGS) $(OBJS) $(LIBS) $(LIB_DIRS_F)

$(ELF) : $(CXM)
	$(info $@)
	$(DWARF) -o $@ $<

$(BIN) : $(ELF)
	$(info $@)
	$(COPY) -O binary $< $@

$(HEX) : $(ELF)
	$(info $@)
	$(COPY) -O ihex $< $@

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
