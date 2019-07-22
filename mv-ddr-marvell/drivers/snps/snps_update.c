/*******************************************************************************
Copyright (C) 2017 Marvell International Ltd.

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

#include "snps.h"
#include "snps_regs.h"
#include "ddr_topology_def.h"
#include "mv_ddr_mrs.h"
#include "mv_ddr_training_db.h"
#include "mv_ddr_common.h"

/* example of an update routine that returns runtime depndant value */
#if 0 /* the routine below is just an example - not to be used */
u16 snps_get_some_runtime_value(void)
{
	u16 val = 0, result;
	u32 state, sequence_ctrl;

	debug_enter();

	/* Get current state */
	state = snps_get_state();

	/* Get next training step */
	sequence_ctrl = snps_sequence_ctrl_get();

	switch (state) {
	case TRAINING_1D:
		switch (sequence_ctrl) {
		case SEQUENCE_CTRL_1D_WR_LVL:
			val = value_for_1d_wr_lv;
			break;
		case SEQUENCE_CTRL_1D_RX_EN:
			/* check some result of previous step  (write leveling) */
			result = snps_get_result(MSG_BLK_1D_CDD_RW_1_0, 1, SEQUENCE_CTRL_1D_WR_LVL);
			if (result < threshold_for_cdd_rw_1_0)
				val = value1_for_1d_rx_en_dq;
			else
				val = value2_for_1d_rx_en_dq;
			break;
		default:
			printf("%s: Error: invalid sequence_ctrl for 1D state (0x%x)\n", __func__, sequence_ctrl);
			break;
		}
	case TRAINING_2D:
		switch (sequence_ctrl) {
		case SEQUENCE_CTRL_2D_READ_DQS:
			val = value_for_2d_read_dqs;
			break;
		case SEQUENCE_CTRL_2D_WRITE_DQ:
			val = value_for_2d_write_dq;
			break;
		default:
			printf("%s: Error: invalid sequence_ctrl for 2D state (0x%x)\n", __func__, sequence_ctrl);
			break;
		}
		break;
	}

	pr_debug("%s: updated value for register XXX is 0x%x\n", __func__, val);
	debug_exit();
	return val;
}
#endif

/* example of an update routine that returns runtime topology dependant value */
u16 sar_get_ddr_freq(void)
{
	debug_enter();
	debug_exit();
	return 0x320; /* 800Mhz */
}

/* To control the total number of debug messages, a verbosity subfield
 * (HdtCtrl, Hardware Debug Trace Control) exists in the message block.
 * Every message has a verbosity level associated with it, and as the
 * HdtCtrl value is increased, less important s messages stop being sent
 * through the mailboxes. The meanings of several major HdtCtrl thresholds are explained below:
 * 0x05 = Detailed debug messages (e.g. Eye delays)
 * 0x0A = Coarse debug messages (e.g. rank information)
 * 0xC8 = Stage completion
 * 0xC9 = Assertion messages
 * 0xFF = Firmware completion messages only
 */
u16 snps_get_hdtctrl(void)
{
	debug_enter();
	u16 hdtctrl, sequence_ctrl = snps_sequence_ctrl_get();


	/* for write leveling stage, use assertion messages */
	if (sequence_ctrl & SEQUENCE_CTRL_1D_WR_LVL)
		hdtctrl = HDT_CTRL_COARSE_DEBUG;
	else
		hdtctrl = HDT_CTRL_FW_COMPLETION;

	debug_exit();
	return hdtctrl;

	/* TODO: add support for setting hdtctrl according to log_level */
	/* TODO: Do we need a dynamic hdtctrl? (e.g verbose 1D, and more quiet 2D) */
}

/* TODO: for all functions below get freq as an api */
u16 init_phy_pllctrl2_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum mv_ddr_freq freq = tm->interface_params[0].memory_freq;

	if (freq == MV_DDR_FREQ_800)
		ret_val = PLL_FREQ_SEL_800MHZ;
	else if (freq == MV_DDR_FREQ_1200)
		ret_val = PLL_FREQ_SEL_1200MHZ;
	else if (freq == MV_DDR_FREQ_1333)
		ret_val = PLL_FREQ_SEL_1333MHZ;
	else if (freq == MV_DDR_FREQ_1466)
		ret_val = PLL_FREQ_SEL_1466MHZ;
	else if (freq == MV_DDR_FREQ_1600)
		ret_val = PLL_FREQ_SEL_1600MHZ;
	else
		printf("error: %s: unsupported frequency found\n", __func__);

	debug_exit();
	return ret_val;
}

