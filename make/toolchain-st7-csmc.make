ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?=
CSMC       ?= /c/sys/cosmic/cxst7/
CSTM       ?= /c/sys/cosmic/cxstm8/

#----------------------------------------------------------#

ifeq ($(strip $(PROJECT)),)
PROJECT    := $(firstword $(notdir $(CURDIR)))
endif

#----------------------------------------------------------#

AS         := $(CSMC)cxst7
CC         := $(CSMC)cxst7
COPY       := $(CSMC)cvdwarf
DUMP       := $(CSMC)chex
LD         := $(CSMC)clnk
AR         := $(CSTM)clib
LABS       := $(CSMC)clabs
SIZE       := size

RM         ?= rm -f

#----------------------------------------------------------#

ELF        := $(PROJECT).elf
HEX        := $(PROJECT).s19
LIB        := $(PROJECT).st7
MAP        := $(PROJECT).map
ST7        := $(PROJECT).st7

OBJS       := $(SRCS:.s=.o)
OBJS       := $(OBJS:.c=.o)
LSTS       := $(OBJS:.o=.ls)
TXTS       := $(OBJS:.o=.la)

#----------------------------------------------------------#

GENERATED  := $(ELF) $(HEX) $(LIB) $(MAP) $(ST7) $(LSTS) $(OBJS) $(TXTS)

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
INCS_F     := $(INCS:%=-i%/)
LIB_DIRS_F := $(LIB_DIRS:%=-l%)

AS_FLAGS   += $(DEFS_F)
C_FLAGS    += $(DEFS_F) $(INCS_F)
LD_FLAGS   += $(SCRIPT)

#----------------------------------------------------------#

$(info Using '$(MAKECMDGOALS)')

all : $(ELF) print_elf_size

lib : $(OBJS)
	$(info $(LIB))
	$(AR) -c -p $(LIB) $?

$(OBJS) : $(MAKEFILE_LIST)

%.o : %.s
	$(AS) $(AS_FLAGS) $<

%.o : %.c
	$(CC) $(C_FLAGS) $<

$(ST7) : $(OBJS) $(SCRIPT)
	$(info $@)
	$(LD) -o $@ $(LD_FLAGS) $(OBJS) $(LIBS) $(LIB_DIRS_F)

$(ELF) : $(ST7)
	$(info $@)
	$(COPY) $<

$(HEX) : $(SM8)
	$(info $@)
	$(DUMP) -o $@ $<

print_elf_size : $(ELF)
	$(info Size of target file:)
	$(SIZE) -B $(ELF)

clean :
	$(info Removing all generated output files)
	$(RM) $(GENERATED)

.PHONY : all lib clean
