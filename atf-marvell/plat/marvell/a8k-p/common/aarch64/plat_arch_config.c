/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <platform.h>
#include <arch_helpers.h>
#include <mmio.h>
#include <debug.h>
#include <cache_llc.h>
#include <plat_marvell.h>

static void plat_enable_affinity(void)
{
	int cluster_id, ap_id;
	uint64_t mpidr = read_mpidr_el1();

	ap_id = MPIDR_AP_ID_GET(mpidr);
	/* set CPU Affinity */
	cluster_id = plat_marvell_calc_core_pos_die(mpidr) / PLAT_MARVELL_CLUSTER_CORE_COUNT;
	mmio_write_32(CCU_HTC_ASET(ap_id), (1 << cluster_id));

	/* set barier */
	__asm__ volatile("isb");
}

void marvell_psci_arch_init(int ap_index)
{
#if LLC_ENABLE
	/* check if LLC is in exclusive mode
	 * as L2 is configured to UniqueClean eviction
	 * (in a8k reset handler)
	 */
	if (llc_is_exclusive(ap_index) == 0)
		ERROR("LLC should be configured to exclusice mode\n");
#endif

	/* Enable Affinity */
	plat_enable_affinity();
}
