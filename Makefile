#toolchain: 
#gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu
#gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabi

PWD := $(shell pwd)

UBOOT_CFG := catdrive.config
DTB := armada-3720-catdrive

DDR := $(PWD)/mv-ddr-marvell
A3700_TOOLS := $(PWD)/A3700-utils-marvell
BL33 := $(PWD)/u-boot-marvell/u-boot.bin

MAKE_ARCH := make CROSS_COMPILE=aarch64-linux-gnu- CROSS_CM3=arm-linux-gnueabi-

all: uboot atf

uboot-config:
	cp -f u-boot-marvell/$(UBOOT_CFG) u-boot-marvell/.config

uboot: uboot-config
	$(MAKE_ARCH) -C u-boot-marvell DEVICE_TREE=$(DTB)

atf: uboot
	$(MAKE_ARCH) -C atf-marvell \
		MV_DDR_PATH=$(DDR) WTP=$(A3700_TOOLS) BL33=$(BL33) \
		CLOCKSPRESET=CPU_1000_DDR_800 DDR_TOPOLOGY=0 \
		BOOTDEV=SPINOR PARTNUM=0 PLAT=a3700 DEBUG=0 \
		USE_COHERENT_MEM=0 LOG_LEVEL=20 SECURE=0 \
		all fip

clean:
	$(MAKE_ARCH) -C u-boot-marvell clean
	$(MAKE_ARCH) -C atf-marvell distclean
