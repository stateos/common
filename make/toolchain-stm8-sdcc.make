ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?=
SDCC       ?= /c/sys/sdcc/bin/
STVP       := stvp_cmdline
STDC       ?= 11

#----------------------------------------------------------#

ifeq ($(strip $(PROJECT)),)
PROJECT    := $(firstword $(notdir $(CURDIR)))
endif

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

ELF        := $(PROJECT).elf
HEX        := $(PROJECT).hex
LIB        := $(PROJECT).lib
MAP        := $(PROJECT).map
CDB        := $(PROJECT).cdb
LKF        := $(PROJECT).lk

OBJS       := $(SRCS:%=%.rel)
ASMS       := $(OBJS:.rel=.asm)
LSTS       := $(OBJS:.rel=.lst)
RSTS       := $(OBJS:.rel=.rst)
SYMS       := $(OBJS:.rel=.sym)
ADBS       := $(OBJS:.rel=.adb)
DEPS       := $(OBJS:.rel=.d)

#----------------------------------------------------------#

GENERATED  := $(BIN) $(ELF) $(HEX) $(LIB) $(LSS) $(MAP) $(CDB) $(LKF) $(LSTS) $(OBJS) $(ASMS) $(DEPS) $(LSTS) $(RSTS) $(SYMS) $(ADBS)

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
INCS_F     := $(INCS:%=-I%/)
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

%.s.rel : %.s
	$(info $<)
	$(AS) $(AS_FLAGS) $@ $<

%.c.rel : %.c
	$(info $<)
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
	$(RM) $(GENERATED)

flash : all $(HEX)
	$(info Programing device...)
	$(STVP) -BoardName=ST-LINK -Port=USB -ProgMode=SWIM -Device=$(TARGET_CHIP) -verif -no_loop -no_log -FileProg=$(HEX)

debug : all $(HEX)
	$(info Debugging device...)
	$(DBG) $(PROJECT) $(SRC_DIRS_F)
#	$(SIM) $(HEX)

.PHONY : all far lib clean flash debug

-include $(DEPS)
