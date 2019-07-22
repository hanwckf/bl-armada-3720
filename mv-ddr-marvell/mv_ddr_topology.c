/*******************************************************************************
Copyright (C) 2016 Marvell International Ltd.

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

#if defined(MV_DDR) /* U-BOOT MARVELL 2013.01 */
#include "ddr_mv_wrapper.h"
#include "mv_ddr_plat.h"
#elif defined(MV_DDR_ATF) /* MARVELL ATF */
#include "mv_ddr_atf_wrapper.h"
#include "mv_ddr_plat.h"
#elif defined(CONFIG_A3700)
#include "mv_ddr_a3700_wrapper.h"
#include "mv_ddr_plat.h"
#else /* U-BOOT SPL */
#include "ddr_ml_wrapper.h"
#include "a38x/mv_ddr_plat.h"
#endif

#include "mv_ddr_topology.h"
#include "mv_ddr_common.h"
#include "mv_ddr_spd.h"
#include "ddr_topology_def.h"
#include "ddr3_training_ip_db.h"
#include "ddr3_training_ip.h"
#include "mv_ddr_training_db.h"

unsigned int mv_ddr_cl_calc(unsigned int taa_min, unsigned int tclk)
{
	unsigned int cl = ceil_div(taa_min, tclk);

	return mv_ddr_spd_supported_cl_get(cl);

}

unsigned int mv_ddr_cwl_calc(unsigned int tclk)
{
	unsigned int cwl;

	if (tclk >= 1250)
		cwl = 9;
	else if (tclk >= 1071)
		cwl = 10;
	else if (tclk >= 938)
		cwl = 11;
	else if (tclk >= 833)
		cwl = 12;
	else if (tclk >= 750)
		cwl = 14;
	else if (tclk >= 625)
		cwl = 16;
	else
		cwl = 0;

	return cwl;
}

int mv_ddr_topology_map_update(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);
	unsigned int octets_per_if_num = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	enum mv_ddr_speed_bin speed_bin_index;
	enum mv_ddr_freq freq = MV_DDR_FREQ_LAST;
	unsigned int tclk;
	unsigned char val = 0;
	int i;

	if (iface_params->memory_freq == MV_DDR_FREQ_SAR)
		iface_params->memory_freq = mv_ddr_init_freq_get();

	if (tm->cfg_src == MV_DDR_CFG_SPD) {
		/* check dram device type */
		val = mv_ddr_spd_dev_type_get(&tm->spd_data);
		if (val != MV_DDR_SPD_DEV_TYPE_DDR4) {
			printf("mv_ddr: unsupported dram device type found\n");
			return -1;
		}

		/* update topology map with timing data */
		if (mv_ddr_spd_timing_calc(&tm->spd_data, tm->timing_data) > 0) {
			printf("mv_ddr: negative timing data found\n");
			return -1;
		}

		/* update device width in topology map */
		iface_params->bus_width = mv_ddr_spd_dev_width_get(&tm->spd_data);

		/* update die capacity in topology map */
		iface_params->memory_size = mv_ddr_spd_die_capacity_get(&tm->spd_data);

		/* update bus bit mask in topology map */
		tm->bus_act_mask = mv_ddr_bus_bit_mask_get();

		/* update cs bit mask in topology map */
		val = mv_ddr_spd_cs_bit_mask_get(&tm->spd_data);
		for (i = 0; i < octets_per_if_num; i++)
			iface_params->as_bus_params[i].cs_bitmask = val;

		/* check dram module type */
		val = mv_ddr_spd_module_type_get(&tm->spd_data);
		switch (val) {
		case MV_DDR_SPD_MODULE_TYPE_UDIMM:
		case MV_DDR_SPD_MODULE_TYPE_SO_DIMM:
		case MV_DDR_SPD_MODULE_TYPE_MINI_UDIMM:
		case MV_DDR_SPD_MODULE_TYPE_72BIT_SO_UDIMM:
		case MV_DDR_SPD_MODULE_TYPE_16BIT_SO_DIMM:
		case MV_DDR_SPD_MODULE_TYPE_32BIT_SO_DIMM:
			break;
		default:
			printf("mv_ddr: unsupported dram module type found\n");
			return -1;
		}

		/* update mirror bit mask in topology map */
		val = mv_ddr_spd_mem_mirror_get(&tm->spd_data);
		for (i = 0; i < octets_per_if_num; i++)
			iface_params->as_bus_params[i].mirror_enable_bitmask = val << 1;

		tclk = 1000000 / mv_ddr_freq_get(iface_params->memory_freq);
		/* update cas write latency (cwl) */
		val = mv_ddr_cwl_calc(tclk);
		if (val == 0) {
			printf("mv_ddr: unsupported cas write latency value found\n");
			return -1;
		}
		iface_params->cas_wl = val;

		/* update cas latency (cl) */
		mv_ddr_spd_supported_cls_calc(&tm->spd_data);
		val = mv_ddr_cl_calc(tm->timing_data[MV_DDR_TAA_MIN], tclk);
		if (val == 0) {
			printf("mv_ddr: unsupported cas latency value found\n");
			return -1;
		}
		iface_params->cas_l = val;
	} else if (tm->cfg_src == MV_DDR_CFG_DEFAULT) {
		/* set cas and cas-write latencies per speed bin, if they unset */
		speed_bin_index = iface_params->speed_bin_index;
		freq = iface_params->memory_freq;

		if (iface_params->cas_l == 0)
			iface_params->cas_l = mv_ddr_cl_val_get(speed_bin_index, freq);

		if (iface_params->cas_wl == 0)
			iface_params->cas_wl = mv_ddr_cwl_val_get(speed_bin_index, freq);
	}

	return 0;
}

