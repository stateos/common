ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?=
GCC        ?= /c/sys/gcc/avr/bin/
PROGRAM    := atprogram
OPTF       ?=
STDC       ?= 11
STDCXX     ?= 11

#----------------------------------------------------------#

ifeq ($(strip $(PROJECT)),)
PROJECT    := $(firstword $(notdir $(CURDIR)))
endif

#----------------------------------------------------------#

AS         := $(GCC)avr-gcc -x assembler-with-cpp
CC         := $(GCC)avr-gcc
CXX        := $(GCC)avr-g++
COPY       := $(GCC)avr-objcopy
DUMP       := $(GCC)avr-objdump
SIZE       := $(GCC)avr-size
LD         := $(GCC)avr-g++
AR         := $(GCC)avr-ar

RM         ?= rm -f

#----------------------------------------------------------#

BIN        := $(PROJECT).bin
EEP        := $(PROJECT).eep
ELF        := $(PROJECT).elf
HEX        := $(PROJECT).hex
LIB        := lib$(PROJECT).a
LSS        := $(PROJECT).lss
MAP        := $(PROJECT).map

OBJS       := $(SRCS:%=%.o)
DEPS       := $(OBJS:.o=.d)
LSTS       := $(OBJS:.o=.lst)

#----------------------------------------------------------#

GENERATED  := $(BIN) $(EEP) $(ELF) $(HEX) $(LIB) $(LSS) $(MAP) $(DEPS) $(LSTS) $(OBJS)

#----------------------------------------------------------#

ifneq (clean,$(MAKECMDGOALS))
COMMON_F   += -mmcu=$(TARGET_DEVICE)
COMMON_F   += -O$(OPTF) -ffunction-sections -fdata-sections
COMMON_F   += -MD -MP
COMMON_F   +=#-Wa,-amhls=$(@:.o=.lst)
COMMON_F   += -mno-gas-isr-prologues
AS_FLAGS   +=
ifneq ($(filter ISO,$(DEFS)),)
$(info Using iso)
DEFS       := $(DEFS:ISO=)
C_FLAGS    += -std=c$(STDC:20=2x)
CXX_FLAGS  += -std=c++$(STDCXX:20=2a)
else
C_FLAGS    += -std=gnu$(STDC:20=2x)
CXX_FLAGS  += -std=gnu++$(STDCXX:20=2a)
endif
CXX_FLAGS  += -fno-rtti -fno-use-cxa-atexit
LD_FLAGS   += -Wl,-Map=$(MAP),--cref,--no-warn-mismatch,--gc-sections
ifneq ($(filter EXCEPTIONS,$(DEFS)),)
$(info Using exceptions)
DEFS       := $(DEFS:EXCEPTIONS=)
else
CXX_FLAGS  += -fno-exceptions
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
COMMON_F   += -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wlogical-op
CXX_FLAGS  += -Wzero-as-null-pointer-constant
endif
ifneq ($(filter DEBUG,$(DEFS)),)
$(info Using debug)
COMMON_F   += -g -ggdb
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

AS_FLAGS   += $(COMMON_F) $(DEFS_F) $(INCS_F)
C_FLAGS    += $(COMMON_F) $(DEFS_F) $(INCS_F)
CXX_FLAGS  += $(COMMON_F) $(DEFS_F) $(INCS_F)
LD_FLAGS   += $(COMMON_F)

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

$(ELF) : $(OBJS)
	$(info $@)
	$(LD) $(LD_FLAGS) $(OBJS) $(LIBS) $(LIB_DIRS_F) -o $@

$(LIB) : $(OBJS)
	$(info $@)
	$(AR) -r $@ $?

$(BIN) : $(ELF)
	$(info $@)
	$(COPY) -O binary $< $@

$(HEX) : $(ELF)
	$(info $@)
	$(COPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

$(EEP) : $(ELF)
	$(info $@)
	$(COPY) -O ihex -j .eeprom --set-section-flags ".eeprom=alloc,load" --change-section-lma ".eeprom=0" $< $@

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

flash : all
	$(info Programing device...)
	$(PROGRAM) -t medbg -i updi -d $(TARGET_DEVICE) program -c -fl -f $(ELF) --verify

eeprom : all # $(EEP)
	$(info Programing device...)
	$(PROGRAM) -t medbg -i updi -d $(TARGET_DEVICE) program -ee -f $(ELF) --verify

reset :
	$(info Reseting device...)
	$(PROGRAM) -t medbg -i updi -d $(TARGET_DEVICE) reset

.PHONY : all lib clean flash eeprom reset

-include $(DEPS)
