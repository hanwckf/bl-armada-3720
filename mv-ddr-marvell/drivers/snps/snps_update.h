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

#ifndef _SNPS_UPDATE_H_
#define _SNPS_UPDATE_H_
#include "snps_regs.h"

u16 sar_get_ddr_freq(void);
u16 snps_get_hdtctrl(void);
/* per platform function providing SNPS driver with non-default
 * configuration of address lines' PHY to IO mapping
 */
u32 *snps_ext_swizzle_cfg_get(void);
u16 init_phy_pllctrl2_get(void);
u16 init_phy_ardptrinitval_get(void);
u16 init_phy_procodttimectl_get(void);
u16 init_phy_caluclkinfo_get(void);
u16 init_phy_seq0bdly0_get(void);
u16 init_phy_seq0bdly1_get(void);
u16 init_phy_seq0bdly2_get(void);
u16 init_odt_ctrl_get(void);
u16 dmem_1d_2d_dram_freq_get(void);
u16 dmem_1d_2d_drv_imp_phy_odt_imp_get(void);
u16 dmem_1d_2d_cs_present_get(void);
u16 dmem_1d_2d_addr_mirror_get(void);
u16 dmem_1d_2d_mr0_get(void);	/* TODO get the mrs from data base */
u16 dmem_1d_2d_mr2_get(void);	/* TODO get the mrs from data base */
u16 dmem_1d_2d_mr5_get(void);	/* TODO get the mrs from data base */
u16 dmem_1d_2d_mr6_get(void);	/* TODO get the mrs from data base */
u16 dmem_1d_2d_rtt_nom_wr_park_get(void);
u16 dmem_1d_2d_en_dq_dis_dbyte_get(void);
u16 dmem_1d_2d_gear_down_x16_present_get(void);

struct snps_address_data init_phy_static_update[] = {
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_MASTER, INST_NUM_0, REG_88_PHY_CAL_RATE),
	 CAL_RATE_VAL},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_MASTER, INST_NUM_0, REG_B2_VREF_IN_GLOBAL),
	 VREF_IN_GLOBAL_VAL},
	{REG_20019_TRISTATE_MODE_CA_TG0, REG_20019_TRISTATE_MODE_CA_VAL},
	{ -1	, -1  }
};

