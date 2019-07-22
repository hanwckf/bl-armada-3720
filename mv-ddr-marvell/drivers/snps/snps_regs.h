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

#ifndef _SNPS_REGS_H_
#define _SNPS_REGS_H_

#define NO_MASK			0xfffe
#define BYTE_MASK		0xff
#define BYTE_OFFSET		8

/* ------- Validation Test registers ------- */
#define DBYTE_CS_OFFSET		20
#define DBYTE_INSTANCE_OFFSET	12
#define DBYTE_BIT_OFFSET	8

#define REG_100C0_TX_DLY_TG0_BASE	0x100C0
#define REG_100C1_TX_DLY_TG1_BASE	0x100C1
#define	TX_FINE_DELAY_OFFS		0
#define	TX_FINE_DELAY_MASK		0x1f
#define TX_COARSE_DELAY_OFFS		6
#define	TX_COARSE_DELAY_MASK		0x7

#define REG_20019_TRISTATE_MODE_CA_TG0	0x20019
#define REG_20019_TRISTATE_MODE_CA_VAL	0x5

#define REG_54043_VREF_DQ_R0_BASE	0x54043
#define REG_5404D_VREF_DQ_R1_BASE	0x5404d
#define MR6_VREF_OFFS			0
#define MR6_VREF_MASK			0x3f
#define MR6_VREF_RANGE_OFFS		6
#define MR6_VREF_RANGE_MASK		0x1

#define REG_1008C_RX_CLK_DLY_TG0_NIBBLE0_BASE	0x1008C
#define REG_1018C_RX_CLK_DLY_TG0_NIBBLE1_BASE	0x1018C

#define REG_1008D_RX_CLK_DLY_TG1_NIBBLE0_BASE	0x1008d
#define REG_1018D_RX_CLK_DLY_TG1_NIBBLE1_BASE	0x1018d

#define REG_10040_VREF_DAC0_BASE		0x10040
#define REG_C0080_UCCLK_HCLK_EN			0xC0080
#define UCCLK_EN_OFFS	0
#define UCCLK_EN_MASK	0x1
#define HCLK_EN_OFFS	1
#define HCLK_EN_MASK	0x1
#define CLK_EN		0x1

/* ------- Message Block registers ------- */
#define REG_54001_PMU_REV_ADDR		0x54001
#define REG_5400A_CS_TEST_FAIL_ADDR	0x5400a

/* synopsis registers */
/* phy registers */
#define P_STATE_OFFS	20
#define P_STATE_MASK	0x7
#define P_STATE_0	0x0
#define P_STATE_1	0x1
#define P_STATE_2	0x2
#define P_STATE_3	0x3
#define P_STATE_BRCAST	0x7

#define BLK_TYPE_OFFS		16
#define BLK_TYPE_MASK		0xf
#define BLK_TYPE_ANIB		0x0
#define BLK_TYPE_DBYTE		0x1
#define BLK_TYPE_MASTER		0x2
#define BLK_TYPE_ACSM		0x4
#define BLK_TYPE_UCTL_MEM	0x5
#define BLK_TYPE_PPGC		0x7
#define BLK_TYPE_INITENG	0x9
#define BLK_TYPE_PUB		0xc
#define BLK_TYPE_APBONLY	0xd

#define INST_NUM_OFFS	12
#define INST_NUM_MASK	0xf
#define INST_NUM_0	0x0
#define INST_NUM_1	0x1
#define INST_NUM_2	0x2
#define INST_NUM_3	0x3
#define INST_NUM_4	0x4
#define INST_NUM_5	0x5
#define INST_NUM_6	0x6
#define INST_NUM_7	0x7
#define INST_NUM_8	0x8

#define ACCESS_REG_OFFS	0
#define ACCESS_REG_MASK	0xfff

#define NIBL_OFFS	8
#define NIBL_MASK	0x1
#define LOW_NIBL	0x0
#define UP_NIBL		0x1

/*
 * The 23 bit register address is specified by the concatenation of the following fields: {PState,
 * Block Type, Instance Number, Register}.
 * note: relevant only for phy init state
 */
#define PHY_REG_ADDR_MAP(pstate, block_type, instance, reg) (((pstate & P_STATE_MASK) << P_STATE_OFFS) | \
							((block_type & BLK_TYPE_MASK) << BLK_TYPE_OFFS) | \
							((instance & INST_NUM_MASK) << INST_NUM_OFFS) | \
							((reg & ACCESS_REG_MASK) << ACCESS_REG_OFFS))

