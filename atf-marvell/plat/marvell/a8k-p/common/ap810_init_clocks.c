/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <ap810_aro.h>
#include <ap810_setup.h>
#include <ap810_init_clocks.h>
#include <eawg.h>
#include <errno.h>
#include <debug.h>
#include <mmio.h>
#include <mvebu_def.h>
#include <stdio.h>

/* PLL's registers with local base address since each AP has its own EAWG*/
#define PLL_DSS_FRACT_ADDRESS		(MVEBU_DFX_SSCG_LOCAL_AP + 0x1B0)
#define PLL_DSS_FRACT_EN_ADDRESS	(MVEBU_DFX_SSCG_LOCAL_AP + 0x1B4)
#define PLL_CLUSTER_1_0_ADDRES		(MVEBU_DFX_SAR_LOCAL_AP + 0x2E0)
#define PLL_CLUSTER_2_3_ADDRES		(MVEBU_DFX_SAR_LOCAL_AP + 0x2E8)
#define PLL_RING_ADDRESS		(MVEBU_DFX_SAR_LOCAL_AP + 0x2F0)
#define PLL_IO_ADDRESS			(MVEBU_DFX_SAR_LOCAL_AP + 0x2F8)
#define PLL_PIDI_ADDRESS		(MVEBU_DFX_SAR_LOCAL_AP + 0x310)
#define PLL_DSS_ADDRESS			(MVEBU_DFX_SAR_LOCAL_AP + 0x300)

/* frequencies values */
#define PLL_FREQ_3000		0x6D477001 /* 3000 */
#define PLL_FREQ_2700		0x6B06B001 /* 2700 */
#define PLL_FREQ_2500		0x6B063001 /* 2500 */
#define PLL_FREQ_2400		0x6AE5F001 /* 2400 */
#define PLL_FREQ_2200		0x6AC57001 /* 2200 */
#define PLL_FREQ_2000		0x6FC9F002 /* 2000 */
#define PLL_FREQ_1800		0x6D88F002 /* 1800 */
#define PLL_FREQ_1600		0x6D47F002 /* 1600 */
#define PLL_FREQ_1466		0x6D274002 /* 1466.5 */
#define PLL_FREQ_1400		0x6D26F002 /* 1400 */
#define PLL_FREQ_1333		0x6B069002 /* 1333.5 */
#define PLL_FREQ_1300		0x6B067002 /* 1300 */
#define PLL_FREQ_1200		0x6AE5F002 /* 1200 */
#define PLL_FREQ_1100		0x6AC57002 /* 1100 */
#define PLL_FREQ_1000		0x6AC4F002 /* 1000 */
#define PLL_FREQ_800		0x6883F002 /* 800 */

/* fractional frequencies values and enable or disable values */
#define PLL_FRAC_FREQ_3000		0x0 /* 3000 */
#define PLL_FRAC_FREQ_2700		0x0 /* 2700 */
#define PLL_FRAC_FREQ_2500		0x0 /* 2500 */
#define PLL_FRAC_FREQ_2400		0x0 /* 2400 */
#define PLL_FRAC_FREQ_2200		0x0 /* 2200 */
#define PLL_FRAC_FREQ_2000		0x0 /* 2000 */
#define PLL_FRAC_FREQ_1800		0x0 /* 1800 */
#define PLL_FRAC_FREQ_1600		0x0 /* 1600 */
#define PLL_FRAC_FREQ_1466		0x0CE7356F /* 1466.5 */
#define PLL_FRAC_FREQ_1400		0x0 /* 1400 */
#define PLL_FRAC_FREQ_1333		0xAC0 /* 1333.5 */
#define PLL_FRAC_FREQ_1300		0x0 /* 1300 */
#define PLL_FRAC_FREQ_1200		0x0 /* 1200 */
#define PLL_FRAC_FREQ_1100		0x0 /* 1100 */
#define PLL_FRAC_FREQ_1000		0x0 /* 1000 */
#define PLL_FRAC_FREQ_800		0x0 /* 800 */

