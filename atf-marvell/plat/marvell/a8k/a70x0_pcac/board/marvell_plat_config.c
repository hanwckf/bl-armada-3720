/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <armada_common.h>
#include <mvebu_def.h>
#include <pci_ep.h>

/*
 * If bootrom is currently at BLE there's no need to include the memory
 * maps structure at this point
 */
#ifndef IMAGE_BLE

/*****************************************************************************
 * AMB Configuration
 *****************************************************************************
 */
struct addr_map_win *amb_memory_map;

int marvell_get_amb_memory_map(struct addr_map_win **win, uint32_t *size,
			       uintptr_t base)
{
	*win = amb_memory_map;
	if (*win == NULL)
		*size = 0;
	else
		*size = sizeof(amb_memory_map)/sizeof(amb_memory_map[0]);

	return 0;
}
#endif

/*****************************************************************************
 * IO WIN Configuration
 *****************************************************************************
 */
struct addr_map_win io_win_memory_map[] = {
#ifndef IMAGE_BLE
	/* MCI 0 indirect window */
	{MVEBU_MCI_REG_BASE_REMAP(0),	0x100000,	MCI_0_TID},
	/* MCI 1 indirect window */
	{MVEBU_MCI_REG_BASE_REMAP(1),	0x100000,	MCI_1_TID},
#endif
};

uint32_t marvell_get_io_win_gcr_target(int ap_index)
{
	return PIDI_TID;
}

int marvell_get_io_win_memory_map(int ap_index, struct addr_map_win **win,
				  uint32_t *size)
{
	*win = io_win_memory_map;
	if (*win == NULL)
		*size = 0;
	else
		*size = sizeof(io_win_memory_map)/sizeof(io_win_memory_map[0]);

	return 0;
}

#ifndef IMAGE_BLE
/*****************************************************************************
 * IOB Configuration
 *****************************************************************************
 */
struct addr_map_win iob_memory_map[] = {
	/* PEX1_X1 window */
	{0x00000000f7000000,	0x1000000,	PEX1_TID},
	/* PEX2_X1 window */
	{0x00000000f8000000,	0x1000000,	PEX2_TID},
	/* PEX0_X4 window */
	{0x0000008000000000,	0x80000000000,	PEX0_TID},
};

int marvell_get_iob_memory_map(struct addr_map_win **win, uint32_t *size,
			       uintptr_t base)
{
	*win = iob_memory_map;
	*size = sizeof(iob_memory_map)/sizeof(iob_memory_map[0]);

	return 0;
}
#endif

/*****************************************************************************
 * CCU Configuration
 *****************************************************************************
 */
struct addr_map_win ccu_memory_map[] = {
#ifdef IMAGE_BLE
	{0x00000000f2000000,	0x4000000,	IO_0_TID}, /* IO window */
#else
	{0x00000000f2000000,	0xe000000,	IO_0_TID}, /* IO window */
	{0x0000008000000000,	0x80000000000,	IO_0_TID}, /* IO window */
#endif
};

uint32_t marvell_get_ccu_gcr_target(int ap)
{
	return DRAM_0_TID;
}

int marvell_get_ccu_memory_map(int ap_index, struct addr_map_win **win,
			       uint32_t *size)
{
	*win = ccu_memory_map;
	*size = sizeof(ccu_memory_map)/sizeof(ccu_memory_map[0]);

	return 0;
}

#ifdef IMAGE_BLE
/*****************************************************************************
 * PCIe Configuration
 *****************************************************************************
 */
struct pci_hw_cfg a70x0_pci_hw_cfg = {
	.delay_cfg	= 1,
	.master_en	= 1,
	.lane_width	= 4,
	.lane_ids	= {0, 1, 2, 3},
	.clk_src	= 0,
	.clk_out	= 0,	/* clk is not output */
	.is_end_point	= 0,
	.mac_base	= MVEBU_PCIE_X4_MAC_BASE(0),
	.comphy_base	= MVEBU_COMPHY_BASE(0),
	.hpipe_base	= MVEBU_HPIPE_BASE(0),
	.dfx_base	= MVEBU_CP_REGS_BASE(0) + MVEBU_CP_DFX_OFFSET,
};

struct pci_hw_cfg *plat_get_pcie_hw_data(void)
{
	return &a70x0_pci_hw_cfg;
}

/*****************************************************************************
 * SKIP IMAGE Configuration
 *****************************************************************************
 */
#if PLAT_RECOVERY_IMAGE_ENABLE
void *plat_get_skip_image_data(void)
{
	/* No recovery button on A7k-PCAC board */
	return NULL;
}
#endif
#endif