#define REG_X4D_PHY_TX_ODT_DRV_STREN(nibble) ((nibble & NIBL_MASK) << NIBL_OFFS | 0x4d)
#define ODT_STREN_P_OFFS	0
#define ODT_STREN_P_MASK	0x3f
#define ODT_STREN_N_OFFS	6
#define ODT_STREN_N_MASK	0x3f
#define HIGH_IMPEDANCE	0x0
#define OHMS_480	0x1
#define OHMS_240	0x2
#define OHMS_160	0x3
#define OHMS_120	0x8
#define OHMS_96		0x9
#define OHMS_80		0xa
#define OHMS_68_6	0xb
#define OHMS_60		0x18
#define OHMS_53_3	0x19
#define OHMS_48		0x1a
#define OHMS_43_6	0x1b
#define OHMS_40		0x38
#define OHMS_36_9	0x39
#define OHMS_34_3	0x3a
#define OHMS_32		0x3b
#define OHMS_30		0x3e
#define OHMS_28_2	0x3f
#define ODT_DRV_STREN_ONE_CS	(((OHMS_60 & ODT_STREN_P_MASK) << ODT_STREN_P_OFFS) | \
				 ((HIGH_IMPEDANCE & ODT_STREN_N_MASK) << ODT_STREN_N_OFFS))
#define ODT_DRV_STREN_TWO_CS	(((OHMS_120 & ODT_STREN_P_MASK) << ODT_STREN_P_OFFS) | \
				 ((HIGH_IMPEDANCE & ODT_STREN_N_MASK) << ODT_STREN_N_OFFS))

#define REG_120_HWT_SWIZZLE_HWT_ADDR0_BASE	0x120
#define HWT_SWIZZLE_HWT_REGS_NUM		27

#define LANE_OFFS		8
#define LANE_MASK		0xf
#define REG_68_RX_PB_DLY(rank, lane)	(((rank) + 0x68) | (((lane) & LANE_MASK) << LANE_OFFS))
#define RX_PB_DLY_OFFS		0
#define RX_PB_DLY_MASK		0x7f

#define REG_88_PHY_CAL_RATE		0x88
#define CAL_INTERVAL_OFFS		0
#define CAL_INTERVAL_MASK		0xf
#define INVAL_CONT		0x0
#define INVAL_0_013_MSEC	0x1
#define INVAL_0_10_MSEC		0x2
#define INVAL_1_MSEC		0x3
#define INVAL_2_MSEC		0x4
#define INVAL_3_MSEC		0x5
#define INVAL_4_MSEC		0x6
#define INVAL_8_MSEC		0x7
#define INVAL_10_MSEC		0x8
#define INVAL_20_MSEC		0x9
#define CAL_RUN_OFFS			4
#define CAL_RUN_MASK			0x1
#define CAL_RUN_INACT		0x0
#define CAL_RUN_0_TO_1		0x1
#define CAL_ONCE_OFFS			5
#define CAL_ONCE_MASK			0x1
#define CAL_ONCE_INTVAL		0x0
#define CAL_ONCE_0_TO_1		0x1
#define DIS_BACKGND_ZQ_UPDT_OFFS	6
#define DIS_BACKGND_ZQ_UPDT_MASK	0x1
#define EN_BACKGND_ZQ_UPDT_VAL	0x0
#define DIS_BACKGND_ZQ_UPDT_VAL	0x1
#define CAL_RATE_VAL	(((INVAL_20_MSEC & CAL_INTERVAL_MASK) << CAL_INTERVAL_OFFS) | \
			 ((CAL_RUN_0_TO_1 & CAL_RUN_MASK) << CAL_RUN_OFFS))

#define REG_8C_RX_CLK_DLY(rank, nibble)	(((rank) + 0x8c) | (((nibble) & NIBL_MASK) << NIBL_OFFS))
#define RX_CLK_DLY_OFFS		0
#define RX_CLK_DLY_MASK		0x3f

#define REG_B2_VREF_IN_GLOBAL		0xb2
#define GLOB_VREF_IN_SEL_OFFS		0
#define GLOB_VREF_IN_SEL_MASK		0x3
#define BP_VREF_HI_Z		0x0
#define BP_VREF_PLL_ANA_BUS	0x2
#define BP_VREF_PHY_VREF_DAC	0x3
#define GLOB_VREF_IN_DAC_OFFS		3
#define GLOB_VREF_IN_DAC_MASK		0x7f
#define DAC_CODE_VAL		0x4c /* TODO get the code from calculation of electrical parameters */
#define VREF_IN_GLOBAL_VAL	(((BP_VREF_HI_Z & GLOB_VREF_IN_SEL_MASK) << GLOB_VREF_IN_SEL_OFFS) | \
				 ((DAC_CODE_VAL & GLOB_VREF_IN_DAC_MASK) << GLOB_VREF_IN_DAC_OFFS))

#define REG_200C5_PHY_PLLCTRL2		0x200c5
#define PLL_FREQ_SEL_800MHZ		0xb
#define PLL_FREQ_SEL_1200MHZ		0xa
#define PLL_FREQ_SEL_1333MHZ		0x19
#define PLL_FREQ_SEL_1466MHZ		0x19
#define PLL_FREQ_SEL_1600MHZ		0x19