/*init phy dynamic update*/
struct snps_address_dynamic_update init_phy_dynamic_update[] = {
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_0, REG_X4D_PHY_TX_ODT_DRV_STREN(LOW_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_0, REG_X4D_PHY_TX_ODT_DRV_STREN(UP_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_1, REG_X4D_PHY_TX_ODT_DRV_STREN(LOW_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_1, REG_X4D_PHY_TX_ODT_DRV_STREN(UP_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_2, REG_X4D_PHY_TX_ODT_DRV_STREN(LOW_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_2, REG_X4D_PHY_TX_ODT_DRV_STREN(UP_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_3, REG_X4D_PHY_TX_ODT_DRV_STREN(LOW_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_3, REG_X4D_PHY_TX_ODT_DRV_STREN(UP_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_4, REG_X4D_PHY_TX_ODT_DRV_STREN(LOW_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_4, REG_X4D_PHY_TX_ODT_DRV_STREN(UP_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_5, REG_X4D_PHY_TX_ODT_DRV_STREN(LOW_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_5, REG_X4D_PHY_TX_ODT_DRV_STREN(UP_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_6, REG_X4D_PHY_TX_ODT_DRV_STREN(LOW_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_6, REG_X4D_PHY_TX_ODT_DRV_STREN(UP_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_7, REG_X4D_PHY_TX_ODT_DRV_STREN(LOW_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_7, REG_X4D_PHY_TX_ODT_DRV_STREN(UP_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_8, REG_X4D_PHY_TX_ODT_DRV_STREN(LOW_NIBL)),
	 init_odt_ctrl_get},
	{PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, INST_NUM_8, REG_X4D_PHY_TX_ODT_DRV_STREN(UP_NIBL)),
	 init_odt_ctrl_get},
	{REG_200C5_PHY_PLLCTRL2, init_phy_pllctrl2_get},
	{REG_2002E_PHY_ARDPTRINITVAL, init_phy_ardptrinitval_get},
	{REG_20056_PHY_PROCODTTIMECTL, init_phy_procodttimectl_get},
	{REG_20008_PHY_CALUCLKINFO, init_phy_caluclkinfo_get},
	{-1, NULL}
};

/* --------------------------- */
/* 1D static & dynamic updates */
/* --------------------------- */

/* 1D IMEM static update*/
struct snps_address_data one_d_imem_static_update[] = {
	{ -1	, -1  }
};

/* 1D DMEM static update*/
struct snps_address_data one_d_dmem_static_update[] = {
	{REG_54000_1D_2D_MSGMISC, (SIMULATION_DIS << BYTE_OFFSET) |
				   (TRAIN_VREFDAC0_DURING_1D_EN | ENABLE_HIGH_EFFORT_WRDQ1D_EN)},
	{REG_54006_1D_2D_DRAM_TYPE_PHY_VREF, (DRAM_TYPE_REG_VAL << BYTE_OFFSET) | PHY_VREF_REG_VAL},
	{REG_5400A_1D_2D_PHY_CFG_CSTEST_FAIL, PHY_CFG_REG_VAL << BYTE_OFFSET},
	{REG_5400C_1D_2D_RESV19_HDT_CTRL, HDT_CTRL_REG_VAL},
	{ -1	, -1 }
};

/* 1D DMEM dynamic update*/
struct snps_address_dynamic_update one_d_dmem_dynamic_update[] = {
	{REG_54003_1D_2D_DRAM_FREQ, dmem_1d_2d_dram_freq_get},
	{REG_54005_1D_2D_DRV_IMP_PHY_ODT_IMP, dmem_1d_2d_drv_imp_phy_odt_imp_get},
	{REG_54007_1D_2D_EN_DQ_DIS_DBYTE, dmem_1d_2d_en_dq_dis_dbyte_get},
	{REG_54008_1D_2D_CS_PRESENT, dmem_1d_2d_cs_present_get},
	{REG_54009_1D_2D_ADDR_MIRROR, dmem_1d_2d_addr_mirror_get},
	{REG_5402F_1D_2D_MR0, dmem_1d_2d_mr0_get},
	{REG_54031_1D_2D_MR2, dmem_1d_2d_mr2_get},
	{REG_54034_1D_2D_MR5, dmem_1d_2d_mr5_get},
	{REG_54035_1D_2D_MR6, dmem_1d_2d_mr6_get},
	{REG_54036_1D_2D_CS_SETUP_GDDEC_X16_PRESENT, dmem_1d_2d_gear_down_x16_present_get},
	{REG_54037_1D_2D_RTT_NOM_WR_PARK0, dmem_1d_2d_rtt_nom_wr_park_get},
	{REG_54038_1D_2D_RTT_NOM_WR_PARK1, dmem_1d_2d_rtt_nom_wr_park_get},
	{-1, NULL}
};


/* --------------------------- */
/* 2D static & dynamic updates */
/* --------------------------- */

/* 2D IMEM static update*/
struct snps_address_data two_d_imem_static_update[] = {
/*	{0x3333, 0xb},	*/
	{ -1	, -1  }
};

/* 2D DMEM static update*/
struct snps_address_data two_d_dmem_static_update[] = {
	{REG_54000_1D_2D_MSGMISC, (SIMULATION_DIS << BYTE_OFFSET) |
				   (TRAIN_VREFDAC0_DURING_1D_EN | ENABLE_HIGH_EFFORT_WRDQ1D_EN)},
	{REG_54006_1D_2D_DRAM_TYPE_PHY_VREF, (DRAM_TYPE_REG_VAL << BYTE_OFFSET) | PHY_VREF_REG_VAL},
	{REG_5400A_1D_2D_PHY_CFG_CSTEST_FAIL, PHY_CFG_REG_VAL << BYTE_OFFSET},
	{REG_5400C_1D_2D_RESV19_HDT_CTRL, HDT_CTRL_REG_VAL | RX2D_TRAIN_OPT_REG_VAL},
	{REG_5400D_1D_2D_RSRV1B_RSRV1A, (RSRV1A_VAL << BYTE_OFFSET) | RSRV1B_VAL},
	{ -1	, -1 }
};

/* 2D DMEM dynamic update*/
struct snps_address_dynamic_update two_d_dmem_dynamic_update[] = {
	{REG_54003_1D_2D_DRAM_FREQ, dmem_1d_2d_dram_freq_get},
	{REG_54005_1D_2D_DRV_IMP_PHY_ODT_IMP, dmem_1d_2d_drv_imp_phy_odt_imp_get},
	{REG_54007_1D_2D_EN_DQ_DIS_DBYTE, dmem_1d_2d_en_dq_dis_dbyte_get},
	{REG_54008_1D_2D_CS_PRESENT, dmem_1d_2d_cs_present_get},
	{REG_54009_1D_2D_ADDR_MIRROR, dmem_1d_2d_addr_mirror_get},
	{REG_5402F_1D_2D_MR0, dmem_1d_2d_mr0_get},
	{REG_54031_1D_2D_MR2, dmem_1d_2d_mr2_get},
	{REG_54034_1D_2D_MR5, dmem_1d_2d_mr5_get},
	{REG_54035_1D_2D_MR6, dmem_1d_2d_mr6_get},
	{REG_54036_1D_2D_CS_SETUP_GDDEC_X16_PRESENT, dmem_1d_2d_gear_down_x16_present_get},
	{REG_54037_1D_2D_RTT_NOM_WR_PARK0, dmem_1d_2d_rtt_nom_wr_park_get},
	{REG_54038_1D_2D_RTT_NOM_WR_PARK1, dmem_1d_2d_rtt_nom_wr_park_get},
	{ -1	, NULL }
};


/* --------------------------- */
/* pie static & dynamic updates */
/* --------------------------- */
/* 2D DMEM dynamic update*/
struct snps_address_dynamic_update pie_dynamic_update[] = {
	{REG_2000B_PHY_SEQ0BDLY0, init_phy_seq0bdly0_get},
	{REG_2000C_PHY_SEQ0BDLY1, init_phy_seq0bdly1_get},
	{REG_2000D_PHY_SEQ0BDLY2, init_phy_seq0bdly2_get},
	{ -1	, NULL }
};

#endif	/* _SNPS_UPDATE_H_ */