u16 init_phy_ardptrinitval_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum mv_ddr_freq freq = tm->interface_params[0].memory_freq;

	if (freq == MV_DDR_FREQ_800)
		ret_val = ARD_PTR_INIT_800MHZ;
	else if (freq == MV_DDR_FREQ_1200)
		ret_val = ARD_PTR_INIT_1200MHZ;
	else if (freq == MV_DDR_FREQ_1333)
		ret_val = ARD_PTR_INIT_1333MHZ;
	else if (freq == MV_DDR_FREQ_1466)
		ret_val = ARD_PTR_INIT_1466MHZ;
	else if (freq == MV_DDR_FREQ_1600)
		ret_val = ARD_PTR_INIT_1600MHZ;
	else
		printf("error: %s: unsupported frequency found\n", __func__);

	debug_exit();
	return ret_val;
}

u16 init_phy_procodttimectl_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum mv_ddr_freq freq = tm->interface_params[0].memory_freq;

	if (freq == MV_DDR_FREQ_800)
		ret_val = PROCODTTIMECTL_800MHZ;
	else if (freq == MV_DDR_FREQ_1200)
		ret_val = PROCODTTIMECTL_1200MHZ;
	else if (freq == MV_DDR_FREQ_1333)
		ret_val = PROCODTTIMECTL_1333MHZ;
	else if (freq == MV_DDR_FREQ_1466)
		ret_val = PROCODTTIMECTL_1466MHZ;
	else if (freq == MV_DDR_FREQ_1600)
		ret_val = PROCODTTIMECTL_1600MHZ;
	else
		printf("error: %s: unsupported frequency found\n", __func__);

	debug_exit();
	return ret_val;
}

u16 init_phy_caluclkinfo_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum mv_ddr_freq freq = tm->interface_params[0].memory_freq;

	if (freq == MV_DDR_FREQ_800)
		ret_val = ATXDLY_800MHZ;
	else if (freq == MV_DDR_FREQ_1200)
		ret_val = ATXDLY_1200MHZ;
	else if (freq == MV_DDR_FREQ_1333)
		ret_val = ATXDLY_1333MHZ;
	else if (freq == MV_DDR_FREQ_1466)
		ret_val = ATXDLY_1466MHZ;
	else if (freq == MV_DDR_FREQ_1600)
		ret_val = ATXDLY_1600MHZ;
	else
		printf("error: %s: unsupported frequency found\n", __func__);

	debug_exit();
	return ret_val;
}

u16 init_phy_seq0bdly0_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum mv_ddr_freq freq = tm->interface_params[0].memory_freq;

	if (freq == MV_DDR_FREQ_800)
		ret_val = SEQ0BDLY0_800MHZ;
	else if (freq == MV_DDR_FREQ_1200)
		ret_val = SEQ0BDLY0_1200MHZ;
	else if (freq == MV_DDR_FREQ_1333)
		ret_val = SEQ0BDLY0_1333MHZ;
	else if (freq == MV_DDR_FREQ_1466)
		ret_val = SEQ0BDLY0_1466MHZ;
	else if (freq == MV_DDR_FREQ_1600)
		ret_val = SEQ0BDLY0_1600MHZ;
	else
		printf("error: %s: unsupported frequency found\n", __func__);

	debug_exit();
	return ret_val;
}

u16 init_phy_seq0bdly1_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum mv_ddr_freq freq = tm->interface_params[0].memory_freq;

	if (freq == MV_DDR_FREQ_800)
		ret_val = SEQ0BDLY1_800MHZ;
	else if (freq == MV_DDR_FREQ_1200)
		ret_val = SEQ0BDLY1_1200MHZ;
	else if (freq == MV_DDR_FREQ_1333)
		ret_val = SEQ0BDLY1_1333MHZ;
	else if (freq == MV_DDR_FREQ_1466)
		ret_val = SEQ0BDLY1_1466MHZ;
	else if (freq == MV_DDR_FREQ_1600)
		ret_val = SEQ0BDLY1_1600MHZ;
	else
		printf("error: %s: unsupported frequency found\n", __func__);

	debug_exit();
	return ret_val;
}