#define REG_2002E_PHY_ARDPTRINITVAL	0x2002e
#define ARD_PTR_INIT_800MHZ		0x1
#define ARD_PTR_INIT_1200MHZ		0x2
#define ARD_PTR_INIT_1333MHZ		0x2
#define ARD_PTR_INIT_1466MHZ		0x2
#define ARD_PTR_INIT_1600MHZ		0x2

#define REG_20056_PHY_PROCODTTIMECTL	0x20056
#define PROCODTTIMECTL_800MHZ		0xa
#define PROCODTTIMECTL_1200MHZ		0x2
#define PROCODTTIMECTL_1333MHZ		0x3
#define PROCODTTIMECTL_1466MHZ		0x3
#define PROCODTTIMECTL_1600MHZ		0x3

#define REG_20008_PHY_CALUCLKINFO	0x20008
#define ATXDLY_800MHZ			0x190
#define ATXDLY_1200MHZ			0x258
#define ATXDLY_1333MHZ			0x29a
#define ATXDLY_1466MHZ			0x2dd
#define ATXDLY_1600MHZ			0x320

#define REG_2000B_PHY_SEQ0BDLY0		0x2000b
#define SEQ0BDLY0_800MHZ		0x32
#define SEQ0BDLY0_1200MHZ		0x4b
#define SEQ0BDLY0_1333MHZ		0x53
#define SEQ0BDLY0_1466MHZ		0x5b
#define SEQ0BDLY0_1600MHZ		0x64

#define REG_2000C_PHY_SEQ0BDLY1		0x2000c
#define SEQ0BDLY1_800MHZ		0x64
#define SEQ0BDLY1_1200MHZ		0x96
#define SEQ0BDLY1_1333MHZ		0xa6
#define SEQ0BDLY1_1466MHZ		0xb7
#define SEQ0BDLY1_1600MHZ		0xc8

#define REG_2000D_PHY_SEQ0BDLY2		0x2000d
#define SEQ0BDLY2_800MHZ		0x3e8
#define SEQ0BDLY2_1200MHZ		0x5dc
#define SEQ0BDLY2_1333MHZ		0x681
#define SEQ0BDLY2_1466MHZ		0x728
#define SEQ0BDLY2_1600MHZ		0x7d0

#define REG_54000_1D_2D_MSGMISC	0x54000
#define TRAIN_VREFDAC0_DURING_1D_EN	0x20
#define TRAIN_VREFDAC0_DURING_1D_DIS	0x0
#define ENABLE_HIGH_EFFORT_WRDQ1D_EN	0x40
#define ENABLE_HIGH_EFFORT_WRDQ1D_DIS	0x0
#define SIMULATION_DIS			0x0

/* 1d 2d shared registers */
#define REG_54003_1D_2D_DRAM_FREQ	0x54003
#define DATA_RATE_1600_MT_S_800MHZ	0x640
#define DATA_RATE_1600_MT_S_1200MHZ	0x960
#define DATA_RATE_1600_MT_S_1333MHZ	0xa6a
#define DATA_RATE_1600_MT_S_1466MHZ	0xb75
#define DATA_RATE_1600_MT_S_1600MHZ	0xc80

#define REG_54005_1D_2D_DRV_IMP_PHY_ODT_IMP	0x54005

#define REG_54007_1D_2D_EN_DQ_DIS_DBYTE	0x54007
#define NC_DBYTES_MASK			0xff
#define NC_DBYTES_OFFS			0x0
#define NC_DBYTES_32_BITS		0xf0
#define NC_DBYTES_64_BITS		0x0
#define TOTAL_DQ_BITS_EN_OFFS		8
#define TOTAL_DQ_BITS_EN_MASK		0xff

#define REG_54008_1D_2D_CS_PRESENT	0x54008
#define REG_54008_1D_2D_CS0_OFFS	0
#define REG_54008_1D_2D_CS0_MASK	0x1
#define REG_54008_1D_2D_CS0_PRESENT	1
#define REG_54008_1D_2D_CS1_OFFS	1
#define REG_54008_1D_2D_CS1_MASK	0x1
#define REG_54008_1D_2D_CS1_PRESENT	1
#define REG_54008_1D_2D_CS0_D0_OFFS	8
#define REG_54008_1D_2D_CS0_D0_MASK	0x1
#define REG_54008_1D_2D_CS0_D0_PRESENT	1
#define REG_54008_1D_2D_CS1_D0_OFFS	9
#define REG_54008_1D_2D_CS1_D0_MASK	0x1
#define REG_54008_1D_2D_CS1_D0_PRESENT	1

#define REG_54009_1D_2D_ADDR_MIRROR	0x54009

