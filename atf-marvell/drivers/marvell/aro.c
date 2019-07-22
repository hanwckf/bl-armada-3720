/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <armada_common.h>
#include <aro.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <mvebu.h>
#include <mvebu_def.h>

#define CPU_ARO_CTRL_BASE		MVEBU_REGS_BASE + (0x6F8D00)
#define SAR_REG_ADDR			MVEBU_REGS_BASE + 0x6f4400
#define RST2_CLOCK_FREQ_MODE_OFFS	0
#define RST2_CLOCK_FREQ_MODE_MASK	0x1f

/* cpu0_pll_ro_cfg0
 * cpu1_pll_ro_cfg0
 */
#define CPU_ARO_CTRL0(CPU)		(CPU_ARO_CTRL_BASE + 0x08 + (CPU * 0xc))

/* Enable ARO Value */
#define USR_REF_STOP_CPU_CNT_OFF	0
#define USR_REF_STOP_CPU_CNT_MASK	(0x1 << USR_REF_STOP_CPU_CNT_OFF)
#define USR_RO_RESET_OFF		5
#define	USR_RO_RESET_MASK		(0x1 << USR_RO_RESET_OFF)

/* cpu0_pll_ro_cfg1
 * cpu1_pll_ro_cfg1
 */
#define CPU_ARO_CTRL1(CPU)		(CPU_ARO_CTRL_BASE + 0x0c + (CPU * 0xc))

/* Set initial ARO freq targets */
#define UPDATED_VAL_OFF			16
#define UPDATED_VAL_MASK		(0x7 << UPDATED_VAL_OFF)
#define CHOOSE_TRGT1_FREQ_OFF		12
#define CHOOSE_TRGT1_FREQ_MASK		(0x8 << CHOOSE_TRGT1_FREQ_OFF)
#define CHOOSE_PLL_OFF			4
#define CHOOSE_PLL_MASK			(0x8 << CHOOSE_PLL_OFF)

/* Stop Forcing Calibration mode */
#define USER_RO_SEL_TRGT_OFF		18
#define USER_RO_SEL_TRGT_MASK		(0x1 << USER_RO_SEL_TRGT_OFF)

#define USER_UPDATE_RO_TRGT_OFF		17
#define USER_UPDATE_RO_TRGT_MASK	(0x1 << USER_UPDATE_RO_TRGT_OFF)

#define CHANGE_PLL_TO_ARO_OFF		16
#define CHANGE_PLL_TO_ARO_MASK		(0x1 << CHANGE_PLL_TO_ARO_OFF)

#define ENABLE_CALIB_MODE_OFF		21
#define ENABLE_CALIB_MODE_MASK		(0x1 << ENABLE_CALIB_MODE_OFF)

/* cpu0_pll_ro_cfg2
 * cpu1_pll_ro_cfg2
 */
#define CPU_ARO_CTRL2(CPU)		(CPU_ARO_CTRL_BASE + 0x10 + (CPU * 0xc))

/* Initialize the calibration counters. */
#define INIT_CALIB_COUNTER_OFF		0
#define INIT_CALIB_COUNTER_MASK		(0x7fffffff << INIT_CALIB_COUNTER_OFF)

#define SET_ARO_TARGET_OFF		0
#define SET_ARO_TARGET_MASK		(0x7fffffff << SET_ARO_TARGET_OFF)

#define SET_REF_INIT_VAL_OFF		31
#define SET_REF_INIT_VAL_MASK		(0x1 << SET_REF_INIT_VAL_OFF)

/* cpu1_pll_ro_cfg2 */
#define DEV_GENERIC_CTRL_32		CPU_ARO_CTRL_BASE + (0x20)

/* Change the ARO bypass mux to point on the ARO */
#define CHANGE_ARO_BYPASS_OFF		0
#define CHANGE_ARO_BYPASS_MASK		(0x3 << CHANGE_ARO_BYPASS_OFF)

void reg_set_val(uintptr_t addr, uint32_t data, uint32_t mask)
{
	uint32_t reg_data;
	reg_data = mmio_read_32(addr);
	reg_data &= ~mask;
	reg_data |= data;
	mmio_write_32(addr, reg_data);
}

/* Take the ARO module out of reset,
 * set the correct clock tree muxing to start working with the ARO,
 * set the initial ARO freq targets (~600Mhz)
 * note: after This stage - PLL still drivers the CPU clock
 */
