/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef AP810_INIT_CLOCKS_H_
#define AP810_INIT_CLOCKS_H_

enum ddr_freq {
	DDR_FREQ_LOW_FREQ,
	DDR_FREQ_650,
	DDR_FREQ_667,
	DDR_FREQ_800,
	DDR_FREQ_933,
	DDR_FREQ_1066,
	DDR_FREQ_900,
	DDR_FREQ_1000,
	DDR_FREQ_1050,
	DDR_FREQ_1200,
	DDR_FREQ_1333,
	DDR_FREQ_1466,
	DDR_FREQ_1600,
	DDR_FREQ_LAST,
	DDR_FREQ_SAR
};

int ap810_clocks_init(int ap_count);
void plat_dram_freq_update(enum ddr_freq freq_option);

#endif /* AP810_INIT_CLOCKS_H_ */