#define REG_5402F_1D_2D_MR0		0x5402f

#define REG_54031_1D_2D_MR2		0x54031

#define REG_54035_1D_2D_MR6		0x54035

#define REG_54006_1D_2D_DRAM_TYPE_PHY_VREF	0x54006 /* TODO -set in dynamic as electrical parameters */
#define MODULE_TYPE_OFFS	0
#define MODULE_TYPE_MASK	0xff
#define DDR4_UNBUFFERED_VAL	0x02
#define VDDQ_PRECENTAGE_OFFS	0
#define MODULE_TYPE_MASK	0xff
#define PHY_VREF_VAL		0x54
#define DRAM_TYPE_REG_VAL	((DDR4_UNBUFFERED_VAL & MODULE_TYPE_MASK) << MODULE_TYPE_OFFS)
#define PHY_VREF_REG_VAL	((PHY_VREF_VAL & MODULE_TYPE_MASK) << VDDQ_PRECENTAGE_OFFS)

#define REG_5400A_1D_2D_PHY_CFG_CSTEST_FAIL	0x5400a /* take from topology */
#define SLOW_ACCESS_MODE_OFFS	0
#define SLOW_ACCESS_MODE_MASK	0xff
#define T_1_VAL			0x0
#define T_2_VAL			0x1
#define PHY_CFG_REG_VAL		((T_2_VAL & SLOW_ACCESS_MODE_MASK) << SLOW_ACCESS_MODE_OFFS)

#define REG_5400C_1D_2D_RESV19_HDT_CTRL		0x5400c /* TODO: set hdt ctrl according to atf log level */
#define VERBOSITY_LEVEL_OFFS	0
#define VERBOSITY_LEVEL_MASK	0xff
#define DETAILED_MSG_VAL			0x05
#define COARSE_MSG_VAL				0x0A
#define STAGE_COMPLETION_MSG_VAL		0xC8
#define ASSERTION_MSG_VAL			0xC9
#define FIRMWARE_COMPLETION_ONLY_MSG_VAL	0xFF
#define HDT_CTRL_REG_VAL	((STAGE_COMPLETION_MSG_VAL & VERBOSITY_LEVEL_MASK) << VERBOSITY_LEVEL_OFFS)
/* decision feedback equaliazation */
#define RX2D_DFE_OFFS			8
#define RX2D_DFE_MASK			0x1
#define RX2D_DFE_ENA			1
#define RX2D_DFE_DIS			0
/* voltage step size  (2^n)*/
#define RX2D_V_STEP_SIZE_OFFS		9
#define RX2D_V_STEP_SIZE_MASK		0x3
#define RX2D_V_STEP_SIZE_1DAC		0
#define RX2D_V_STEP_SIZE_2DAC		1
#define RX2D_V_STEP_SIZE_4DAC		2
#define RX2D_V_STEP_SIZE_8DAC		3
/* delay step size (2^n) */
#define RX2D_DLY_STEP_SIZE_OFFS		11
#define RX2D_DLY_STEP_SIZE_MASK		0x3
#define RX2D_DLY_STEP_SIZE_1LCDL	0
#define RX2D_DLY_STEP_SIZE_2LCDL	1
#define RX2D_DLY_STEP_SIZE_4LCDL	2
#define RX2D_DLY_STEP_SIZE_8LCDL	3
#define RX2D_TRAIN_OPT_REG_VAL	(((RX2D_DFE_ENA & RX2D_DFE_MASK) << RX2D_DFE_OFFS) | \
				 ((RX2D_V_STEP_SIZE_1DAC & RX2D_V_STEP_SIZE_MASK) << RX2D_V_STEP_SIZE_OFFS) | \
				 ((RX2D_DLY_STEP_SIZE_1LCDL & RX2D_DLY_STEP_SIZE_MASK) << RX2D_DLY_STEP_SIZE_OFFS))

#define REG_5400D_1D_2D_RSRV1B_RSRV1A		0x5400d
#define RSRV1B_VAL	0x0
#define RSRV1A_VAL	0x1

#define REG_54034_1D_2D_MR5			0x54034 /* TODO: check if to take from electrical */

