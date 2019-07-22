#/*
#* ***************************************************************************
#* Copyright (C) 2017 Marvell International Ltd.
#* ***************************************************************************
#*
#* Redistribution and use in source and binary forms, with or without
#* modification, are permitted provided that the following conditions are met:
#*
#* Redistributions of source code must retain the above copyright notice, this
#* list of conditions and the following disclaimer.
#*
#* Redistributions in binary form must reproduce the above copyright notice,
#* this list of conditions and the following disclaimer in the documentation
#* and/or other materials provided with the distribution.
#*
#* Neither the name of Marvell nor the names of its contributors may be used
#* to endorse or promote products derived from this software without specific
#* prior written permission.
#*
#* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
#* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#* POSSIBILITY OF SUCH DAMAGE.
#*
#***************************************************************************
#*/

CROSS_CM3	?= arm-linux-gnueabi-

LD       = $(CROSS_CM3)ld
CC       = $(CROSS_CM3)gcc
AS       = $(CROSS_CM3)as
OBJCOPY  = $(CROSS_CM3)objcopy
OBJDUMP  = $(CROSS_CM3)objdump
HOSTCC   = gcc

RM       = @rm -rf
MKDIR    = @mkdir -p
CD       = @cd
MV       = @mv
CAT      = @cat
PWD      = @pwd
ECHO     = @echo
SED      = @sed

LOCAL_VERSION_STRING	?= -armada
BUILD_STRING		:= $(shell git log -n 1 --pretty=format:"%h")
VERSION_STRING		:= $(LOCAL_VERSION_STRING)-$(BUILD_STRING)

CPUOPTS  = -mthumb -mcpu=cortex-m3 -mlittle-endian
BINPATH  = build
LDSCRIPT = sys_init.ld
INCLUDE  = -I. -I$(COMMON_PATH)

CFLAGS   = -g -gdwarf-2 -Wall -fno-stack-protector $(INCLUDE) -Os $(CPUOPTS) -DVERSION=\"$(VERSION_STRING)\"
CPPFLAGS =
ASFLAGS  = -g --gdwarf-2 --warn $(INCLUDE) $(CPUOPTS)
LDFLAGS  = -nostdlib -T $(LDSCRIPT) -Xlinker "--build-id=none"

