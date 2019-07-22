/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */
#include <ap810_aro.h>
#include <ap810_setup.h>
#include <delay_timer.h>
#include <mmio.h>
#include <mvebu_def.h>

#define ARO_REG_BASE_ADDR(ap)			(MVEBU_DFX_SR_BASE(ap) + (0xD00))
#define ARO_CLUSTER_REG0_ADDR(cluster, ap)	(ARO_REG_BASE_ADDR(ap) + 0x48 + ((cluster) * 0x8))
#define ARO_CLUSTER_REG1_ADDR(cluster, ap)	(ARO_CLUSTER_REG0_ADDR(cluster, ap) + 0x4)

#define CPU_FREQ_3000		0x000004B0
#define CPU_FREQ_2700		0x00000438
#define	CPU_FREQ_2500		0x000003E8
#define	CPU_FREQ_2400		0x000003C0
#define	CPU_FREQ_2200		0x00000370
#define	CPU_FREQ_2000		0x00000320
#define	CPU_FREQ_1800		0x000002D0
#define	CPU_FREQ_1600		0x00000280
#define	CPU_FREQ_1200		0x000001E0
#define CPU_FREQ_1000		0x00000190

#define ARO_DELAY		0x9

/* In case of target frequency 2700 and 3000,
 * ARO flow is done in 2 steps, in step A a smaller
 * frequency (than 2700 and 3000) is configured and afterwards in Step B
 * the actual target frequency is configured (2700 OR 3000).
 */
#define ARO_FREQ_2700_STEP_A	0x270F
#define ARO_FREQ_2700_STEP_B	0x107AC0
#define ARO_FREQ_3000_STEP_A	0x270F
#define ARO_FREQ_3000_STEP_B	0x124F80

static int cpu_freq_table[10] = {
	CPU_FREQ_1000,
	CPU_FREQ_1200,
	CPU_FREQ_1600,
	CPU_FREQ_1800,
	CPU_FREQ_2000,
	CPU_FREQ_2200,
	CPU_FREQ_2400,
	CPU_FREQ_2500,
	CPU_FREQ_2700,
	CPU_FREQ_3000,
};

/* Init all CPU's for each AP required. */
void ap810_aro_init(int target_freq, int ap_num)
{
	int cluster_num;
	int freq, cluster0_shift;

	freq = cpu_freq_table[target_freq];

	/* 2700 and 3000 touch the device speed limit,
	 * therefore it requires a special flow when building
	 * the clock frequency from a certain stage, else the device may fail
	 */
	for (cluster_num = 0 ; cluster_num < PLAT_MARVELL_CLUSTER_PER_NB ; cluster_num++) {

		/* Due to shift by 2 in cluster0 registers (A0 bug) .
		 * TODO: update with EERATA number.
		 */
		cluster0_shift = 1;
		if ((cluster_num == 0) && (ap810_rev_id_get(ap_num) == MVEBU_AP810_REV_ID_A0))
			cluster0_shift = 4;

		if (freq == CPU_FREQ_2700) {
			mmio_write_32(ARO_CLUSTER_REG1_ADDR(cluster_num, ap_num), ARO_DELAY);
			mmio_write_32(ARO_CLUSTER_REG0_ADDR(cluster_num, ap_num), CPU_FREQ_2400 * cluster0_shift);
			/* delay for 10 ms */
			udelay(10000);
			mmio_write_32(ARO_CLUSTER_REG1_ADDR(cluster_num, ap_num), ARO_FREQ_2700_STEP_A);
			mmio_write_32(ARO_CLUSTER_REG0_ADDR(cluster_num, ap_num),
				      ARO_FREQ_2700_STEP_B * cluster0_shift);

		} else if (freq == CPU_FREQ_3000) {
			mmio_write_32(ARO_CLUSTER_REG1_ADDR(cluster_num, ap_num), ARO_DELAY);
			mmio_write_32(ARO_CLUSTER_REG0_ADDR(cluster_num, ap_num), CPU_FREQ_2700  * cluster0_shift);
			/* delay for 10 ms */
			udelay(10000);
			mmio_write_32(ARO_CLUSTER_REG1_ADDR(cluster_num, ap_num), ARO_FREQ_3000_STEP_A);
			mmio_write_32(ARO_CLUSTER_REG0_ADDR(cluster_num, ap_num),
				      ARO_FREQ_3000_STEP_B * cluster0_shift);

		} else {
			mmio_write_32(ARO_CLUSTER_REG1_ADDR(cluster_num, ap_num), ARO_DELAY);
			mmio_write_32(ARO_CLUSTER_REG0_ADDR(cluster_num, ap_num), freq * cluster0_shift);
		}
	}
}