#define REG_54036_1D_2D_CS_SETUP_GDDEC_X16_PRESENT	0x54036 /* TODO: take from topology */
/* X16_device_map_offs, corresponds to CS[3:0] */
#define X16_PRESENT_CS0_OFFS	0
#define X16_PRESENT_CS0_MASK	0x1
#define X16_PRESENT_CS1_OFFS	1
#define X16_PRESENT_CS1_MASK	0x1
#define X16_PRESENT_CS2_OFFS	2
#define X16_PRESENT_CS2_MASK	0x1
#define X16_PRESENT_CS3_OFFS	3
#define X16_PRESENT_CS3_MASK	0x1
#define X16_PRESENT_RSRV_OFFS	4
#define X16_PRESENT_RSRV_MASK	0xf
#define X16_NOT_PRESENT_VAL	0
#define X16_PRESENT_VAL		1
#define ZERO_VAL		0x0
#define DDR4_GEAR_DOWN_CS_TIMING_CTRL_OFFS	0
#define DDR4_GEAR_DOWN_CS_TIMING_CTRL_MASK	0xff
#define SAME_DELAY_VAL	0x0
#define ADD_1UI_VAL	0x1
#define CS_SETUP_GDDEC_REG_VAL	((ADD_1UI_VAL & DDR4_GEAR_DOWN_CS_TIMING_CTRL_MASK) << \
				 DDR4_GEAR_DOWN_CS_TIMING_CTRL_OFFS)

/* TODO: remove the below registers configuration to electrical */
/* Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 0 DRAM */
#define REG_54037_1D_2D_RTT_NOM_WR_PARK0	0x54037
#define RTT_NOM_WR_PARK0_EN_OFFS	0
#define RTT_NOM_WR_PARK0_EN_MASK	0x1
#define RTT_NOM_WR_PARK0_EN_VAL		0x1
#define RTT_NOM_WR_PARK0_RTT_NOM_MR1_OFFS	3
#define RTT_NOM_WR_PARK0_RTT_NOM_MR1_MASK	0x7
#define RTT_NOM_WR_PARK0_RTT_WR_MR2_OFFS	9
#define RTT_NOM_WR_PARK0_RTT_WR_MR2_MASK	0x7
#define RTT_NOM_WR_PARK0_RTT_PARK_MR5_OFFS	6
#define RTT_NOM_WR_PARK0_RTT_PARK_MR5_MASK	0x7
#define RTT_NOM_WR_PARK0_RTT_PARK_MR5_VAL	0x1
#define RTT_NOM_WR_PARK0_REG_VAL	(((RTT_NOM_WR_PARK0_RTT_PARK_MR5_VAL & RTT_NOM_WR_PARK0_RTT_PARK_MR5_MASK) << \
					  RTT_NOM_WR_PARK0_RTT_PARK_MR5_OFFS) | \
					 ((RTT_NOM_WR_PARK0_EN_VAL & RTT_NOM_WR_PARK0_EN_MASK) << \
					  RTT_NOM_WR_PARK0_EN_OFFS))

/* Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 1 DRAM */
#define REG_54038_1D_2D_RTT_NOM_WR_PARK1	0x54038
#define RTT_NOM_WR_PARK1_EN_OFFS	0
#define RTT_NOM_WR_PARK1_EN_MASK	0x1
#define RTT_NOM_WR_PARK1_EN_VAL		0x1
#define RTT_NOM_WR_PARK1_RTT_NOM_MR1_OFFS	3
#define RTT_NOM_WR_PARK1_RTT_NOM_MR1_MASK	0x7
#define RTT_NOM_WR_PARK1_RTT_NOM_MR1_VAL	0x8
#define RTT_NOM_WR_PARK1_RTT_WR_MR2_OFFS	9
#define RTT_NOM_WR_PARK1_RTT_WR_MR2_MASK	0x7
#define RTT_NOM_WR_PARK1_RTT_PARK_MR5_OFFS	6
#define RTT_NOM_WR_PARK1_RTT_PARK_MR5_MASK	0x7
#define RTT_NOM_WR_PARK1_RTT_PARK_MR5_VAL	0x1
#define RTT_NOM_WR_PARK1_REG_VAL	(((RTT_NOM_WR_PARK1_RTT_PARK_MR5_VAL & RTT_NOM_WR_PARK1_RTT_PARK_MR5_MASK) << \
					  RTT_NOM_WR_PARK1_RTT_PARK_MR5_OFFS) | \
					 ((RTT_NOM_WR_PARK1_EN_VAL & RTT_NOM_WR_PARK1_EN_MASK) << \
					  RTT_NOM_WR_PARK1_EN_OFFS))

/* Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 2 DRAM */
#define REG_54039_1D_2D_RTT_NOM_WR_PARK2	0x54039
#define RTT_NOM_WR_PARK2_EN_OFFS	0
#define RTT_NOM_WR_PARK2_EN_MASK	0x1
#define RTT_NOM_WR_PARK2_EN_VAL		0x1
#define RTT_NOM_WR_PARK2_RTT_NOM_MR1_OFFS	3
#define RTT_NOM_WR_PARK2_RTT_NOM_MR1_MASK	0x7
#define RTT_NOM_WR_PARK2_RTT_NOM_MR1_VAL	0x8
#define RTT_NOM_WR_PARK2_RTT_WR_MR2_OFFS	9
#define RTT_NOM_WR_PARK2_RTT_WR_MR2_MASK	0x7
#define RTT_NOM_WR_PARK2_RTT_PARK_MR5_OFFS	6
#define RTT_NOM_WR_PARK2_RTT_PARK_MR5_MASK	0x7
#define RTT_NOM_WR_PARK2_RTT_PARK_MR5_VAL	0x1
#define RTT_NOM_WR_PARK2_REG_VAL	(((RTT_NOM_WR_PARK2_RTT_PARK_MR5_VAL & RTT_NOM_WR_PARK2_RTT_PARK_MR5_MASK) << \
					  RTT_NOM_WR_PARK2_RTT_PARK_MR5_OFFS) | \
					 ((RTT_NOM_WR_PARK2_EN_VAL & RTT_NOM_WR_PARK2_EN_MASK) << \
					  RTT_NOM_WR_PARK2_EN_OFFS))

/* Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 3 DRAM */
#define REG_5403A_1D_2D_RTT_NOM_WR_PARK3	0x5403a
#define RTT_NOM_WR_PARK3_EN_OFFS	0
#define RTT_NOM_WR_PARK3_EN_MASK	0x1
#define RTT_NOM_WR_PARK3_EN_VAL		0x1
#define RTT_NOM_WR_PARK3_RTT_NOM_MR1_OFFS	3
#define RTT_NOM_WR_PARK3_RTT_NOM_MR1_MASK	0x7
#define RTT_NOM_WR_PARK3_RTT_NOM_MR1_VAL	0x8
#define RTT_NOM_WR_PARK3_RTT_WR_MR2_OFFS	9
#define RTT_NOM_WR_PARK3_RTT_WR_MR2_MASK	0x7
#define RTT_NOM_WR_PARK3_RTT_PARK_MR5_OFFS	6
#define RTT_NOM_WR_PARK3_RTT_PARK_MR5_MASK	0x7
#define RTT_NOM_WR_PARK3_RTT_PARK_MR5_VAL	0x1
#define RTT_NOM_WR_PARK3_REG_VAL	(((RTT_NOM_WR_PARK3_RTT_PARK_MR5_VAL & RTT_NOM_WR_PARK3_RTT_PARK_MR5_MASK) << \
					  RTT_NOM_WR_PARK3_RTT_PARK_MR5_OFFS) | \
					 ((RTT_NOM_WR_PARK3_EN_VAL & RTT_NOM_WR_PARK3_EN_MASK) << \
					  RTT_NOM_WR_PARK3_EN_OFFS))

/* Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 4 DRAM */
#define REG_5403B_1D_2D_RTT_NOM_WR_PARK4	0x5403b
#define RTT_NOM_WR_PARK4_EN_OFFS	0
#define RTT_NOM_WR_PARK4_EN_MASK	0x1
#define RTT_NOM_WR_PARK4_EN_VAL		0x1
#define RTT_NOM_WR_PARK4_RTT_NOM_MR1_OFFS	3
#define RTT_NOM_WR_PARK4_RTT_NOM_MR1_MASK	0x7
#define RTT_NOM_WR_PARK4_RTT_NOM_MR1_VAL	0x8
#define RTT_NOM_WR_PARK4_RTT_WR_MR2_OFFS	9
#define RTT_NOM_WR_PARK4_RTT_WR_MR2_MASK	0x7
#define RTT_NOM_WR_PARK4_RTT_PARK_MR5_OFFS	6
#define RTT_NOM_WR_PARK4_RTT_PARK_MR5_MASK	0x7
#define RTT_NOM_WR_PARK4_RTT_PARK_MR5_VAL	0x1
#define RTT_NOM_WR_PARK4_REG_VAL	(((RTT_NOM_WR_PARK4_RTT_PARK_MR5_VAL & RTT_NOM_WR_PARK4_RTT_PARK_MR5_MASK) << \
					  RTT_NOM_WR_PARK4_RTT_PARK_MR5_OFFS) | \
					 ((RTT_NOM_WR_PARK4_EN_VAL & RTT_NOM_WR_PARK4_EN_MASK) << \
					  RTT_NOM_WR_PARK4_EN_OFFS))

/* Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 5 DRAM */
#define REG_5403C_1D_2D_RTT_NOM_WR_PARK5	0x5403c
#define RTT_NOM_WR_PARK5_EN_OFFS	0
#define RTT_NOM_WR_PARK5_EN_MASK	0x1
#define RTT_NOM_WR_PARK5_EN_VAL		0x1
#define RTT_NOM_WR_PARK5_RTT_NOM_MR1_OFFS	3
#define RTT_NOM_WR_PARK5_RTT_NOM_MR1_MASK	0x7
#define RTT_NOM_WR_PARK5_RTT_NOM_MR1_VAL	0x8
#define RTT_NOM_WR_PARK5_RTT_WR_MR2_OFFS	9
#define RTT_NOM_WR_PARK5_RTT_WR_MR2_MASK	0x7
#define RTT_NOM_WR_PARK5_RTT_PARK_MR5_OFFS	6
#define RTT_NOM_WR_PARK5_RTT_PARK_MR5_MASK	0x7
#define RTT_NOM_WR_PARK5_RTT_PARK_MR5_VAL	0x1
#define RTT_NOM_WR_PARK5_REG_VAL	(((RTT_NOM_WR_PARK5_RTT_PARK_MR5_VAL & RTT_NOM_WR_PARK5_RTT_PARK_MR5_MASK) << \
					  RTT_NOM_WR_PARK5_RTT_PARK_MR5_OFFS) | \
					 ((RTT_NOM_WR_PARK5_EN_VAL & RTT_NOM_WR_PARK5_EN_MASK) << \
					  RTT_NOM_WR_PARK5_EN_OFFS))

/* Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 6 DRAM */
#define REG_5403D_1D_2D_RTT_NOM_WR_PARK6	0x5403d
#define RTT_NOM_WR_PARK6_EN_OFFS	0
#define RTT_NOM_WR_PARK6_EN_MASK	0x1
#define RTT_NOM_WR_PARK6_EN_VAL		0x1
#define RTT_NOM_WR_PARK6_RTT_NOM_MR1_OFFS	3
#define RTT_NOM_WR_PARK6_RTT_NOM_MR1_MASK	0x7
#define RTT_NOM_WR_PARK6_RTT_NOM_MR1_VAL	0x8
#define RTT_NOM_WR_PARK6_RTT_WR_MR2_OFFS	9
#define RTT_NOM_WR_PARK6_RTT_WR_MR2_MASK	0x7
#define RTT_NOM_WR_PARK6_RTT_PARK_MR5_OFFS	6
#define RTT_NOM_WR_PARK6_RTT_PARK_MR5_MASK	0x7
#define RTT_NOM_WR_PARK6_RTT_PARK_MR5_VAL	0x1
#define RTT_NOM_WR_PARK6_REG_VAL	(((RTT_NOM_WR_PARK6_RTT_PARK_MR5_VAL & RTT_NOM_WR_PARK6_RTT_PARK_MR5_MASK) << \
					  RTT_NOM_WR_PARK6_RTT_PARK_MR5_OFFS) | \
					 ((RTT_NOM_WR_PARK6_EN_VAL & RTT_NOM_WR_PARK6_EN_MASK) << \
					  RTT_NOM_WR_PARK6_EN_OFFS))

/* Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 7 DRAM */
#define REG_5403E_1D_2D_RTT_NOM_WR_PARK7	0x5403e
#define RTT_NOM_WR_PARK7_EN_OFFS	0
#define RTT_NOM_WR_PARK7_EN_MASK	0x1
#define RTT_NOM_WR_PARK7_EN_VAL		0x1
#define RTT_NOM_WR_PARK7_RTT_NOM_MR1_OFFS	3
#define RTT_NOM_WR_PARK7_RTT_NOM_MR1_MASK	0x7
#define RTT_NOM_WR_PARK7_RTT_NOM_MR1_VAL	0x8
#define RTT_NOM_WR_PARK7_RTT_WR_MR2_OFFS	9
#define RTT_NOM_WR_PARK7_RTT_WR_MR2_MASK	0x7
#define RTT_NOM_WR_PARK7_RTT_PARK_MR5_OFFS	6
#define RTT_NOM_WR_PARK7_RTT_PARK_MR5_MASK	0x7
#define RTT_NOM_WR_PARK7_RTT_PARK_MR5_VAL	0x1
#define RTT_NOM_WR_PARK7_REG_VAL	(((RTT_NOM_WR_PARK7_RTT_PARK_MR5_VAL & RTT_NOM_WR_PARK7_RTT_PARK_MR5_MASK) << \
					  RTT_NOM_WR_PARK7_RTT_PARK_MR5_OFFS) | \
					 ((RTT_NOM_WR_PARK7_EN_VAL & RTT_NOM_WR_PARK7_EN_MASK) << \
					  RTT_NOM_WR_PARK7_EN_OFFS))


/* Sequence Control: set the training steps to run.
 * Each bit corresponds to a training step */
#define REG_5400B_SEQUENCE_CTRL		0x5400b
#define SEQUENCE_CTRL_DEV_INIT		(1 << 0)	/* Device/PHY init */
#define SEQEUNCE_CTRL_ALL		0xff