#define PLL_FRAC_FREQ_DIS		0x0
#define PLL_FRAC_FREQ_EN		0x2

/* PLL device control registers */
#define PLL_CONTROL_REG(ap, n)	(MVEBU_DFX_SR_BASE(ap) + ((n > 0) ? (0xD34 + ((n + 1) * 0x4)) : 0xD34))
#define PLL_CTRL_REG_NUM	4

/* EAWG functionality */
#define SCRATCH_PAD_LOCAL_REG	(MVEBU_REGS_BASE_LOCAL_AP + 0x6F43E0)
#define CPU_WAKEUP_COMMAND(ap)	(MVEBU_CCU_LOCL_CNTL_BASE(ap) + 0x80)

/* fetching target frequencies */
#define SCRATCH_PAD_FREQ_REG	SCRATCH_PAD_ADDR(0, 1)
#define EFUSE_FREQ_REG		(MVEBU_AP_MPP_REGS(0, 0) + 0x410)
#define EFUSE_FREQ_OFFSET	24
#define EFUSE_FREQ_MASK		(0x1 << EFUSE_FREQ_OFFSET)

#define SAR_SUPPORTED_TABLES	2
#define SAR_SUPPORTED_OPTIONS	8

#define ARO_MODE		0x0
#define PLL_MODE		0x1

enum pll_type {
	RING,
	IO,
	PIDI,
	DSS,
	DSS_FRAC_EN,
	DSS_FRAC,
	PLL_CLUSTER_0_FREQ, /* PLL for cluster0 and cluster1 */
	PLL_CLUSTER_2_FREQ, /* PLL for cluster2 and cluster3 */
	PLL_LAST,
	CPU_FREQ,
	DDR_FREQ,
};

