/*
 * Copyright (C) 2016 - 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */
 
#include <arch_helpers.h>
#include <plat_marvell.h>
#include <debug.h>
#include <mmio.h>
#include <pci_ep.h>

/* Set a weak stub for platforms that don't use PCIe end point */
#pragma weak plat_get_pcie_hw_data
struct pci_hw_cfg *plat_get_pcie_hw_data(void)
{
	return 0;
}

void ble_plat_pcie_ep_setup(void)
{
	int ret, lane = 0;
	struct pci_hw_cfg *hw_cfg;

	hw_cfg = (struct pci_hw_cfg *)plat_get_pcie_hw_data();
	if (!hw_cfg)
		return;

	INFO("Setting up PCI as end point\n");

	/* First setup all COMPHY lanes */
	while (lane < hw_cfg->lane_width) {
		ret = comphy_pcie_power_up(hw_cfg->lane_ids[lane], hw_cfg);
		if (ret == 0) {
			ERROR(" Failed to setup SERDES lane %d\n",
			      hw_cfg->lane_ids[lane]);
			return;
		}
		lane++;
	}

	/* Now setup the MAC */
	dw_pcie_ep_init(hw_cfg->mac_base, hw_cfg->delay_cfg, hw_cfg->master_en);
}
