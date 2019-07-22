CROSS_CM3 ?= arm-linux-gnueabi-

CC       = $(CROSS_CM3)gcc

MKDIR    = @mkdir -p

BINPATH		= build
WTMI_PATH	= $(shell pwd)
COMMON_PATH	= $(WTMI_PATH)/common
WTMI_IMG	?= fuse/build/fuse.bin
SYS_INIT_IMG	:= sys_init/build/sys_init.bin
MULTI_IMG	:= $(BINPATH)/wtmi.bin
WTMI_PAD_IMG	:= $(BINPATH)/wtmi_app_pad.bin

COMPILER_FOUND  = $(shell which $(CC))
TOOLCHAIN_CHECK = \
	if [ -z "$(COMPILER_FOUND)" ]; then \
		echo "***************************************************" >&2; \
		echo "ARM GCC toolchain installation is missing!" >&2; \
		echo "Please install GNU ARM toolchain suitable for" >&2; \
		echo "compiling code for Cortex M3 targets" >&2; \
		echo "On Debian/Ubuntu run the following command for" >&2; \
		echo "the default toolchain installation:" >&2; \
		echo " sudo apt-get install gcc-arm-linux-gnueabi" >&2; \
		echo "If non-default toolchain is used, please export" >&2; \
		echo "the CROSS_CM3 environment variable pointin to it." >&2; \
		echo "For instance, with BASH shell:" >&2; \
		echo " export CROSS_CM3=/opt/arm-cross/bin/arm-linux-gnueabi" >&2; \
		echo "***************************************************" >&2; \
		exit 1; \
	fi

all: tools $(BINPATH) $(WTMI_IMG) $(SYS_INIT_IMG)
	cat $(WTMI_PAD_IMG) $(SYS_INIT_IMG) > $(MULTI_IMG)

$(BINPATH):
	@$(MKDIR) $(BINPATH)

$(SYS_INIT_IMG): $(WTMI_PAD_IMG)
	${MAKE} -C sys_init LOAD_OFFSET=$(shell wc -c < $(WTMI_PAD_IMG)) COMMON_PATH=$(COMMON_PATH) CROSS_CM3=$(CROSS_CM3)

$(WTMI_PAD_IMG): $(WTMI_IMG)
	cp -f $(WTMI_IMG) $(WTMI_PAD_IMG)
	truncate -s %16 $(WTMI_PAD_IMG)

$(WTMI_IMG):
	${MAKE} -C fuse COMMON_PATH=$(COMMON_PATH) CROSS_CM3=$(CROSS_CM3)

clean:
	${Q}${MAKE} -C fuse clean COMMON_PATH=$(COMMON_PATH)
	${Q}${MAKE} -C sys_init clean COMMON_PATH=$(COMMON_PATH)
	@$(RM) -rf $(BINPATH)

tools:
	@$(TOOLCHAIN_CHECK)