unsigned int pll_freq_tables[SAR_SUPPORTED_TABLES]
			    [SAR_SUPPORTED_OPTIONS]
			    [PLL_LAST + 2] = {
	{
		/* RING, IO, PIDI, DSS_FRAC_EN, DSS, DSS_FRAC, PLL_CLUSTER_0_FREQ, PLL_CLUSTER_2_FREQ, CPU_FREQ */
		{PLL_FREQ_1200, PLL_FREQ_800, PLL_FREQ_1000, PLL_FREQ_800, PLL_FRAC_FREQ_DIS,
		 PLL_FRAC_FREQ_800, PLL_FREQ_1600, PLL_FREQ_1600, TARGET_FREQ_1600, DDR_FREQ_800},
		{PLL_FREQ_1200, PLL_FREQ_800, PLL_FREQ_1000, PLL_FREQ_1200, PLL_FRAC_FREQ_DIS,
		 PLL_FRAC_FREQ_1200, PLL_FREQ_2000, PLL_FREQ_2000, TARGET_FREQ_2000, DDR_FREQ_1200},
		{PLL_FREQ_1400, PLL_FREQ_1000, PLL_FREQ_1000, PLL_FREQ_1333, PLL_FRAC_FREQ_EN,
		 PLL_FRAC_FREQ_1333, PLL_FREQ_2000, PLL_FREQ_2000, TARGET_FREQ_2000, DDR_FREQ_1333},
		{PLL_FREQ_1400, PLL_FREQ_1000, PLL_FREQ_1000, PLL_FREQ_1200, PLL_FRAC_FREQ_DIS,
		 PLL_FRAC_FREQ_1200, PLL_FREQ_2200, PLL_FREQ_2200, TARGET_FREQ_2200, DDR_FREQ_1200},
		{PLL_FREQ_1400, PLL_FREQ_1000, PLL_FREQ_1000, PLL_FREQ_1333, PLL_FRAC_FREQ_EN,
		 PLL_FRAC_FREQ_1333, PLL_FREQ_2200, PLL_FREQ_2200, TARGET_FREQ_2200, DDR_FREQ_1333},
		{PLL_FREQ_1400, PLL_FREQ_1000, PLL_FREQ_1000, PLL_FREQ_1333, PLL_FRAC_FREQ_EN,
		 PLL_FRAC_FREQ_1333, PLL_FREQ_2500, PLL_FREQ_2500, TARGET_FREQ_2500, DDR_FREQ_1333},
		{PLL_FREQ_1400, PLL_FREQ_1000, PLL_FREQ_1000, PLL_FREQ_1466, PLL_FRAC_FREQ_EN,
		 PLL_FRAC_FREQ_1466, PLL_FREQ_2500, PLL_FREQ_2500, TARGET_FREQ_2500, DDR_FREQ_1466},
		{PLL_FREQ_1400, PLL_FREQ_1000, PLL_FREQ_1000, PLL_FREQ_1600, PLL_FRAC_FREQ_EN,
		 PLL_FRAC_FREQ_1600, PLL_FREQ_2700, PLL_FREQ_2700, TARGET_FREQ_2700, DDR_FREQ_1600},
	},
	{
		/* RING, IO, PIDI, DSS_FRAC_EN, DSS, DSS_FRAC, PLL_CLUSTER_0_FREQ, PLL_CLUSTER_2_FREQ, CPU_FREQ */
		{PLL_FREQ_800, PLL_FREQ_800, PLL_FREQ_1000, PLL_FREQ_800, PLL_FRAC_FREQ_DIS,
		 PLL_FRAC_FREQ_800, PLL_FREQ_1200, PLL_FREQ_1200, TARGET_FREQ_1200, DDR_FREQ_800},
		{PLL_FREQ_800, PLL_FREQ_800, PLL_FREQ_1000, PLL_FREQ_1200, PLL_FRAC_FREQ_DIS,
		 PLL_FRAC_FREQ_1200, PLL_FREQ_1600, PLL_FREQ_1600, TARGET_FREQ_1600, DDR_FREQ_1200},
		{PLL_FREQ_1200, PLL_FREQ_800, PLL_FREQ_1000, PLL_FREQ_1333, PLL_FRAC_FREQ_EN,
		 PLL_FRAC_FREQ_1333, PLL_FREQ_1600, PLL_FREQ_1600, TARGET_FREQ_1600, DDR_FREQ_1333},
		{PLL_FREQ_1200, PLL_FREQ_800, PLL_FREQ_1000, PLL_FREQ_1200, PLL_FRAC_FREQ_DIS,
		 PLL_FRAC_FREQ_1200, PLL_FREQ_1800, PLL_FREQ_1800, TARGET_FREQ_1800, DDR_FREQ_1200},
		{PLL_FREQ_1300, PLL_FREQ_800, PLL_FREQ_1000, PLL_FREQ_1333, PLL_FRAC_FREQ_EN,
		 PLL_FRAC_FREQ_1333, PLL_FREQ_1800, PLL_FREQ_1800, TARGET_FREQ_1800, DDR_FREQ_1333},
		{PLL_FREQ_1200, PLL_FREQ_800, PLL_FREQ_1000, PLL_FREQ_1200, PLL_FRAC_FREQ_DIS,
		 PLL_FRAC_FREQ_1200, PLL_FREQ_2000, PLL_FREQ_2000, TARGET_FREQ_2000, DDR_FREQ_1200},
		{PLL_FREQ_1300, PLL_FREQ_800, PLL_FREQ_1000, PLL_FREQ_1333, PLL_FRAC_FREQ_EN,
		 PLL_FRAC_FREQ_1333, PLL_FREQ_2000, PLL_FREQ_2000, TARGET_FREQ_2000, DDR_FREQ_1333},
		{PLL_FREQ_1300, PLL_FREQ_800, PLL_FREQ_1000, PLL_FREQ_1333, PLL_FRAC_FREQ_EN,
		 PLL_FRAC_FREQ_1333, PLL_FREQ_2000, PLL_FREQ_2000, TARGET_FREQ_2000, DDR_FREQ_1333},
	},
};

