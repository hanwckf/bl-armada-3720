#/*
# * ***************************************************************************
# * Copyright (C) 2015 Marvell International Ltd.
# * ***************************************************************************
# * This program is free software: you can redistribute it and/or modify it
# * under the terms of the GNU General Public License as published by the Free
# * Software Foundation, either version 2 of the License, or any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program.  If not, see <http://www.gnu.org/licenses/>.
# * ***************************************************************************
#*/

CROSS_CM3 ?= arm-linux-gnueabi-

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

CPUOPTS  = -mthumb -mcpu=cortex-m3 -mlittle-endian

BINPATH  = build
LDSCRIPT = fuse.ld
INCLUDE  = -I. -I$(COMMON_PATH)

CFLAGS   = -g -gdwarf-2 -Wall -fno-stack-protector $(INCLUDE) -Os $(CPUOPTS)
CPPFLAGS =
ASFLAGS  = -g --gdwarf-2 --warn $(INCLUDE) $(CPUOPTS)
LDFLAGS  = -nostdlib -T $(LDSCRIPT) -Xlinker "--build-id=none"


CSRC = $(wildcard *.c)
CSRC += $(wildcard $(COMMON_PATH)/*.c)
ASRC = $(wildcard *.S)

COBJ   = $(CSRC:.c=.o)
AOBJ   = $(ASRC:.S=.o)

LOAD_ADDR	:= 0x1fff0000

.SILENT:

all: $(COBJ) $(AOBJ) $(BINPATH)
	rm -f ./$(LDSCRIPT)
	$(SED) 's|. = LOAD_ADDR|. = $(LOAD_ADDR)|1' $(COMMON_PATH)/template.ld > ./$(LDSCRIPT)
	$(ECHO) "  CC    $(LDFLAGS)  $(AOBJ)  $(COBJ) -o $(BINPATH)/fuse.elf"
	$(CC) $(LDFLAGS)  $(AOBJ)  $(COBJ) -o $(BINPATH)/fuse.elf
	$(OBJCOPY) -S -O binary $(BINPATH)/fuse.elf $(BINPATH)/fuse.bin
	@if [ `uname -m` = "x86_64" ]; then $(OBJDUMP) -D -S $(BINPATH)/fuse.elf > $(BINPATH)/fuse.dis; fi

%.o: %.c
	$(ECHO) "  CC    $<"
	$(CC) -MMD -MP -c $(CFLAGS) $(CPPFLAGS) -o $@ $(subst .o,.c,$@)

%.o: %.S
	$(ECHO) "  AS    $<"
	$(AS) $(ASFLAGS) -o $@ $(subst .o,.S,$@)

-include $(COBJ:.o=.d)

$(BINPATH):
	@$(MKDIR) $(BINPATH)

clean:
	$(ECHO) "  CLEAN"
	@$(RM) -rf $(BINPATH)
	@$(RM) -f *.o $(COMMON_PATH)/*.o *.elf *.bin *dis