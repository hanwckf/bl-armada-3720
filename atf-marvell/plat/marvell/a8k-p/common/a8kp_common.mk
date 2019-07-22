#
# Copyright (C) 2017 Marvell International Ltd.
#
# SPDX-License-Identifier:	BSD-3-Clause
# https://spdx.org/licenses
#
include tools/doimage/doimage.mk

PLAT_FAMILY		:= a8k-p
PLAT_FAMILY_BASE	:= plat/marvell/$(PLAT_FAMILY)
PLAT_INCLUDE_BASE	:= include/plat/marvell/$(PLAT_FAMILY)
PLAT_COMMON_BASE	:= $(PLAT_FAMILY_BASE)/common
MARVELL_DRV_BASE	:= drivers/marvell
MARVELL_COMMON_BASE	:= plat/marvell/common

$(eval $(call add_define,PLAT_FAMILY))

ERRATA_A72_859971       := 1

# Enable MSS support for a8kp family
MSS_SUPPORT		:= 1

ifeq (${PALLADIUM},1)
CP_NUM			:= 0
else
CP_NUM			:= 2
endif
$(eval $(call add_define,CP_NUM))

AP_NUM			:= 2
$(eval $(call add_define,AP_NUM))

DOIMAGEPATH		?=	tools/doimage
DOIMAGETOOL		?=	${DOIMAGEPATH}/doimage

ROM_BIN_EXT	?= $(BUILD_PLAT)/ble.bin
DOIMAGE_FLAGS	+= -b $(ROM_BIN_EXT) $(NAND_DOIMAGE_FLAGS) $(DOIMAGE_SEC_FLAGS)


# This define specifies DDR type for BLE
$(eval $(call add_define,CONFIG_DDR4))

MARVELL_GICV3_SOURCES   :=      drivers/arm/gic/common/gic_common.c     \
				drivers/arm/gic/v3/gicv3_main.c         \
				drivers/arm/gic/v3/gicv3_helpers.c      \
				plat/common/plat_gicv3.c		\
				$(PLAT_COMMON_BASE)/plat_marvell_gicv3.c \
				plat/marvell/common/marvell_gicv3.c	\
				drivers/arm/gic/v3/gic600.c

ATF_INCLUDES		:=	-Iinclude/common/tbbr

PLAT_INCLUDES		:=	-I$(PLAT_FAMILY_BASE)/$(PLAT)		\
				-I$(PLAT_COMMON_BASE)/include		\
				-I$(PLAT_INCLUDE_BASE)/common		\
				-Iinclude/drivers/marvell		\
				-Iinclude/drivers/marvell/mochi		\
				$(ATF_INCLUDES)

PLAT_BL_COMMON_SOURCES	:=	$(PLAT_COMMON_BASE)/aarch64/a8kp_common.c \
				drivers/console/aarch64/console.S	 \
				drivers/ti/uart/aarch64/16550_console.S

BLE_PORTING_SOURCES	:=	$(PLAT_FAMILY_BASE)/$(PLAT)/board/dram_port.c \
			$(PLAT_FAMILY_BASE)/$(PLAT)/board/marvell_plat_config.c

BLE_SOURCES		:=	$(PLAT_COMMON_BASE)/plat_ble_setup.c 	\
				$(PLAT_COMMON_BASE)/plat_dram.c		\
				$(PLAT_COMMON_BASE)/ap810_init_clocks.c	\
				$(MARVELL_DRV_BASE)/i2c/a8k_i2c.c	\
				$(PLAT_COMMON_BASE)/plat_pm.c		\
				$(MARVELL_DRV_BASE)/mochi/ap810_setup.c	\
				$(MARVELL_DRV_BASE)/ccu.c		\
				$(MARVELL_DRV_BASE)/gwin.c		\
				$(MARVELL_DRV_BASE)/jtag.c		\
				$(MARVELL_DRV_BASE)/eawg.c		\
				$(MARVELL_DRV_BASE)/ap810_aro.c		\
				$(BLE_PORTING_SOURCES)

