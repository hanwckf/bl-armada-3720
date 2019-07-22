/*******************************************************************************
Copyright (C) 2018 Marvell International Ltd.

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the three
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, either version 2 of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************************
Marvell GNU General Public License FreeRTOS Exception

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the Lesser
General Public License Version 2.1 plus the following FreeRTOS exception.
An independent module is a module which is not derived from or based on
FreeRTOS.
Clause 1:
Linking FreeRTOS statically or dynamically with other modules is making a
combined work based on FreeRTOS. Thus, the terms and conditions of the GNU
General Public License cover the whole combination.
As a special exception, the copyright holder of FreeRTOS gives you permission
to link FreeRTOS with independent modules that communicate with FreeRTOS solely
through the FreeRTOS API interface, regardless of the license terms of these
independent modules, and to copy and distribute the resulting combined work
under terms of your choice, provided that:
1. Every copy of the combined work is accompanied by a written statement that
details to the recipient the version of FreeRTOS used and an offer by yourself
to provide the FreeRTOS source code (including any modifications you may have
made) should the recipient request it.
2. The combined work is not itself an RTOS, scheduler, kernel or related
product.
3. The independent modules add significant and primary functionality to
FreeRTOS and do not merely extend the existing functionality already present in
FreeRTOS.
Clause 2:
FreeRTOS may not be used for any competitive or comparative purpose, including
the publication of any form of run time or compile time metric, without the
express permission of Real Time Engineers Ltd. (this is the norm within the
industry and is intended to ensure information accuracy).

********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.

	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.

	* Neither the name of Marvell nor the names of its contributors may be
	  used to endorse or promote products derived from this software without
	  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include "mv_ddr_atf_wrapper.h"
#include "snps_if.h"
#include "snps_regs.h"
#include "mv_ddr_mc6.h"
#include "mv_ddr_xor_v2.h"
#include "mv_ddr_plat.h"
#include "ddr_topology_def.h"
#include "mv_ddr_common.h"
/* TODO: remove this include when removing attribute mechanism */
#include "ddr3_training_ip_db.h"
#include "mv_ddr_validate.h"

/*
 * SNPS address lines' PHY to IO mapping configuration.
 * APN807 address lines' PHY to IO routing is swizzled;
 * not connected in ascending order, which is SNPS default configuration.
 */
static u32 snps_swizzle_cfg[HWT_SWIZZLE_HWT_REGS_NUM] = {
	0x0018,		/* addr0 */
	0x0004,		/* addr1 */
	0x0005,		/* addr2 */
	0x0014,		/* addr3 */
	0x0013,		/* addr4 */
	0x0001,		/* addr5 */
	0x0007,		/* addr6 */
	0x0006,		/* addr7 */
	0x0002,		/* addr8 */
	0x0008,		/* addr9 */
	0x0019,		/* addr10 */
	0x0009,		/* addr11 */
	0x0003,		/* addr12 */
	0x0017,		/* addr13 */
	0x0,		/* addr14 */
	0x0,		/* addr15 */
	0x0,		/* addr17 */
	0x000a,		/* actn */
	0x000b,		/* bank0 */
	0x000d,		/* bank1 */
	0x0,		/* bank2 */
	0x000e,		/* bg0 */
	0x000c,		/* bg1 */
	0x001a,		/* casn */
	0x0012,		/* rasn */
	0x0016,		/* wen */
	0x0		/* parityin */
};

static unsigned int ap_regs_base;

/* remap overlapping dram region to the top */
#define ALIAS_BANKED_UID(n)	(0x800 + 0x400 * (n))
#define DRAM_UID		3
#define CCU_MC_RCR_BASE		0x300
#define CCU_MC_RCR_REG		(ALIAS_BANKED_UID(DRAM_UID) + CCU_MC_RCR_BASE)
#define REMAP_EN_ENA		1
#define REMAP_EN_OFFS		0
#define REMAP_EN_MASK		0x1
#define REMAP_SIZE_OFFS		20
#define REMAP_SIZE_MASK		0xfff
#define CCU_MC_RSBR_BASE	0x304
#define CCU_MC_RSBR_REG		(ALIAS_BANKED_UID(DRAM_UID) + CCU_MC_RSBR_BASE)
#define SOURCE_BASE_OFFS	10
#define SOURCE_BASE_MASK	0xfffff
#define CCU_MC_RTBR_BASE	0x308
#define CCU_MC_RTBR_REG		(ALIAS_BANKED_UID(DRAM_UID) + CCU_MC_RTBR_BASE)
#define TARGET_BASE_OFFS	10
#define TARGET_BASE_MASK	0xfffff

int mv_ddr_mc_remap(void)
{
	uintptr_t remap_target = NON_DRAM_MEM_RGN_START_ADDR;
	uintptr_t remap_source = mv_ddr_mem_sz_get();
	uint64_t remap_size = NON_DRAM_MEM_RGN_END_ADDR - NON_DRAM_MEM_RGN_START_ADDR;
	u32 val;

	if (remap_source < NON_DRAM_MEM_RGN_START_ADDR)
		return 0;

	if ((remap_size == 0) || /* can't be zero */
	    (remap_size >> 32) || /* can't be more than 4GB */
	    (remap_size % (1 << 20))) { /* must be multiple of 1MB */
		printf("%s: incorrect mc remap size found\n", __func__);
		return -1;
	}

	if (remap_target % remap_size) { /* must be multiple of remap size */
		printf("%s: incorrect mc remap target base addr found\n", __func__);
		return -1;
	}

	if (remap_source % remap_size) { /* must be multiple of remap size */
		printf("%s: incorrect mc remap source base addr found\n", __func__);
		return -1;
	}

	/* set mc remap source base to the top of dram */
	remap_source >>= 20; /* in MB */
	val = (remap_source & SOURCE_BASE_MASK) << SOURCE_BASE_OFFS;
	reg_write(CCU_MC_RSBR_REG, val);

	/* set mc remap target base to the overlapping dram region */
	remap_target >>= 20; /* in MB */
	val = (remap_target & TARGET_BASE_MASK) << TARGET_BASE_OFFS;
	reg_write(CCU_MC_RTBR_REG, val);

	/* set mc remap size to the size of the overlapping dram region */
	remap_size >>= 20; /* in MB */
	val = ((remap_size - 1) & REMAP_SIZE_MASK) << REMAP_SIZE_OFFS;

	/* enable remapping */
	val |= (REMAP_EN_ENA << REMAP_EN_OFFS);
	reg_write(CCU_MC_RCR_REG, val);

	return 0;
}