unsigned int pll_base_address[PLL_LAST] = {
	PLL_RING_ADDRESS, /* RING */
	PLL_IO_ADDRESS, /* IO */
	PLL_PIDI_ADDRESS, /* PIDI */
	PLL_DSS_ADDRESS, /* DSS */
	PLL_DSS_FRACT_EN_ADDRESS, /* DSS FRACTIONAL EN */
	PLL_DSS_FRACT_ADDRESS, /* DSS FRACTIONAL */
	PLL_CLUSTER_1_0_ADDRES, /* PLL for cluster0 and cluster1 */
	PLL_CLUSTER_2_3_ADDRES, /* PLL for cluster2 and cluster3 */
};

#if !ARO_ENABLE
/* This function switch clock mode,
 * clock driven by ARO.
 * clock driven by PLL.
 */
static void clocks_switch_aro_pll(unsigned int clock_mode, int ap_count)
{
	unsigned int reg;
	int ap, pll_ctrl_reg, pll_ctrl_reg_offset, aro_ctrl_reg_offset;

	for (ap = 0 ; ap < ap_count; ap++) {
		for (pll_ctrl_reg = 0 ; pll_ctrl_reg < PLL_CTRL_REG_NUM ; pll_ctrl_reg++) {

			/* bits in pll control register for enable/disable of
			 * pll and aro.
			 */
			pll_ctrl_reg_offset = 7;
			aro_ctrl_reg_offset = 2;

			/* in cluster0 the bits are different from cluster 1-3. */
			if (!pll_ctrl_reg) {
				pll_ctrl_reg_offset = 31;
				aro_ctrl_reg_offset = 26;
			}

			/* Enable/Disable PLL in control_reg */
			reg = mmio_read_32(PLL_CONTROL_REG(ap, pll_ctrl_reg));
			reg &= ~(0x1 << pll_ctrl_reg_offset);
			reg |= (clock_mode << pll_ctrl_reg_offset);
			mmio_write_32(PLL_CONTROL_REG(ap, pll_ctrl_reg), reg);

			/* Enable/Disable ARO*/
			reg = mmio_read_32(PLL_CONTROL_REG(ap, pll_ctrl_reg));
			reg &= ~(0x1 << aro_ctrl_reg_offset);
			reg |= (clock_mode << aro_ctrl_reg_offset);
			mmio_write_32(PLL_CONTROL_REG(ap, pll_ctrl_reg), reg);
		}
	}
}
#endif

/* read efuse value which device if it's low/high frequency
 * fetch frequency values from appropriate table
 */
void clocks_fetch_options(uint32_t *freq_mode, uint32_t *clk_index)
{
	/* fetch eFuse value and device whether it's H/L */
	*freq_mode = mmio_read_32(EFUSE_FREQ_REG);
	*freq_mode &= EFUSE_FREQ_MASK;
	*freq_mode = (*freq_mode) >> EFUSE_FREQ_OFFSET;

	/* in A0 sampled-at-reset register is not functional */
	if (ap810_rev_id_get(MVEBU_AP0) == MVEBU_AP810_REV_ID_A0)
		*clk_index = (mmio_read_32(SCRATCH_PAD_ADDR(0, 1)) & 0x7);
	else
		ERROR("sample at reset register is missing - failed to configure clocks\n");
}