unsigned short mv_ddr_bus_bit_mask_get(void)
{
	unsigned short pri_and_ext_bus_width = 0x0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int octets_per_if_num = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);

	if (tm->cfg_src == MV_DDR_CFG_SPD) {
		enum mv_ddr_pri_bus_width pri_bus_width = mv_ddr_spd_pri_bus_width_get(&tm->spd_data);
		enum mv_ddr_bus_width_ext bus_width_ext = mv_ddr_spd_bus_width_ext_get(&tm->spd_data);

		switch (pri_bus_width) {
		case MV_DDR_PRI_BUS_WIDTH_16:
			pri_and_ext_bus_width = BUS_MASK_16BIT;
			break;
		case MV_DDR_PRI_BUS_WIDTH_32:
			pri_and_ext_bus_width = BUS_MASK_32BIT;
			break;
		case MV_DDR_PRI_BUS_WIDTH_64:
			pri_and_ext_bus_width = MV_DDR_64BIT_BUS_MASK;
			break;
		default:
			pri_and_ext_bus_width = 0x0;
		}

		if (bus_width_ext == MV_DDR_BUS_WIDTH_EXT_8)
			pri_and_ext_bus_width |= 1 << (octets_per_if_num - 1);
	}

	return pri_and_ext_bus_width;
}

unsigned int mv_ddr_if_bus_width_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int bus_width;

	switch (tm->bus_act_mask) {
	case BUS_MASK_16BIT:
	case BUS_MASK_16BIT_ECC:
	case BUS_MASK_16BIT_ECC_PUP3:
		bus_width = 16;
		break;
	case BUS_MASK_32BIT:
	case BUS_MASK_32BIT_ECC:
	case MV_DDR_32BIT_ECC_PUP8_BUS_MASK:
		bus_width = 32;
		break;
	case MV_DDR_64BIT_BUS_MASK:
	case MV_DDR_64BIT_ECC_PUP8_BUS_MASK:
		bus_width = 64;
		break;
	default:
		printf("mv_ddr: unsupported bus active mask parameter found\n");
		bus_width = 0;
	}

	return bus_width;
}

unsigned int mv_ddr_cs_num_get(void)
{
	unsigned int cs_num = 0;
	unsigned int cs, sphy;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);
	unsigned int sphy_max = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);

	for (sphy = 0; sphy < sphy_max; sphy++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sphy);
		break;
	}

	for (cs = 0; cs < MAX_CS_NUM; cs++) {
		VALIDATE_ACTIVE(iface_params->as_bus_params[sphy].cs_bitmask, cs);
		cs_num++;
	}

	return cs_num;
}