/* 1D training Sequence control options */
#define SEQUENCE_CTRL_1D_WR_LVL		(1 << 1)	/* Write leveling */
#define SEQUENCE_CTRL_1D_RX_EN		(1 << 2)	/* Read gate training */
#define SEQUENCE_CTRL_1D_RD_DQS_1D	(1 << 3)	/* 1D read dqs training */
#define SEQUENCE_CTRL_1D_WR_DQ_1D	(1 << 4)	/* 1D write dq training */
#define SEQUENCE_CTRL_1D_RFU5		(1 << 5)	/* RFU, must be zero */
#define SEQUENCE_CTRL_1D_RFU6		(1 << 6)	/* RFU, must be zero */
#define SEQUENCE_CTRL_1D_RFU7		(1 << 7)	/* RFU, must be zero */
#define SEQUENCE_CTRL_1D_RD_DESKEW	(1 << 8)	/* Per lane read dq deskew training */
#define SEQUENCE_CTRL_1D_MX_RD_LAT	(1 << 9)	/* Max read latency training */
#define SEQUENCE_CTRL_1D_RESERVED10	(1 << 10)	/* Reserved */
#define SEQUENCE_CTRL_1D_RESERVED11	(1 << 11)	/* Reserved */
#define SEQUENCE_CTRL_1D_RESERVED12	(1 << 12)	/* Reserved */
#define SEQUENCE_CTRL_1D_RESERVED13	(1 << 13)	/* Reserved */
#define SEQUENCE_CTRL_1D_RFU14		(1 << 14)	/* RFU, must be zero */
#define SEQUENCE_CTRL_1D_RFU15		(1 << 15)	/* RFU, must be zero */
#define SEQEUNCE_CTRL_1D_COUNT		16

/* 2D training Sequence control options */
#define SEQUENCE_CTRL_2D_READ_DQS	(1 << 5)	/* 2D read dqs training */
#define SEQUENCE_CTRL_2D_WRITE_DQ	(1 << 6)	/* 2D write dq training */

/* HdtCtrl: Hardware Debug Trace Control */
#define REG_5400C_HDT_CTRL_REG		0x5400c
#define HDT_CTRL_DETAIL_DEBUG		0x05 /* e.g Eye Delays */
#define HDT_CTRL_COARSE_DEBUG		0x0a /* e.g Rank information */
#define HDT_CTRL_STAGE_COMLETION	0xc8
#define HDT_CTRL_ASSERT_MSGS		0xc9
#define HDT_CTRL_FW_COMPLETION		0xff

/* 1D Message Block registers */
/* CDD_RW_1_0 & CDD_RW_0_3:
 * Read to write critical delay difference from cs 1 to cs 0 and from 0 to 3 */
#define REG_54024_1D_CDD_RW_1_0_RW_0_3_ADDR	0x54024


/* 2D Message Block registers */
/* DFIMRLMargin & R0_RxClkDly_Margin: Distance from the trained center to closest failing region in DLL steps.
 * This value is the minimum of all eyes in this timing group */
#define REG_54012_2D_R0_RX_CLK_DELAY_MARGIN_ADDR	0x54012



/* MicroContMuxSel: Enable/Isolate access to the internal CSRs */
#define MICRO_CONT_MUX_SEL_REG		0xd0000
#define MICRO_CONT_MUX_SEL_ENABLE	0x0
#define MICRO_CONT_MUX_SEL_ISOLATE	0x1

/* MicroReset CSR reg: trigger training execution */
#define MICRO_RESET_CSR_REG		0xd0099
#define MICRO_RESET_CSR_STALL_TO_MICRO	(1 << 0)
#define MICRO_RESET_CSR_RESET_TO_MICRO	(1 << 3)

/* Mail Box registers */

/* This is used for the mailbox protocol between the firmware and the system*/
#define UCT_SHADOW_REGS				0xd0004
/* When following bit set to 0, the PMU has a message for the user */
#define UCT_SHADOW_REGS_WRITE_PROT_MASK		(1 << 0)

/* DCT downstream mailbox protocol CSR. */
#define DCT_WRITE_PROT_REG			0xd0031
/* By setting this register to 0, the user acknowledges the receipt of the message */
#define DCT_WRITE_PROT_MASK			(1 << 0)




/* This is used for the mailbox protocol between the firmware and the system*/
#define UCT_WRITE_ONLY_SHADOW_REG		0xd0032
/* Used to pass the message ID for major messages.
 * Also used to pass the lower 16 bits for streaming messages. */
#define UCT_WRITE_ONLY_SHADOW_MASK		0xffff

/* Read-only view of the csr UctDatWriteOnly */
#define UCT_DAT_WRITE_ONLY_SHADOW_REG		0xd0034
/* Used to pass the upper 16 bits for streaming messages.
 * Not used in passing major messages */
#define UCT_DAT_WRITE_ONLY_SHADOW_MASK		0xffff


/* Mail box message format */
#define MAILBOX_STREAM_MSG_ID_OFFSET		16
#define MAILBOX_STREAM_MSG_ARG_COUNT_MASK	0xff
#endif	/* _SNPS_REGS_H_ */