/* prepares the transactions to be send to each AP's EAWG FIFO */
static int clocks_prepare_transactions(uint32_t *plls_clocks_vals,
				       struct eawg_transaction *trans_array,
				       int clock_id_first, int clock_id_last,
				       int *transactions_num, int *sub_transactions_num)
{
	int pll, i;

	/*build transactions array to be written to EAWGs' FIFO */
	for (pll = clock_id_first, i = 0 ; pll <= clock_id_last ; pll++) {

		if (pll_base_address[pll] == -1) {
			printf("PLL number %d value is not initialized", pll);
			return -1;
		}

		/* in case the DSS fractional frequency is disabled skip
		 * the current and the next transactions which are:
		 * 1. fractional frequency enable
		 * 2. fraction frequency value
		 * in case the fractional frequency is enabled
		 * add a single "sub transaction" per register configuration
		 */
		if ((pll == DSS_FRAC_EN) && (plls_clocks_vals[pll] == PLL_FRAC_FREQ_DIS)) {
			pll++;
			continue;
		} else {
			if ((pll == DSS_FRAC_EN) || (pll == DSS_FRAC)) {
				/* setting the new desired frequency */
				trans_array[i].address = pll_base_address[pll];
				trans_array[i].data = plls_clocks_vals[pll];
				trans_array[i].delay = 0x1;

				i++;
				(*sub_transactions_num)++;
				continue;
			}
		}

		/* For each PLL type there's 4 transactions to be written */
		/* setting use RF bit to 1 */
		trans_array[i].address = pll_base_address[pll] + 0x4;
		trans_array[i].data = 0x200;
		trans_array[i].delay = 0x1;

		i++;

		/* ring bypass while still in RF conf */
		trans_array[i].address = pll_base_address[pll] + 0x4;
		trans_array[i].data = 0x201;
		trans_array[i].delay = 0x1;

		i++;

		/* setting the new desired frequency */
		trans_array[i].address = pll_base_address[pll];
		trans_array[i].data = plls_clocks_vals[pll];
		trans_array[i].delay = 0x1;

		i++;

		/* turning off ring bypass while leaving RF conf on */
		trans_array[i].address = pll_base_address[pll] + 0x4;
		trans_array[i].data = 0x200;
		trans_array[i].delay = 0x0;

		i++;
		(*transactions_num)++;
	}

	return 0;
}

/* 1.constructs array of transactions built on the configuration
 *   chosen in SAR.
 * 2.for each AP, load transactions using CPU 0 to APs' EAWG FIFO.
 * 3.trigger each AP's EAWG and finally CPU 0 in AP0.
 * 4.when CPU0 wakes from EAWG, read SCRATCH_PAD registers to check
 *   whether all APs' EAWG finished.
 */