u16 init_phy_seq0bdly2_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum mv_ddr_freq freq = tm->interface_params[0].memory_freq;

	if (freq == MV_DDR_FREQ_800)
		ret_val = SEQ0BDLY2_800MHZ;
	else if (freq == MV_DDR_FREQ_1200)
		ret_val = SEQ0BDLY2_1200MHZ;
	else if (freq == MV_DDR_FREQ_1333)
		ret_val = SEQ0BDLY2_1333MHZ;
	else if (freq == MV_DDR_FREQ_1466)
		ret_val = SEQ0BDLY2_1466MHZ;
	else if (freq == MV_DDR_FREQ_1600)
		ret_val = SEQ0BDLY2_1600MHZ;
	else
		printf("error: %s: unsupported frequency found\n", __func__);

	debug_exit();
	return ret_val;
}

u16 init_odt_ctrl_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	u32 cs_num = mv_ddr_cs_num_get();

	if (cs_num == 1)
		ret_val = ODT_DRV_STREN_ONE_CS;
	else if (cs_num == 2)
		ret_val = ODT_DRV_STREN_TWO_CS;
	else
		printf("error: %s: unsupported cs number found\n", __func__);

	debug_exit();

	return ret_val;
}

static u32 mv_ddr_snps_phy_drv_odt_calc(u32 cfg)
{
	u32 val;

	switch (cfg) {
	case MV_DDR_OHM_30:
	case MV_DDR_OHM_48:
	case MV_DDR_OHM_60:
	case MV_DDR_OHM_80:
	case MV_DDR_OHM_120:
	case MV_DDR_OHM_240:
		val = cfg;
		break;
	default:
		val = PARAM_UNDEFINED;
	}

	return val;
}

static u32 mv_ddr_snps_phy_drv_data_p_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 drv_data_p = mv_ddr_snps_phy_drv_odt_calc(tm->edata.phy_edata.drv_data_p);

	if (drv_data_p == PARAM_UNDEFINED)
		printf("error: %s: unsupported drv_data_p parameter found\n", __func__);

	return drv_data_p;
}

static u32 mv_ddr_snps_phy_odt_p_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 cs_num = mv_ddr_cs_num_get();
	u32 odt_p = PARAM_UNDEFINED;

	if (cs_num > 0 && cs_num <= MAX_CS_NUM)
		odt_p = mv_ddr_snps_phy_drv_odt_calc(tm->edata.phy_edata.odt_p[cs_num - 1]);

	if (odt_p == PARAM_UNDEFINED)
		printf("error: %s: unsupported odt_p parameter found\n", __func__);

	return odt_p;
}

u16 dmem_1d_2d_dram_freq_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum mv_ddr_freq freq = tm->interface_params[0].memory_freq;

	if (freq == MV_DDR_FREQ_800)
		ret_val = DATA_RATE_1600_MT_S_800MHZ;
	else if (freq == MV_DDR_FREQ_1200)
		ret_val = DATA_RATE_1600_MT_S_1200MHZ;
	else if (freq == MV_DDR_FREQ_1333)
		ret_val = DATA_RATE_1600_MT_S_1333MHZ;
	else if (freq == MV_DDR_FREQ_1466)
		ret_val = DATA_RATE_1600_MT_S_1466MHZ;
	else if (freq == MV_DDR_FREQ_1600)
		ret_val = DATA_RATE_1600_MT_S_1600MHZ;
	else
		printf("error: %s: unsupported frequency found\n", __func__);

	debug_exit();
	return ret_val;
}

u16 dmem_1d_2d_drv_imp_phy_odt_imp_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	u32 drv_data_p, odt_p;

	drv_data_p = mv_ddr_snps_phy_drv_data_p_get();
	odt_p = mv_ddr_snps_phy_odt_p_get();

	if (drv_data_p != PARAM_UNDEFINED && odt_p != PARAM_UNDEFINED)
		ret_val = (drv_data_p << BYTE_OFFSET) | odt_p;

	debug_exit();

	return ret_val;
}

