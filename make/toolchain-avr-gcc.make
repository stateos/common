ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

#----------------------------------------------------------#

PROJECT    ?= # project name
BUILD      ?= # build folder name
GCC        ?= # toolchain path
PROGRAM    := atprogram
OPTF       ?=
STDC       ?= 11
STDCXX     ?= 11

#----------------------------------------------------------#

PROJECT    := $(firstword $(PROJECT) $(notdir $(CURDIR)))
BUILD      := $(firstword $(BUILD) build)

#----------------------------------------------------------#

AS         := $(GCC)avr-gcc
CC         := $(GCC)avr-gcc
CXX        := $(GCC)avr-g++
COPY       := $(GCC)avr-objcopy
DUMP       := $(GCC)avr-objdump
SIZE       := $(GCC)avr-size
LD         := $(GCC)avr-g++
AR         := $(GCC)avr-ar

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
COMMON_F   += -mmcu=$(TARGET_DEVICE)
COMMON_F   += -ffunction-sections -fdata-sections
COMMON_F   += -MD -MP
COMMON_F   +=#-Wa,-amhls=$(@:.o=.lst)
COMMON_F   += -mno-gas-isr-prologues
AS_FLAGS   += -xassembler-with-cpp
C_FLAGS    += -Wlogical-op
CXX_FLAGS  += -Wlogical-op -fno-use-cxa-atexit
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
ifneq ($(filter EXCEPTIONS,$(DEFS)),)
$(info Using exceptions)
DEFS       := $(DEFS:EXCEPTIONS=)
else
CXX_FLAGS  += -fno-rtti -fno-exceptions
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
INCS_F     := $(INCS:%=-I%)
LIB_DIRS_F := $(LIB_DIRS:%=-L%)

AS_FLAGS   += $(COMMON_F) $(DEFS_F) $(INCS_F)
C_FLAGS    += $(COMMON_F) $(DEFS_F) $(INCS_F)
CXX_FLAGS  += $(COMMON_F) $(DEFS_F) $(INCS_F)
LD_FLAGS   += $(COMMON_F) $(LIB_DIRS_F)

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

$(BUILD)/%.cpp.o : /%.cpp
	$(info $<)
	mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(ELF) : $(OBJS)
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
	$(COPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

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
	$(PROGRAM) -t medbg -i updi -d $(TARGET_DEVICE) program -c -fl -f $(ELF) --verify

eeprom : all
	$(info Programing device...)
	$(PROGRAM) -t medbg -i updi -d $(TARGET_DEVICE) program -ee -f $(ELF) --verify

reset :
	$(info Reseting device...)
	$(PROGRAM) -t medbg -i updi -d $(TARGET_DEVICE) reset

.PHONY : all lib clean flash eeprom reset

-include $(DEPS)