void mv_ddr_base_set(unsigned int base)
{
	ap_regs_base = base;
}

u32 *snps_ext_swizzle_cfg_get(void)
{
	return snps_swizzle_cfg;
}

/* TODO: relocate to wrapper file */
/* register accessor functions */
void reg_write(u32 addr, u32 val)
{
	mmio_write_32(ap_regs_base + addr, val);
}

u32 reg_read(u32 addr)
{
	return mmio_read_32(ap_regs_base + addr);
}

void reg_bit_set(u32 addr, u32 mask)
{
	mmio_write_32(ap_regs_base + addr,
		      mmio_read_32(ap_regs_base + addr) | mask);
}

void reg_bit_clr(u32 addr, u32 mask)
{
	mmio_write_32(ap_regs_base + addr,
		      mmio_read_32(ap_regs_base + addr) & ~mask);
}

void reg_bit_clrset(u32 addr, u32 val, u32 mask)
{
	mmio_clrsetbits_32(ap_regs_base + addr, mask, val);
}

static void mv_ddr_mem_scrubbing(void)
{
	uint64_t val = 0;
	uint64_t tot_mem_sz;

	tot_mem_sz = mv_ddr_mem_sz_get();

	printf("mv_ddr: scrubbing memory...\n");

	/* scrub memory up to non-dram memory region */
	if (tot_mem_sz < NON_DRAM_MEM_RGN_START_ADDR)
		mv_ddr_dma_memset(0, tot_mem_sz, val);
	else
		mv_ddr_dma_memset(0, NON_DRAM_MEM_RGN_START_ADDR, val);

	/* scrub memory up to the end */
	if (tot_mem_sz >= NON_DRAM_MEM_RGN_END_ADDR)
		mv_ddr_dma_memset(NON_DRAM_MEM_RGN_END_ADDR,
				  tot_mem_sz - NON_DRAM_MEM_RGN_START_ADDR, val);
}

int mv_ddr_pre_config(void)
{
	struct mv_ddr_iface *curr_iface = mv_ddr_iface_get();
	mv_ddr_base_set(curr_iface->ap_base);

	/* TODO: remove attribute mechanism */
	ddr3_tip_dev_attr_init(0);
	ddr3_tip_dev_attr_set(0, MV_ATTR_OCTET_PER_INTERFACE, DDR_INTERFACE_OCTETS_NUM);

	if (mv_ddr_topology_map_update()) {
		printf("mv_ddr: failed to update topology\n");
		return -1;
	}

	return 0;
}

int mv_ddr_post_config(void)
{
	struct mv_ddr_iface *iface = mv_ddr_iface_get();

	if (mv_ddr_is_ecc_ena())
		mv_ddr_mem_scrubbing();

	/* Update the size of the iface in bytes */
	iface->iface_byte_size = mv_ddr_mem_sz_get();

	if (iface->validation != MV_DDR_VAL_DIS) {
		if (mv_ddr_validate())
			printf("DRAM validation interface %d start address 0x%llx\n",
				iface->id, iface->iface_base_addr);
	}

	return 0;
}

/* return ddr frequency from sar */
#define CPU_2000_DDR_1200_RCLK_1200	0x0
#define CPU_1600_DDR_1200_RCLK_1200	0xd
#define CPU_1300_DDR_800_RCLK_800	0x14
static enum mv_ddr_freq mv_ddr_sar_freq_get(void)
{
	enum mv_ddr_freq freq;
	u32 ddr_clk_config;

	/* read ddr clk config from sar */
	ddr_clk_config = (reg_read(SAR_REG_ADDR) >>
		RST2_CLOCK_FREQ_MODE_OFFS) &
		RST2_CLOCK_FREQ_MODE_MASK;

	switch (ddr_clk_config) {
	case CPU_1600_DDR_1200_RCLK_1200:
	case CPU_2000_DDR_1200_RCLK_1200:
		freq = MV_DDR_FREQ_1200;
		break;
	case CPU_1300_DDR_800_RCLK_800:
		freq = MV_DDR_FREQ_800;
		break;
	default:
		printf("mv_ddr: ddr freq %x unsupported\n", ddr_clk_config);
		freq = 0;
	}

	return freq;
}

unsigned int mv_ddr_init_freq_get(void)
{
	return mv_ddr_sar_freq_get();
}

int mv_ddr_mc_config(void)
{
	struct mv_ddr_iface *iface = mv_ddr_iface_get();

	if (mv_ddr_mc6_config(MC6_BASE, iface->iface_base_addr, mv_ddr_is_ecc_ena())) {
		printf("error: %s failed\n", __func__);
		return -1;
	}

	return 0;
}


/* enable memory controllers */
int mv_ddr_mc_ena(void)
{
	mv_ddr_mc6_init(MC6_BASE);

	return 0;
}

int mv_ddr_phy_config(void)
{
	return snps_init(ap_regs_base + SNPS_PHY_OFFSET);
}

void mv_ddr_mmap_config(void)
{

}
