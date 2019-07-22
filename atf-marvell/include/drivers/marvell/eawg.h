/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */
#ifndef _EAWG_H_
#define _EAWG_H_

#include <stdint.h>

#define MAX_TRANSACTIONS	48
#define PRIMARY_CPU_TRANS	1
#define TRANS_PER_PLL		4

struct eawg_transaction {
	uint32_t address;
	uint32_t data;
	uint32_t delay;
};

void disable_eawg(int ap_num);
int eawg_load_transactions(struct eawg_transaction *conf_input, int transactions_num, int ap_num);
void eawg_start(int ap_num);
int eawg_check_is_done(uintptr_t scratch_pad_reg, int ap_num);

#endif /* _EAWG_H_ */
