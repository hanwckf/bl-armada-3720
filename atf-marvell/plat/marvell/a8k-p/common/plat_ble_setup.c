/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <ap_setup.h>
#include <ap810_setup.h>
#include <ap810_init_clocks.h>
#include <a8kp_plat_def.h>
#include <debug.h>
#include <mmio.h>
#include <mvebu_def.h>
#include <plat_marvell.h>
#include <plat_dram.h>

/* Read Frequency Value from MPPS 15-17 and save
 * to scratch-pad Register as a temporary solution
 * in AP810 A0 revision to cover the bug in sampled-at-reset
 * register.
*/
static void ble_read_cpu_freq(void)
{
	unsigned int mpp_address, val;

	/* Relevant for A0 only */
	if (ap810_rev_id_get(MVEBU_AP0) != MVEBU_AP810_REV_ID_A0)
		return;

	/* TODO: add errata for this WA, we can't read from sample at reset
	 * register.
	 */
	mpp_address = MVEBU_AP_GPIO_DATA_IN(0);
	val = mmio_read_32(mpp_address);
	val = (val >> 15) & 0x7;
	INFO("sar option read from MPPs = 0x%x\n", val);
	mmio_write_32(SCRATCH_PAD_ADDR(0, 1), val);
}

int ble_plat_setup(int *skip)
{
	int ret = 0;

#if !PALLADIUM
	/* SW WA for AP link bring-up over JTAG connection */
	if ((ap_get_count() != 1) &&
	    (ap810_rev_id_get(MVEBU_AP0) == MVEBU_AP810_REV_ID_A0))
		jtag_init_ihb_dual_ap();
#endif

	ble_read_cpu_freq();

	/* Initialize the enumeration algorithm in BLE stage to
	 * enable access to another APs.
	 */
	ap810_enumeration_algo();

	/* init clocks for single AP */
	ap810_clocks_init(ap_get_count());

	/* TODO: need to check if need early cpu powerdown */

	/* TODO: check if recovery feature is needed */

	/* TODO: check if SVC is needed */

	/* Trigger DRAM driver initialization */
	ret = plat_dram_init();

	return ret;
}
