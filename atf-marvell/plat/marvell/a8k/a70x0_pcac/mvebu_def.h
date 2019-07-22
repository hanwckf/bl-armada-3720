/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */
#ifndef __MVEBU_DEF_H__
#define __MVEBU_DEF_H__

#include <a8k_plat_def.h>

#define CP_COUNT		1	/* A70x0 has single CP0 */

/* Force disable of LLC, regardless of LLC_ENABLE compilation flag
 * this is due to an issue observed while running iperf with
 * smartNIC End Poind driver/application, while LLC is enabled.
 * as a temporary workaround, until issue is solved, we disable LLC
 */
#ifdef LLC_ENABLE
#undef LLC_ENABLE
#define LLC_ENABLE		0
#endif

/***********************************************************************
 * Required platform porting definitions common to all
 * Mangement Compute SubSystems (MSS)
 ***********************************************************************
 */
/*
 * Load address of SCP_BL2
 * SCP_BL2 is loaded to the same place as BL31.
 * Once SCP_BL2 is transferred to the SCP,
 * it is discarded and BL31 is loaded over the top.
 */
#ifdef SCP_IMAGE
#define SCP_BL2_BASE                    BL31_BASE
#endif


#endif /* __MVEBU_DEF_H__ */
