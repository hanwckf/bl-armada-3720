#
# Copyright (C) 2018 Marvell International Ltd.
#
# SPDX-License-Identifier:	BSD-3-Clause
# https://spdx.org/licenses
#

MARVELL_DRV_BASE	:=	drivers/marvell
PLAT_MARVELL		:= 	plat/marvell
A8KP_MSS_SOURCE		:= 	$(PLAT_MARVELL)/a8k-p/common/mss


BL2_SOURCES		+=	$(A8KP_MSS_SOURCE)/mss_bl2_setup.c	\
				$(MARVELL_DRV_BASE)/mochi/ap810_setup.c

PLAT_INCLUDES           +=      -I$(A8KP_MSS_SOURCE)

ifneq (${SCP_BL2},)
# Subsystems require a SCP_BL2 image
$(eval $(call FIP_ADD_IMG,SCP_BL2,--scp-fw))

# This define is used to inidcate the SCP image is present
$(eval $(call add_define,SCP_IMAGE))
endif
