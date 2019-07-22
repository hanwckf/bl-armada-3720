# *
# * Copyright (C) 2017 Marvell International Ltd.
# *
# * SPDX-License-Identifier:     BSD-3-Clause
# * https://spdx.org/licenses
# *
MARVELLSPD_DIR		:=	services/spd/marvell_spd
SPD_INCLUDES		:=	-Iinclude/bl32/payloads

SPD_SOURCES		:=	services/spd/marvell_spd/marvell_spd_common.c	\
				services/spd/marvell_spd/marvell_spd_main.c	\
				services/spd/marvell_spd/marvell_spd_helpers.S

NEED_BL32		:=	yes
