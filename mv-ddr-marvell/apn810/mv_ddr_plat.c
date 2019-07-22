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
#include "mv_ddr_mc6.h"
#include "mv_ddr_xor_v2.h"
#include "mv_ddr_plat.h"
#include "ddr_topology_def.h"
#include "mv_ddr_common.h"
/* TODO: remove this include when removing attribute mechanism */
#include "ddr3_training_ip_db.h"
#include "mv_ddr_validate.h"

#define DDR_INTERFACES_NUM		1

static unsigned int ap_regs_base;
static int is_pre_configured;

void mv_ddr_base_set(unsigned int base)
{
	ap_regs_base = base;
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

void mmio_write2_32(u32 val, u32 addr)
{
	mmio_write_32(addr, val);
}

u32 *snps_ext_swizzle_cfg_get(void)
{
	return NULL;
}

static void mv_ddr_mem_scrubbing(void)
{
	uint64_t val = 0;
	uint64_t tot_mem_sz;
	struct mv_ddr_iface *curr_iface = mv_ddr_iface_get();

	tot_mem_sz = mv_ddr_mem_sz_get();

	printf("mv_ddr: scrubbing memory...\n");

	/* scrub memory up to non-dram memory region */
	if ((curr_iface->iface_base_addr + tot_mem_sz < NON_DRAM_MEM_RGN_START_ADDR) ||
	    (curr_iface->iface_base_addr >= NON_DRAM_MEM_RGN_END_ADDR))
		mv_ddr_dma_memset(curr_iface->iface_base_addr, tot_mem_sz, val);
	else
		mv_ddr_dma_memset(curr_iface->iface_base_addr, NON_DRAM_MEM_RGN_START_ADDR, val);

	/* scrub memory up to the end */
	if ((tot_mem_sz >= NON_DRAM_MEM_RGN_END_ADDR) &&
	    (curr_iface->iface_base_addr < NON_DRAM_MEM_RGN_START_ADDR))
		mv_ddr_dma_memset(NON_DRAM_MEM_RGN_END_ADDR,
				  tot_mem_sz - NON_DRAM_MEM_RGN_START_ADDR, val);
}

int mv_ddr_pre_config(void)
{
	struct mv_ddr_iface *curr_iface;

	/* ensure the function is called once */
	if (is_pre_configured)
		return 0;

	curr_iface = mv_ddr_iface_get();
	mv_ddr_base_set(curr_iface->ap_base);

	/* TODO: remove attribute mechanism */
	ddr3_tip_dev_attr_init(0);
	ddr3_tip_dev_attr_set(0, MV_ATTR_OCTET_PER_INTERFACE, DDR_INTERFACE_OCTETS_NUM);

	if (mv_ddr_topology_map_update()) {
		printf("mv_ddr: failed to update topology\n");
		return -1;
	}

	/* ensure the function is called once */
	is_pre_configured = 1;

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

	/* unset is_pre_configured variable */
	is_pre_configured = 0;

	return 0;
}

unsigned int mv_ddr_init_freq_get(void)
{
	printf("error: %s: unset frequency found\n", __func__);

	return MV_DDR_FREQ_800;
}

int mv_ddr_mc_config(void)
{
	struct mv_ddr_iface *iface = mv_ddr_iface_get();
	int ecc_is_ena;

	ecc_is_ena = mv_ddr_is_ecc_ena();
	if (mv_ddr_mc6_config(MC6_BASE(iface->id), iface->iface_base_addr, ecc_is_ena)) {
		printf("error: %s failed\n", __func__);
		return -1;
	}

	return 0;
}


/* enable memory controllers */
int mv_ddr_mc_ena(void)
{
	struct mv_ddr_iface *iface = mv_ddr_iface_get();
	mv_ddr_mc6_init(MC6_BASE(iface->id));

	return 0;
}

int mv_ddr_phy_config(void)
{
	return snps_init(ap_regs_base + SNPS_PHY_OFFSET);
}

void mv_ddr_mmap_config(void)
{
	struct mv_ddr_iface *curr_iface = mv_ddr_iface_get();
	mv_ddr_mc6_sizes_cfg(MC6_BASE(curr_iface->id), curr_iface->iface_base_addr);
}