ifeq (${PCI_EP_SUPPORT}, 1)
BLE_SOURCES		+=	$(MARVELL_COMMON_BASE)/pci_ep_setup.c	 \
				$(MARVELL_DRV_BASE)/dw-pcie-ep.c	 \
				$(MARVELL_DRV_BASE)/pcie-comphy-cp110.c
endif

MARVELL_BL1_DRV		:= 	$(MARVELL_DRV_BASE)/ccu.c		\
				$(MARVELL_DRV_BASE)/gwin.c		\
				$(MARVELL_DRV_BASE)/iob.c		\
				$(MARVELL_DRV_BASE)/io_win.c		\
				$(MARVELL_DRV_BASE)/mci.c

MARVELL_BL31_DRV	:=	$(MARVELL_DRV_BASE)/amb_adec.c		\
				$(MARVELL_DRV_BASE)/cache_llc.c 	\
				$(MARVELL_DRV_BASE)/icu.c		\
				$(MARVELL_DRV_BASE)/iob.c

ifneq (${PALLADIUM},1)
MARVELL_BL31_DRV	+=	$(MARVELL_DRV_BASE)/comphy/phy-comphy-cp110.c
endif

MARVELL_MOCHI_DRV	:=	$(MARVELL_DRV_BASE)/mochi/ap810_setup.c \
				$(MARVELL_DRV_BASE)/mochi/cp110_setup.c

PORTING_SOURCES		:=	$(PLAT_FAMILY_BASE)/$(PLAT)/board/marvell_plat_config.c

BL1_SOURCES		+=	lib/cpus/aarch64/cortex_a72.S			\
				$(PLAT_COMMON_BASE)/aarch64/plat_arch_config.c	\
				$(PLAT_COMMON_BASE)/aarch64/plat_helpers.S	\
				$(PLAT_COMMON_BASE)/plat_bl1_setup.c		\
				$(MARVELL_BL1_DRV)				\
				$(MARVELL_MOCHI_DRV)				\
				$(PORTING_SOURCES)

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a72.S			\
				$(PLAT_COMMON_BASE)/aarch64/plat_arch_config.c	\
				$(PLAT_COMMON_BASE)/aarch64/plat_helpers.S	\
				$(PLAT_COMMON_BASE)/plat_bl31_setup.c		\
				$(PLAT_COMMON_BASE)/plat_pm.c			\
				$(MARVELL_COMMON_BASE)/mrvl_sip_svc.c		\
				$(MARVELL_COMMON_BASE)/marvell_ddr_info.c	\
				$(MARVELL_BL31_DRV)				\
				$(MARVELL_GICV3_SOURCES)			\
				$(MARVELL_MOCHI_DRV)				\
				$(PORTING_SOURCES)

# Disable the PSCI platform compatibility layer (allows porting
# from Old Platform APIs to the new APIs).
# It is not needed since Marvell platform already used the new platform APIs.
ENABLE_PLAT_COMPAT	:= 	0

# MSS (SCP) build
include $(PLAT_COMMON_BASE)/mss/mss_a8kp.mk

# BLE (ROM context execution code, AKA binary extension)
BLE_PATH	?=  ble

include ${BLE_PATH}/ble.mk
$(eval $(call MAKE_BL,e))

mrvl_flash: ${BUILD_PLAT}/${FIP_NAME} ${DOIMAGETOOL} ${BUILD_PLAT}/ble.bin
	$(shell truncate -s %128K ${BUILD_PLAT}/bl1.bin)
	$(shell cat ${BUILD_PLAT}/bl1.bin ${BUILD_PLAT}/${FIP_NAME} > ${BUILD_PLAT}/${BOOT_IMAGE})
	${DOIMAGETOOL} ${DOIMAGE_FLAGS} ${BUILD_PLAT}/${BOOT_IMAGE} ${BUILD_PLAT}/${FLASH_IMAGE}
