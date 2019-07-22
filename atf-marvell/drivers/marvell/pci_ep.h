/*
 * Copyright (C) 2016 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef _PCIE_EP_H_
#define _PCIE_EP_H_

#define PCIE_MAX_BARS		6
#define PCIE_MAX_LANES		16

struct pci_hw_cfg {
	uint8_t delay_cfg;
	uint8_t master_en;
	uint8_t	lane_width;
	uint8_t	lane_ids[PCIE_MAX_LANES];
	uint8_t	clk_src;
	uint8_t	clk_out;
	uint8_t	is_end_point;
	uintptr_t mac_base;
	uintptr_t comphy_base;
	uintptr_t hpipe_base;
	uintptr_t dfx_base;
};

void dw_pcie_ep_init(uintptr_t dw_base, uint8_t delay_cfg, uint8_t master_en);
int comphy_pcie_power_up(uint32_t lane, struct pci_hw_cfg *hw);

#endif /* _PCIE_EP_H_ */

