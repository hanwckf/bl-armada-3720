/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <ap810_setup.h>
#include <bl_common.h>
#include <debug.h>
#include <mmio.h>
#include "mss_scp_bootloader.h"
#include <marvell_plat_priv.h>
#include <platform_def.h>

#define MSS_AP_REG_BASE			0x580000
#define MSS_CP_REG_BASE			0x280000

/* MSS windows configuration */
#define MSS_AEBR(base)			(base + 0x160)
#define MSS_AIBR(base)			(base + 0x164)
#define MSS_AEBR_MASK			0xFFF
#define MSS_AIBR_MASK			0xFFF

#define MSS_EXTERNAL_SPACE		0x50000000
#define MSS_EXTERNAL_ACCESS_BIT		28
#define MSS_EXTERNAL_ADDR_MASK		0xfffffff
#define MSS_INTERNAL_ACCESS_BIT		28

#define MSS_ADDR_REMAP			0x6f0600
#define MSS_REMAP_WIN1_ALR(ap)		(MVEBU_REGS_BASE_AP(ap) + MSS_ADDR_REMAP + 0x10)
#define MSS_WIN_ALR_VAL			0xf401
#define MSS_REMAP_WIN1_AHR(ap)		(MVEBU_REGS_BASE_AP(ap) + MSS_ADDR_REMAP + 0x14)
#define MSS_WIN_AHR_VAL			0xf7f0
#define MSS_REMAP_WIN1_CR(ap)		(MVEBU_REGS_BASE_AP(ap) + MSS_ADDR_REMAP + 0x18)
#define MSS_WIN1_CR_ADDR_MASK		0x3f000000
#define MSS_WIN1_CR_REMAP_ADDR_OFFSET	20

#define GP_LOG_REG_0			0x6F43E0

static void bl2_store_num_of_cps(void)
{
	int i;

	for (i = 0; i < ap_get_count(); i++)
		mmio_write_32(MVEBU_REGS_BASE_AP(i) + GP_LOG_REG_0,
			      ap810_get_cp_per_ap_cnt(i));
}

static void bl2_plat_mss_remap(void)
{
	int i;
	/* This init the MSS remap to enable access for CP0 via CM3
	 * AP0 -> 0x40000000 MSS AIBR will change the address to 0xF000_0000,
	 * then MSS remap will remap the address to 0xEC00_0000
	 * CP0 -> 0x44000000 MSS AIBR will change the address to 0xF400_0000,
	 * then MSS remap will remap the address to 0x81_0000_0000
	 */
	/* MSS remap for CP1 */
	for (i = 0; i < ap_get_count(); i++) {
		mmio_write_32(MSS_REMAP_WIN1_ALR(i), MSS_WIN_ALR_VAL);
		mmio_write_32(MSS_REMAP_WIN1_AHR(i), MSS_WIN_AHR_VAL);
		mmio_write_32(MSS_REMAP_WIN1_CR(i), MSS_WIN1_CR_ADDR_MASK |
						    (MVEBU_CP_REGS_BASE(i, 0) >>
						    MSS_WIN1_CR_REMAP_ADDR_OFFSET));
	}
}

/*******************************************************************************
 * Transfer SCP_BL2 from Trusted RAM using the SCP Download protocol.
 * Return 0 on success, -1 otherwise.
 ******************************************************************************/
int bl2_plat_handle_scp_bl2(image_info_t *scp_bl2_image_info)
{
	int ret;

	INFO("BL2: Initiating SCP_BL2 transfer to SCP\n");
	printf("BL2: Initiating SCP_BL2 transfer to SCP\n");

	/* initialize time (for delay functionality) */
	plat_delay_timer_init();

	/* Remap MSS */
	bl2_plat_mss_remap();

	/* Store num of CPs for MSS use */
	bl2_store_num_of_cps();

	ret = scp_bootloader_transfer((void *)scp_bl2_image_info->image_base,
		scp_bl2_image_info->image_size);

	if (ret == 0)
		INFO("BL2: SCP_BL2 transferred to SCP\n");
	else
		ERROR("BL2: SCP_BL2 transfer failure\n");

	return ret;
}

uintptr_t bl2_plat_get_cp_mss_regs(int ap_idx, int cp_idx)
{
	return MVEBU_CP_REGS_BASE(ap_idx, cp_idx) + MSS_CP_REG_BASE;
}

uintptr_t bl2_plat_get_ap_mss_regs(int ap_idx)
{
	return MVEBU_REGS_BASE_AP(ap_idx) + MSS_AP_REG_BASE;
}

uint32_t bl2_plat_get_cp_count(int ap_idx)
{
	return ap810_get_cp_per_ap_cnt(ap_idx);
}

uint32_t bl2_plat_get_ap_count(void)
{
	return ap_get_count();
}

void bl2_plat_configure_mss_windows(uintptr_t mss_regs)
{
	/* CP address are bigger than 32 bits:
	 * CPx on AP0 0x8100000000 + 0x700000000 * x
	 * CPx on AP1 0x9e00000000 + 0x700000000 * x
	 *
	 * AP address is 32 bit.
	 *
	 * Shifting mss_regs >> 32 tells us if it's CP or AP.
	 */
	if (mss_regs >> 32) {
		/* Configure AIBR (AXI Internal Address Bus extension) to map
		 * CP base address. This allows each CP to access it's
		 * own memory space.
		 */
		mmio_write_32(MSS_AIBR(mss_regs), mss_regs >> 28);
	} else {
		/* Configure AIBR (AXI Internal Address Bus extension) to allow AP
		 * to access it's own memory space
		 */
		mmio_write_32(MSS_AIBR(mss_regs),
			      ((0xf0000000 >> MSS_INTERNAL_ACCESS_BIT) & MSS_AIBR_MASK));
	}

	/* Configure AEBR (AXI Internal Address Bus extension) to map access:
	 * CP: Access from CP to AP
	 * AP: Access from AP to DRAM
	 */
	mmio_write_32(MSS_AEBR(mss_regs), ((0xe0000000 >> MSS_EXTERNAL_ACCESS_BIT) & MSS_AEBR_MASK));
}