int ap810_clocks_init(int ap_count)
{
	/* for each PLL there's 4 transactions and another transaction for
	 * writing to each AP's scratch-pad register to notify EAWG is done.
	 *
	 * extra transaction is needed
	 * for CPU0 in AP0:
	 * 1.loading a wake up command to CPU0 (1 transaction).
	 *
	 * plls_clocks_vals contains info about each PLL clock value
	 * and one extra value on cpu freq for ARO use.
	 */
	struct eawg_transaction trans_array[(PLL_LAST * TRANS_PER_PLL)];
	struct eawg_transaction eawg_done_indication;
	struct eawg_transaction eawg_wakeup_indication;
	uint32_t plls_clocks_vals[PLL_LAST];
	uint32_t freq_mode, clk_config;
	int ddr_clock_option;
	int clock_id_end;
	int transactions_num = 0;
	int sub_transactions_num = 0;
	int ap;

	/* check if the total number of transactions doesn't exceeds EAWG's
	 * FIFO capacity.
	 */
	if ((PLL_LAST * TRANS_PER_PLL) > (MAX_TRANSACTIONS - 1)) {
		printf("transactions number exceeded fifo size\n");
		return -1;
	}

	/* fetch frequency option*/
	clocks_fetch_options(&freq_mode, &clk_config);
	if (clk_config < 0 || clk_config > (SAR_SUPPORTED_OPTIONS - 1)) {
		printf("clk option 0x%d is not supported\n", clk_config);
		return -1;
	}

	plls_clocks_vals[RING] = pll_freq_tables[freq_mode][clk_config][RING];
	plls_clocks_vals[IO] = pll_freq_tables[freq_mode][clk_config][IO];
	plls_clocks_vals[PIDI] = pll_freq_tables[freq_mode][clk_config][PIDI];
	plls_clocks_vals[DSS] = pll_freq_tables[freq_mode][clk_config][DSS];
	plls_clocks_vals[DSS_FRAC_EN] = pll_freq_tables[freq_mode][clk_config][DSS_FRAC_EN];
	plls_clocks_vals[DSS_FRAC] = pll_freq_tables[freq_mode][clk_config][DSS_FRAC];
	plls_clocks_vals[PLL_CLUSTER_0_FREQ] = pll_freq_tables[freq_mode][clk_config][PLL_CLUSTER_0_FREQ];
	plls_clocks_vals[PLL_CLUSTER_2_FREQ] = pll_freq_tables[freq_mode][clk_config][PLL_CLUSTER_2_FREQ];

	/* update ddr clock option in dram topology */
	ddr_clock_option = pll_freq_tables[freq_mode][clk_config][DDR_FREQ - 1];
	plat_dram_freq_update(ddr_clock_option);

#if ARO_ENABLE
	clock_id_end = DSS_FRAC;
#else
	clock_id_end = PLL_CLUSTER_2_FREQ;
	clocks_switch_aro_pll(PLL_MODE, ap_count);
#endif

	if (clocks_prepare_transactions(plls_clocks_vals, trans_array, RING, clock_id_end,
					&transactions_num, &sub_transactions_num))
		return -1;

	/* one extra transaction to write to a scratch-pad register in each AP */
	eawg_done_indication.address = SCRATCH_PAD_LOCAL_REG;
	eawg_done_indication.data = 0x1;
	eawg_done_indication.delay = 0x0;

	/* Loading a wake up command to primary CPU */
	eawg_wakeup_indication.address = CPU_WAKEUP_COMMAND(0);
	eawg_wakeup_indication.data = 0x1;
	eawg_wakeup_indication.delay = 0x3;

	/* write transactions to each APs' EAWG FIFO */
	for (ap = 0 ; ap < ap_count ; ap++) {
		if (eawg_load_transactions(trans_array,
					  (transactions_num * TRANS_PER_PLL +
					   sub_transactions_num), ap)) {
			printf("couldn't load all transactions to AP%d EAWG FIFO\n", ap);
			return -1;
		}
		/* Load done indication for each AP's EAWG */
		if (eawg_load_transactions(&eawg_done_indication, 1, ap)) {
			printf("couldn't load done-indication transaction to AP%d EAWG FIFO\n", ap);
			return -1;
		}
		/* Load wake up transaction for CPU0 */
		if (ap == 0) {
			if (eawg_load_transactions(&eawg_wakeup_indication, 1, ap)) {
				printf("couldn't load wake-up transaction to AP%d EAWG FIFO\n", ap);
				return -1;
			}
		}
	}

	/* trigger each AP's EAWG and finally CPU 0 in AP0,
	 * after this step all CPUs are in WFE status.
	 */
	for (ap = ap_count - 1 ; ap >= 0 ; ap--)
		eawg_start(ap);

	/* when CPU0 wakes from EAWG, read SCRATCH_PAD registers to check
	 * whether all APs' EAWG finished.
	 */
	for (ap = ap_count - 1 ; ap >= 0 ; ap--) {
		if (!eawg_check_is_done(SCRATCH_PAD_ADDR(ap, 0), ap))
			disable_eawg(ap);
	}

#if ARO_ENABLE
	/* configure CPU's frequencies in suppored AP's */
	for (ap = 0 ; ap < ap_count ; ap++)
		ap810_aro_init(pll_freq_tables[freq_mode][clk_config][CPU_FREQ - 1], ap);
#endif
	return 0;
}