u16 dmem_1d_2d_cs_present_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	u32 cs_num = mv_ddr_cs_num_get();

	if (cs_num == 1)
		ret_val = (REG_54008_1D_2D_CS0_PRESENT << REG_54008_1D_2D_CS0_OFFS) |
			  (REG_54008_1D_2D_CS0_D0_PRESENT << REG_54008_1D_2D_CS0_D0_OFFS);
	else if (cs_num == 2)
		ret_val = (REG_54008_1D_2D_CS0_PRESENT << REG_54008_1D_2D_CS0_OFFS) |
			  (REG_54008_1D_2D_CS1_PRESENT << REG_54008_1D_2D_CS1_OFFS) |
			  (REG_54008_1D_2D_CS0_D0_PRESENT << REG_54008_1D_2D_CS0_D0_OFFS) |
			  (REG_54008_1D_2D_CS1_D0_PRESENT << REG_54008_1D_2D_CS1_D0_OFFS);
	else
		printf("error: %s: unsupported cs number found\n", __func__);

	debug_exit();

	return ret_val;
}

u16 dmem_1d_2d_addr_mirror_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);

	ret_val = iface_params->as_bus_params[0].mirror_enable_bitmask << BYTE_OFFSET;

	debug_exit();

	return ret_val;
}

u16 dmem_1d_2d_mr0_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	u32 freq, tclk, wr;
	u32 mr0_wr, mr0_cl;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);
	enum mv_ddr_speed_bin sb_idx = iface_params->speed_bin_index;

	/* get frequency in MHz */
	freq = mv_ddr_freq_get(iface_params->memory_freq);

	/* calculate clock period in ps */
	tclk = MEGA / freq;

	/* calculate write recovery */
	wr = mv_ddr_speed_bin_timing_get(sb_idx, SPEED_BIN_TWR);
	wr = time_to_nclk(wr, tclk);

	if (!mv_ddr_mr0_cl_get(iface_params->cas_l, &mr0_cl) &&
	    !mv_ddr_mr0_wr_get(wr, &mr0_wr))
		ret_val = mr0_cl | mr0_wr;

	debug_exit();
	return ret_val;
}

u16 dmem_1d_2d_mr2_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	u32 mr2_cwl, rtt_wr;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);

	rtt_wr = mv_ddr_rtt_wr_get();

	if (rtt_wr != PARAM_UNDEFINED && (!mv_ddr_mr2_cwl_get(iface_params->cas_wl, &mr2_cwl)))
		ret_val = mr2_cwl | (rtt_wr << MV_DDR_MR2_RTT_WR_OFFS);

	debug_exit();
	return ret_val;
}

u16 dmem_1d_2d_mr5_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	u32 rtt_park;

	rtt_park = mv_ddr_rtt_park_get();
	if (rtt_park != PARAM_UNDEFINED)
		ret_val = (MV_DDR_MR5_PD_ODT_IBUF_DIS << MV_DDR_MR5_PD_ODT_IBUF_OFFS) |
			(MV_DDR_MR5_DM_ENA << MV_DDR_MR5_DM_OFFS) |
			(rtt_park << MV_DDR_MR5_RTT_PARK_OFFS);

	debug_exit();
	return ret_val;
}

u16 dmem_1d_2d_mr6_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	u32 freq, tclk;
	u32 tccdl, mr6_tccdl;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);
	enum mv_ddr_speed_bin sb_idx = iface_params->speed_bin_index;

	/* get frequency in MHz */
	freq = mv_ddr_freq_get(iface_params->memory_freq);

	/* calculate clock period in ps */
	tclk = MEGA / freq;

	/* calculate tccdl */
	tccdl = mv_ddr_speed_bin_timing_get(sb_idx, SPEED_BIN_TCCDL);
	tccdl = GET_MAX_VALUE(tclk * 5, tccdl);
	tccdl = time_to_nclk(tccdl, tclk);

	if (!mv_ddr_mr6_tccdl_get(tccdl, &mr6_tccdl))
		ret_val = mr6_tccdl |
			  MV_DDR_MR6_VREFDQ_TRNG_VAL << MV_DDR_MR6_VREFDQ_TRNG_VAL_OFFS |
			  MV_DDR_MR6_VREFDQ_TRNG_RNG1 << MV_DDR_MR6_VREFDQ_TRNG_RNG_OFFS |
			  MV_DDR_MR6_VREFDQ_TRNG_DIS << MV_DDR_MR6_VREFDQ_TRNG_ENA_OFFS;

	debug_exit();
	return ret_val;
}

