/*
 * Copyright (C) 2016 - 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */
#ifndef __MVEBU_DEF_H__
#define __MVEBU_DEF_H__

#include <a8k_plat_def.h>

#define CP_COUNT		2	/* A80x0 has both CP0 & CP1 */
#define OCP_COMPHY_TUNE

/* Force disable of LLC, regardless of LLC_ENABLE compilation flag
 * this is due to an issue observed while running iperf with
 * smartNIC End Poind driver/application, while LLC is enabled.
 * as a temporary workaround, until issue is solved, we disable LLC
 */
#ifdef LLC_ENABLE
#undef LLC_ENABLE
#define LLC_ENABLE		0
#endif

#endif /* __MVEBU_DEF_H__ */
