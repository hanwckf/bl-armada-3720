/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */
#ifndef __MVEBU_DEF_H__
#define __MVEBU_DEF_H__

#include <a8kp_plat_def.h>

#if (CP_NUM > 8) || (CP_NUM < 0)
#error "Supported CP_NUM values are 0 to 8."
#else
#define CP110_DIE_NUM			CP_NUM
#endif
#define MPP_MCI_RELEASE_FROM_RESET	16

#endif /* __MVEBU_DEF_H__ */