static unsigned int enable_aro_module(void)
{
	unsigned int mask, data;
	/* -Set the Ref counter to stop the CPU counter,
	 *  Needed for calibration mode
	 * -Take the ARO out of reset
	 */
	data = 0x1 << USR_RO_RESET_OFF;
	mask = USR_RO_RESET_MASK;
	data |= 0x1 << USR_REF_STOP_CPU_CNT_OFF;
	mask |= USR_REF_STOP_CPU_CNT_MASK;

	/* Write registers for cluster 0 and cluster 1 */
	reg_set_val(CPU_ARO_CTRL0(0), data, mask);
	reg_set_val(CPU_ARO_CTRL0(1), data, mask);

	/* -Take the update values
	 * -Choose taregt 1 Freq
	 * -Choose the PLL
	 * -Initial target freq in USER mode
	 * -Initial target freq in calibration mode
	 */
	data = 0x7 << UPDATED_VAL_OFF;
	mask = UPDATED_VAL_MASK;
	data |= 0x8 << CHOOSE_TRGT1_FREQ_OFF;
	mask |= CHOOSE_TRGT1_FREQ_MASK;
	data |= 0x8 << CHOOSE_PLL_OFF;
	mask |= CHOOSE_PLL_MASK;
	reg_set_val(CPU_ARO_CTRL1(0), data, mask);
	reg_set_val(CPU_ARO_CTRL1(1), data, mask);

	/* Change the ARO bypass mux to point on the ARO */
	data = 0x0 << CHANGE_ARO_BYPASS_OFF;
	mask = CHANGE_ARO_BYPASS_MASK;
	reg_set_val(DEV_GENERIC_CTRL_32, data, mask);

	return 0;
}

/* Initialize the calibration counters
 * set the ARO to intermediate targets
 * change from PLL to ARO
 */
static void start_aro_mode(unsigned int ref_counter_init_val, unsigned int cpu_counter_init_val)
{
	unsigned int data, mask;

	/* Initialize the calibration counters */
	data = (ref_counter_init_val - 1) << INIT_CALIB_COUNTER_OFF;
	mask = INIT_CALIB_COUNTER_MASK;
	reg_set_val(CPU_ARO_CTRL2(0), data, mask);
	reg_set_val(CPU_ARO_CTRL2(1), data, mask);

	/* Set ref_init_val_ld */
	data = 0x1 << SET_REF_INIT_VAL_OFF;
	mask = SET_REF_INIT_VAL_MASK;
	reg_set_val(CPU_ARO_CTRL2(0), data, mask);
	reg_set_val(CPU_ARO_CTRL2(1), data, mask);

	/* Wait 1 nanosecond */
	udelay(1);

	/* clear ref_init_val_ld */
	data = 0x0 << SET_REF_INIT_VAL_OFF;
	mask = SET_REF_INIT_VAL_MASK;
	reg_set_val(CPU_ARO_CTRL2(0), data, mask);
	reg_set_val(CPU_ARO_CTRL2(1), data, mask);

	/* Wait 1 microsecond */
	udelay(1);

	/* Set the ARO to intermediate targets */
	data = cpu_counter_init_val << SET_ARO_TARGET_OFF;
	mask = SET_ARO_TARGET_MASK;
	reg_set_val(CPU_ARO_CTRL2(0), data, mask);
	reg_set_val(CPU_ARO_CTRL2(1), data, mask);

	/* Change from PLL to ARO */
	data = 0x0 << CHANGE_PLL_TO_ARO_OFF;
	mask = CHANGE_PLL_TO_ARO_MASK;
	reg_set_val(CPU_ARO_CTRL1(0), data, mask);
	reg_set_val(CPU_ARO_CTRL1(1), data, mask);

	/* enable calibration mode. */
	data = 0x1 << ENABLE_CALIB_MODE_OFF;
	mask = ENABLE_CALIB_MODE_MASK;
	reg_set_val(CPU_ARO_CTRL1(0), data, mask);
	reg_set_val(CPU_ARO_CTRL1(1), data, mask);
}

/* list of allowed frequencies listed in order of enum hws_freq */
uint32_t cpu_freq_val[DDR_FREQ_LAST] = {
	2000,	/* CPU_FREQ_2000 */
	1800,	/* CPU_FREQ_1800 */
	1600,	/* CPU_FREQ_1600 */
	1400,	/* CPU_FREQ_1400 */
	1300,	/* CPU_FREQ_1300 */
	1200,	/* CPU_FREQ_1200 */
	1000,	/* CPU_FREQ_1000 */
	600,	/* CPU_FREQ_600 */
	800,	/* CPU_FREQ_800 */
};

