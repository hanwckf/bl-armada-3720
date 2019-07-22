/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */
#ifndef AP810_ARO_H_
#define AP810_ARO_H_

void ap810_aro_init(int target_freq, int ap_num);

enum cpu_supported_options {
	TARGET_FREQ_1000,
	TARGET_FREQ_1200,
	TARGET_FREQ_1600,
	TARGET_FREQ_1800,
	TARGET_FREQ_2000,
	TARGET_FREQ_2200,
	TARGET_FREQ_2400,
	TARGET_FREQ_2500,
	TARGET_FREQ_2700,
	TARGET_FREQ_3000,
};

#endif /* AP810_ARO_H_ */