CSRC = $(wildcard *.c)
CSRC += $(wildcard ddr/*.c)
CSRC += $(wildcard $(COMMON_PATH)/*.c)
COBJ   = $(CSRC:.c=.o)

ASRC = $(wildcard *.S)
AOBJ   = $(ASRC:.S=.o)

DEBUG			?= 0
CLOCKSPRESET		?= CPU_800_DDR_800
DDR_TOPOLOGY		?= 0
SRAM_START_ADDR		:= $(shell printf %d 0x1fff0000)

ifeq ($(CLOCKSPRESET), CPU_600_DDR_600)
		WTMI_CLOCK = 0
else ifeq ($(CLOCKSPRESET), CPU_800_DDR_800)
		WTMI_CLOCK = 1
else ifeq ($(CLOCKSPRESET), CPU_1000_DDR_800)
		WTMI_CLOCK = 2
else ifeq ($(CLOCKSPRESET), CPU_1200_DDR_750)
		WTMI_CLOCK = 3
endif

START_ADDR_DEC		:= $(shell printf %d 0x1fff0000)
LOAD_ADDR_DEC		:= $(shell expr $(LOAD_OFFSET) + $(START_ADDR_DEC))
LOAD_ADDR		:= $(shell printf 0x%x $(LOAD_ADDR_DEC))

CFG_HEADER = autoconf.h

define filechk
	@set -e;				\
	echo '  CHK     $@';		\
	mkdir -p $(dir $@);			\
	$(filechk_$(1)) > $@.tmp;		\
	if [ -r $@ ] && cmp -s $@ $@.tmp; then	\
		rm -f $@.tmp;			\
	else					\
		echo '  UPD     $@';	\
		mv -f $@.tmp $@;		\
	fi
endef

# FIX-ME:
# replace the relative path to the DDR topology
# files and scripts
DDR_TOPOLOGY_PATH=../../tim/ddr
DDR_TOPOLOGY_FILE=$(DDR_TOPOLOGY_PATH)/DDR_TOPOLOGY_$(DDR_TOPOLOGY).txt

SCRIPT_PATH=../../script
GET_DDR_PARAMS=$(SCRIPT_PATH)/getddrparams.sh

CONFIG_DDR_TYPE  := $(shell $(GET_DDR_PARAMS) $(DDR_TOPOLOGY_FILE) ddr_type)
CONFIG_BUS_WIDTH := $(shell $(GET_DDR_PARAMS) $(DDR_TOPOLOGY_FILE) ddr_bus_width_index)
CONFIG_SPEED_BIN := $(shell $(GET_DDR_PARAMS) $(DDR_TOPOLOGY_FILE) ddr_speedbin_index)
CONFIG_CS_NUM    := $(shell $(GET_DDR_PARAMS) $(DDR_TOPOLOGY_FILE) ddr_cs_mask)
CONFIG_DEV_CAP   := $(shell $(GET_DDR_PARAMS) $(DDR_TOPOLOGY_FILE) ddr_mem_size_index)

define filechk_autoconf.h
	(echo \#define DEBUG $(DEBUG);\
	echo \#define WTMI_CLOCK $(WTMI_CLOCK);\
	echo \#define CONFIG_DDR_TYPE $(CONFIG_DDR_TYPE);\
	echo \#define CONFIG_BUS_WIDTH $(CONFIG_BUS_WIDTH);\
	echo \#define CONFIG_SPEED_BIN $(CONFIG_SPEED_BIN);\
	echo \#define CONFIG_CS_NUM $(CONFIG_CS_NUM);\
	echo \#define CONFIG_DEV_CAP $(CONFIG_DEV_CAP);)
endef

.SILENT:
.PHONY: $(CFG_HEADER)

all: $(CFG_HEADER) $(COBJ) $(AOBJ) $(BINPATH)
	rm -f ./$(LDSCRIPT)
	$(SED) 's|. = LOAD_ADDR|. = $(LOAD_ADDR)|1' $(COMMON_PATH)/template.ld > ./$(LDSCRIPT)
	$(ECHO) "  CC    $(LDFLAGS)  $(AOBJ)  $(COBJ) -o $(BINPATH)/sys_init.elf"
	$(CC) $(LDFLAGS)  $(AOBJ)  $(COBJ) -o $(BINPATH)/sys_init.elf
	$(OBJCOPY) -S -O binary $(BINPATH)/sys_init.elf $(BINPATH)/sys_init.bin
	@if [ `uname -m` = "x86_64" ]; then $(OBJDUMP) -D -S $(BINPATH)/sys_init.elf > $(BINPATH)/sys_init.dis; fi

$(CFG_HEADER):
	$(call filechk,autoconf.h)

%.o: %.c $(CFG_HEADER)
	$(ECHO) "  CC    $<"
	$(CC) -MMD -MP -c $(CFLAGS) $(CPPFLAGS) -o $@ $(subst .o,.c,$@)

%.o: %.S $(CFG_HEADER)
	$(ECHO) "  AS    $<"
	$(AS) $(ASFLAGS) -o $@ $(subst .o,.S,$@)

-include $(COBJ:.o=.d)

$(BINPATH):
	@$(MKDIR) $(BINPATH)

clean:
	$(ECHO) "  SYS INIT CLEAN"
	@$(RM) -rf $(BINPATH)
	@$(RM) -f $(CFG_HEADER) *.o *.elf *.bin *.d *dis ddr/*.o ddr/*.d $(COMMON_PATH)/*.o $(LDSCRIPT)