/* return CPU frequency from sar */
static int sar_freq_get(int dev_num, enum hws_freq *freq)
{
	uint32_t clk_config;

	/* Read clk config from sar */
	clk_config = (mmio_read_32(SAR_REG_ADDR) >>
		RST2_CLOCK_FREQ_MODE_OFFS) &
		RST2_CLOCK_FREQ_MODE_MASK;

	switch (clk_config) {
	case CPU_2000_DDR_1200_RCLK_1200:
		*freq = CPU_FREQ_2000;
		break;
	case CPU_2000_DDR_1050_RCLK_1050:
		*freq = CPU_FREQ_2000;
		break;
	case CPU_1600_DDR_800_RCLK_800:
		*freq = CPU_FREQ_1600;
		break;
	case CPU_1800_DDR_1200_RCLK_1200:
		*freq = CPU_FREQ_1800;
		break;
	case CPU_1800_DDR_1050_RCLK_1050:
		*freq = CPU_FREQ_1800;
		break;
	case CPU_1600_DDR_1050_RCLK_1050:
		*freq = CPU_FREQ_1600;
		break;
	case CPU_1000_DDR_650_RCLK_650:
		*freq = CPU_FREQ_1000;
		break;
	case CPU_1300_DDR_800_RCLK_800:
		*freq = CPU_FREQ_1300;
		break;
	case CPU_1300_DDR_650_RCLK_650:
		*freq = CPU_FREQ_1300;
		break;
	case CPU_1200_DDR_800_RCLK_800:
		*freq = CPU_FREQ_1200;
		break;
	case CPU_1400_DDR_800_RCLK_800:
		*freq = CPU_FREQ_1400;
		break;
	case CPU_600_DDR_800_RCLK_800:
		*freq = CPU_FREQ_600;
		break;
	case CPU_800_DDR_800_RCLK_800:
		*freq = CPU_FREQ_800;
		break;
	case CPU_1000_DDR_800_RCLK_800:
		*freq = CPU_FREQ_1000;
		break;
	default:
		*freq = 0;
		return -1;
	}

	return 0;
}

int init_aro(void)
{
	unsigned int cpu_counter_init_val, max_freq_guardeband, ref_counter_init_val;
	unsigned int data, mask;
	uint32_t target_freq;
	enum hws_freq freq;

	sar_freq_get(0, &freq);
	target_freq = cpu_freq_val[freq];

	/* Set initial ARO freq targets (600 Mhz)
	 * After this stage, PLL still drives the CPU clock
	 */
	enable_aro_module();

	/* -Stop forcing USER mode, if callibration mode will be enabled
	 *  ARO would work in calibrationb mode
	 */
	data = 0x0 << USER_RO_SEL_TRGT_OFF;
	mask = USER_RO_SEL_TRGT_MASK;
	reg_set_val(CPU_ARO_CTRL1(0), data, mask);
	reg_set_val(CPU_ARO_CTRL1(1), data, mask);

	udelay(1);

	data = 0x0 << USER_UPDATE_RO_TRGT_OFF;
	mask = USER_UPDATE_RO_TRGT_MASK;
	reg_set_val(CPU_ARO_CTRL1(0), data, mask);
	reg_set_val(CPU_ARO_CTRL1(1), data, mask);

	/* 2000 and 1800 touch the device speed limit,
	 * therefore it requires a special flow when building
	 * the clock frequency from a certain stage, else the device may fail
	 */
	if (target_freq == (2000) || target_freq == (1800)) {
		max_freq_guardeband = 200;
		ref_counter_init_val = 5;
		cpu_counter_init_val = (((target_freq - max_freq_guardeband) / 25) * ref_counter_init_val);
		start_aro_mode(ref_counter_init_val, cpu_counter_init_val);

		/* CPU is now running in ARO mode*/
		udelay(100);

		/* update the Ref count to higher value to gain accuracy. */
		ref_counter_init_val = 10000;
		cpu_counter_init_val = ((target_freq / 25) * (ref_counter_init_val));
		start_aro_mode(ref_counter_init_val, cpu_counter_init_val);

	} else {
		max_freq_guardeband = 0;
		ref_counter_init_val = 5;
		cpu_counter_init_val = (((target_freq - max_freq_guardeband) / 25) * ref_counter_init_val);
		start_aro_mode(ref_counter_init_val, cpu_counter_init_val);

		/* CPU is now running in ARO mode*/
		udelay(100);
	}

	return 0;
}