u16 dmem_1d_2d_rtt_nom_wr_park_get(void)
{
	debug_enter();

	u16 ret_val = 0;
	u16 rtt_park, rtt_nom, rtt_wr;

	rtt_nom = mv_ddr_rtt_nom_get();
	rtt_park = mv_ddr_rtt_park_get();
	rtt_wr = mv_ddr_rtt_wr_get();
	if (rtt_nom != PARAM_UNDEFINED && rtt_park != PARAM_UNDEFINED && rtt_wr != PARAM_UNDEFINED) {
		ret_val = (rtt_nom & RTT_NOM_WR_PARK0_RTT_NOM_MR1_MASK) << RTT_NOM_WR_PARK0_RTT_NOM_MR1_OFFS;
		ret_val |= (rtt_park & RTT_NOM_WR_PARK0_RTT_PARK_MR5_MASK) << RTT_NOM_WR_PARK0_RTT_PARK_MR5_OFFS;
		ret_val |= (rtt_wr & RTT_NOM_WR_PARK0_RTT_WR_MR2_MASK) << RTT_NOM_WR_PARK0_RTT_WR_MR2_OFFS;
		ret_val |= RTT_NOM_WR_PARK0_EN_VAL;
	}

	debug_exit();

	return ret_val;
}

u16 dmem_1d_2d_en_dq_dis_dbyte_get(void)
{
	u16 ret_val = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);
	u8 tot_dq_bits_en;
	u8 dis_dbyte = 0;
	debug_enter();

	tot_dq_bits_en = mv_ddr_if_bus_width_get();
	if (tot_dq_bits_en == 32)
		dis_dbyte = NC_DBYTES_32_BITS;
	else if (tot_dq_bits_en == 64) /* 64 bits */
		dis_dbyte = NC_DBYTES_64_BITS;
	else
		return -1;

	/* get ddr device width in case ecc en */
	if (mv_ddr_is_ecc_ena())
		tot_dq_bits_en +=
			(iface_params->bus_width == MV_DDR_DEV_WIDTH_8BIT) ? 8 : 16;

	ret_val = (tot_dq_bits_en & TOTAL_DQ_BITS_EN_MASK) << TOTAL_DQ_BITS_EN_OFFS;
	ret_val |= (dis_dbyte & NC_DBYTES_MASK) << NC_DBYTES_OFFS;

	debug_exit();

	return ret_val;
}

u16 dmem_1d_2d_gear_down_x16_present_get(void)
{
	u16 ret_val = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);
	u32 cs_num = mv_ddr_cs_num_get();
	u8 x16_present_val;
	debug_enter();

	x16_present_val = (iface_params->bus_width == MV_DDR_DEV_WIDTH_8BIT) ?
			   X16_NOT_PRESENT_VAL :
#ifdef APN807
/*
 * APN807 workaround for x16 devices
 * In PDA mode, VREF is set per PHY.
 * There are two PHYs per x16 device with possible different VREF values.
 * The workaround overrides x16 to x8 device configuration to ensure
 * the first connected PHY's VREF value is taken.
 */
			   X16_NOT_PRESENT_VAL;
#else
			   X16_PRESENT_VAL;
#endif

	if (cs_num == 1) /* single cs */
		ret_val = (x16_present_val & X16_PRESENT_CS0_MASK) << X16_PRESENT_CS0_OFFS;
	else /* dual cs */
		ret_val = ((x16_present_val & X16_PRESENT_CS0_MASK) << X16_PRESENT_CS0_OFFS) |
			   ((x16_present_val & X16_PRESENT_CS1_MASK) << X16_PRESENT_CS1_OFFS);
	ret_val |= ((ZERO_VAL & X16_PRESENT_RSRV_MASK) << X16_PRESENT_RSRV_OFFS);
	ret_val |= (CS_SETUP_GDDEC_REG_VAL << BYTE_OFFSET);

	debug_exit();

	return ret_val;
}