int mv_ddr_is_ecc_ena(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (DDR3_IS_ECC_PUP4_MODE(tm->bus_act_mask) ||
	    DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask) ||
	    DDR3_IS_ECC_PUP8_MODE(tm->bus_act_mask))
		return 1;
	else
		return 0;
}

/* translate topology map definition to real memory size in bits */
static unsigned int mem_size[] = {
	ADDR_SIZE_512MB,
	ADDR_SIZE_1GB,
	ADDR_SIZE_2GB,
	ADDR_SIZE_4GB,
	ADDR_SIZE_8GB
	/* TODO: add capacity up to 256GB */
};

unsigned long long mv_ddr_mem_sz_per_cs_get(void)
{
	unsigned long long mem_sz_per_cs;
	unsigned int i, sphys, sphys_per_dunit;
	unsigned int sphy_max = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);

	/* calc number of active subphys excl. ecc one */
	for (i = 0, sphys = 0; i < sphy_max - 1; i++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, i);
		sphys++;
	}

	/* calc number of subphys per ddr unit */
	if (iface_params->bus_width == MV_DDR_DEV_WIDTH_8BIT)
		sphys_per_dunit = MV_DDR_ONE_SPHY_PER_DUNIT;
	else if (iface_params->bus_width == MV_DDR_DEV_WIDTH_16BIT)
		sphys_per_dunit = MV_DDR_TWO_SPHY_PER_DUNIT;
	else {
		printf("mv_ddr: unsupported bus width type found\n");
		return 0;
	}

	/* calc dram size per cs */
	mem_sz_per_cs = (unsigned long long)mem_size[iface_params->memory_size] *
			(unsigned long long)sphys /
			(unsigned long long)sphys_per_dunit;

	return mem_sz_per_cs;
}

unsigned long long mv_ddr_mem_sz_get(void)
{
	unsigned long long tot_mem_sz = 0;
	unsigned long long mem_sz_per_cs = 0;
	unsigned long long max_cs = mv_ddr_cs_num_get();

	mem_sz_per_cs = mv_ddr_mem_sz_per_cs_get();
	tot_mem_sz = max_cs * mem_sz_per_cs;

	return tot_mem_sz;
}

unsigned int mv_ddr_rtt_nom_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int rtt_nom = tm->edata.mem_edata.rtt_nom;

	if (rtt_nom >= MV_DDR_RTT_NOM_PARK_RZQ_LAST) {
		printf("error: %s: unsupported rtt_nom parameter found\n", __func__);
		rtt_nom = PARAM_UNDEFINED;
	}

	return rtt_nom;
}

unsigned int mv_ddr_rtt_park_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int cs_num = mv_ddr_cs_num_get();
	unsigned int rtt_park = MV_DDR_RTT_NOM_PARK_RZQ_LAST;

	if (cs_num > 0 && cs_num <= MAX_CS_NUM)
		rtt_park = tm->edata.mem_edata.rtt_park[cs_num - 1];

	if (rtt_park >= MV_DDR_RTT_NOM_PARK_RZQ_LAST) {
		printf("error: %s: unsupported rtt_park parameter found\n", __func__);
		rtt_park = PARAM_UNDEFINED;
	}

	return rtt_park;
}

unsigned int mv_ddr_rtt_wr_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int cs_num = mv_ddr_cs_num_get();
	unsigned int rtt_wr = MV_DDR_RTT_WR_RZQ_LAST;

	if (cs_num > 0 && cs_num <= MAX_CS_NUM)
		rtt_wr = tm->edata.mem_edata.rtt_wr[cs_num - 1];

	if (rtt_wr >= MV_DDR_RTT_WR_RZQ_LAST) {
		printf("error: %s: unsupported rtt_wr parameter found\n", __func__);
		rtt_wr = PARAM_UNDEFINED;
	}

	return rtt_wr;
}

unsigned int mv_ddr_dic_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int dic = tm->edata.mem_edata.dic;

	if (dic >= MV_DDR_DIC_RZQ_LAST) {
		printf("error: %s: unsupported dic parameter found\n", __func__);
		dic = PARAM_UNDEFINED;
	}

	return dic;
}
