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

#if defined(CONFIG_PHY_STATIC) || defined(CONFIG_PHY_STATIC_PRINT) || \
	defined(CONFIG_MC_STATIC)  || defined(CONFIG_MC_STATIC_PRINT)

#include "ddr3_init.h"
#include "mv_ddr_mc6.h"

#define MAX_DATA_PHY_NUM	MAX_BUS_NUM
#define MAX_CTRL_PHY_NUM	3
#define DUNIT_BASE_ADDR		0x10000

#if defined(A80X0)
#define DATA_PHY_REGS_NUM	44
#define CTRL_PHY_REGS_NUM	7
#define DUNIT_4PHY_REGS_NUM	5
#define MC6_4PHY_REGS_NUM	3
#define MC6_REGS_NUM		47
#define DUNIT_REGS_NUM		26
#endif /* A80X0 */


#if defined(CONFIG_PHY_STATIC) || defined(CONFIG_MC_STATIC)
struct mc_reg_data {
	u32 addr;
	u32 data;
};

struct mc_regs_config {
	u32 num;
	struct mc_reg_data *regs_data;
};
#endif /* CONFIG_PHY_STATIC or CONFIG_MC_STATIC */

#if defined(CONFIG_MC_STATIC_PRINT) /* print mc static configuration */
#if defined(A80X0)
static u32 dunit_regs_list[DUNIT_REGS_NUM] = {
	SDRAM_CFG_REG,
	HORZ_SSTL_CAL_MACH_CTRL_REG,
	HORZ_POD_CAL_MACH_CTRL_REG,
	VERT_SSTL_CAL_MACH_CTRL_REG,
	VERT_POD_CAL_MACH_CTRL_REG,
	DUNIT_CTRL_LOW_REG,
	SDRAM_TIMING_LOW_REG,
	SDRAM_TIMING_HIGH_REG,
	SDRAM_ADDR_CTRL_REG,
	SDRAM_OPEN_PAGES_CTRL_REG,
	DUNIT_CTRL_HIGH_REG,
	DDR_TIMING_REG,
	SDRAM_ODT_CTRL_HIGH_REG,
	DRAM_DLL_TIMING_REG,
	DRAM_ZQ_INIT_TIMIMG_REG,
	DRAM_ZQ_TIMING_REG,
	DRAM_LONG_TIMING_REG,
	DDR3_RANK_CTRL_REG,
	ZQC_CFG_REG,
	DRAM_PHY_CFG_REG,
	DDR4_MR0_REG,
	DDR4_MR1_REG,
	DDR4_MR2_REG,
	DDR4_MR3_REG,
	DDR4_MR4_REG,
	DDR4_MR5_REG
};

static u32 mc6_regs_list[MC6_REGS_NUM] = {
	MC6_MC_CTRL0_REG,
	MC6_RAS_CTRL_REG,
	MC6_SPOOL_CTRL_REG,
	MC6_MC_PWR_CTRL_REG,
	MC6_RD_DPATH_CTRL_REG,
	MC6_RPP_STARVATION_CTRL_REG,
	MC6_CH0_MC_CTRL1_REG,
	MC6_CH0_MC_CTRL2_REG,
	MC6_CH0_MC_CTRL3_REG,
	MC6_CH0_MMAP_LOW_REG(0),
	MC6_CH0_MMAP_HIGH_REG(0),
	MC6_CH1_MMAP_LOW_REG(0),
	MC6_CH1_MMAP_HIGH_REG(0),
	MC6_CH0_MC_CFG_REG(0),
	MC6_CH0_DRAM_CFG1_REG,
	MC6_CH0_DRAM_CFG2_REG,
	MC6_CH0_DRAM_CFG3_REG,
	MC6_CH0_DRAM_CFG4_REG,
	MC6_CH0_DRAM_CFG5_REG(0),
	MC6_CH0_DRAM_CFG5_REG(1),
	MC6_CH0_DRAM_CFG5_REG(2),
	MC6_CH0_DRAM_CFG5_REG(3),
	MC6_CH0_ODT_CTRL1_REG,
	MC6_CH0_ODT_CTRL2_REG,
	MC6_CH0_DDR_INIT_TIMING_CTRL0_REG,
	MC6_CH0_DDR_INIT_TIMING_CTRL1_REG,
	MC6_CH0_DDR_INIT_TIMING_CTRL2_REG,
	MC6_CH0_ZQC_TIMING0_REG,
	MC6_CH0_ZQC_TIMING1_REG,
	MC6_CH0_REFRESH_TIMING_REG,
	MC6_CH0_SELFREFRESH_TIMING0_REG,
	MC6_CH0_SELFREFRESH_TIMING1_REG,
	MC6_CH0_PWRDOWN_TIMING0_REG,
	MC6_CH0_PWRDOWN_TIMING1_REG,
	MC6_CH0_MRS_TIMING_REG,
	MC6_CH0_ACT_TIMING_REG,
	MC6_CH0_PRECHARGE_TIMING_REG,
	MC6_CH0_CAS_RAS_TIMING0_REG,
	MC6_CH0_CAS_RAS_TIMING1_REG,
	MC6_CH0_OFF_SPEC_TIMING0_REG,
	MC6_CH0_OFF_SPEC_TIMING1_REG,
	MC6_CH0_DRAM_READ_TIMING_REG,
	MC6_CH0_DRAM_MPD_TIMING_REG,
	MC6_CH0_DRAM_PDA_TIMING_REG,
	MC6_CH0_PHY_CTRL1_REG,
	MC6_CH0_PHY_WL_RL_CTRL_REG,
	MC6_CH0_PHY_RL_CTRL_B0_REG(0)
};

static int mc_regs_print(void)
{
	u32 i, addr, data;

	printf("static struct mc_reg_data mc_regs_data[%s] = {\n",
	       "MC6_REGS_NUM + DUNIT_REGS_NUM + 1");

	for (i = 0; i < DUNIT_REGS_NUM; i++) {
		addr = dunit_regs_list[i] + DUNIT_BASE_ADDR;
		data = reg_read(addr);

		if (addr == (SDRAM_CFG_REG + DUNIT_BASE_ADDR)) {
			printf("\t{0x%x, 0x%x},\n",
			       addr,
			       data &
			       ~(PUP_RST_DIVIDER_MASK << PUP_RST_DIVIDER_OFFS));
		}

		if (i < (DUNIT_REGS_NUM - 1)) {
			printf("\t{0x%x, 0x%x},\n", addr, data);
		} else {
			if (MC6_REGS_NUM > 0)
				printf("\t{0x%x, 0x%x},\n", addr, data);
			else
				printf("\t{0x%x, 0x%x}\n", addr, data);
		}
	}

	for (i = 0; i < MC6_REGS_NUM; i++) {
		addr = MC6_BASE + mc6_regs_list[i];
		data = reg_read(addr);

		if (i < (MC6_REGS_NUM - 1))
			printf("\t{0x%x, 0x%x},\n", addr, data);
		else
			printf("\t{0x%x, 0x%x}\n", addr, data);
	}
	printf("};\n\n");

	printf("static struct mc_regs_config mc_regs_cfg = {\n");
	printf("\t.num = %s,\n", "MC6_REGS_NUM + DUNIT_REGS_NUM + 1");
	printf("\t.regs_data = mc_regs_data\n");
	printf("};\n\n");

	return MV_OK;
}

int mv_ddr_mc_static_print(void)
{
	mc_regs_print();

	return MV_OK;
}
#else /* not A80X0 */
int mv_ddr_mc_static_print(void)
{
	return MV_OK;
}
#endif /* A80X0 */
#endif /* CONFIG_MC_STATIC_PRINT */

#if defined(CONFIG_MC_STATIC) /* configure mc statically */
#if defined(A80X0)
static struct mc_reg_data mc_regs_data[MC6_REGS_NUM + DUNIT_REGS_NUM + 1] = {
	{0x11400, 0x6b10cc30},
	{0x11400, 0x7b10cc30},
	{0x114c8, 0x51420d1},
	{0x117c8, 0x5143091},
	{0x11dc8, 0x51420c1},
	{0x11ec8, 0x5143091},
	{0x11404, 0x36300848},
	{0x11408, 0x5311baa9},
	{0x1140c, 0x6e421f97},
	{0x11410, 0x13114444},
	{0x11414, 0x700},
	{0x11424, 0x60f3f7},
	{0x1142c, 0x14c5133},
	{0x11498, 0xf},
	{0x114e0, 0x400},
	{0x114e4, 0x3ff},
	{0x114e8, 0x1ff01ff},
	{0x114ec, 0x53},
	{0x115e0, 0x1},
	{0x115e4, 0x203c18},
	{0x115ec, 0xd8000029},
	{0x11900, 0x310},
	{0x11904, 0x1},
	{0x11908, 0x200},
	{0x1190c, 0x0},
	{0x11910, 0x0},
	{0x11914, 0x460},
	{0x20044, 0x30400},
	{0x2004c, 0x2},
	{0x20050, 0xff},
	{0x20054, 0x4c0},
	{0x20064, 0x606},
	{0x20180, 0x30200},
	{0x202c0, 0x6000},
	{0x202c4, 0x120030},
	{0x202c8, 0xfefe},
	{0x20200, 0x110001},
	{0x20204, 0x0},
	{0x20400, 0x110001},
	{0x20404, 0x2},
	{0x20220, 0x5010639},
	{0x20300, 0x90b},
	{0x20304, 0x0},
	{0x20308, 0x1},
	{0x2030c, 0x90000},
	{0x20310, 0x21000000},
	{0x20314, 0x0},
	{0x20318, 0x0},
	{0x2031c, 0x0},
	{0x20340, 0x0},
	{0x20344, 0x30000000},
	{0x20380, 0x61a80},
	{0x20384, 0x27100},
	{0x20388, 0x9600050},
	{0x2038c, 0x1b0400},
	{0x20390, 0x800200},
	{0x20394, 0x1180618},
	{0x20398, 0x1200300},
	{0x2039c, 0x200808},
	{0x203a0, 0x8040500},
	{0x203a4, 0x2},
	{0x203a8, 0x1808},
	{0x203ac, 0x28250b1a},
	{0x203b0, 0xb0c060b},
	{0x203b4, 0x6040602},
	{0x203b8, 0x404},
	{0x203bc, 0x1050505},
	{0x203c0, 0x50504},
	{0x203c4, 0x0},
	{0x203cc, 0x10120},
	{0x203d0, 0x0},
	{0x21000, 0x60},
	{0x210c0, 0x81000001},
	{0x21180, 0x500}
};

static struct mc_regs_config mc_regs_cfg = {
	.num = MC6_REGS_NUM + DUNIT_REGS_NUM + 1,
	.regs_data = mc_regs_data
};

static int mc_regs_set(void)
{
	u32 i, addr, data;

	for (i = 0; i < mc_regs_cfg.num; i++) {
		addr = mc_regs_cfg.regs_data[i].addr;
		data = mc_regs_cfg.regs_data[i].data;
		reg_write(addr, data);
	}

	return MV_OK;
}

int mv_ddr_mc_static_config(void)
{
	mc_regs_set();

	return MV_OK;
}
#else /* not A80X0 */
int mv_ddr_mc_static_config(void)
{
	return MV_OK;
}
#endif /* A80X0 */
#endif /* CONFIG_MC_STATIC */

#if defined(CONFIG_PHY_STATIC_PRINT) /* print phy static configuration */
#if defined(A80X0)
static u32 data_phy_regs_list[DATA_PHY_REGS_NUM] = {
	WL_PHY_REG(0),
	CTX_PHY_REG(0),
	RL_PHY_REG(0),
	CRX_PHY_REG(0),
	PBS_TX_PHY_REG(0, 0),
	PBS_TX_PHY_REG(0, 1),
	PBS_TX_PHY_REG(0, 2),
	PBS_TX_PHY_REG(0, 3),
	PBS_TX_PHY_REG(0, 4),
	PBS_TX_PHY_REG(0, 5),
	PBS_TX_PHY_REG(0, 6),
	PBS_TX_PHY_REG(0, 7),
	PBS_TX_PHY_REG(0, 8),
	PBS_TX_PHY_REG(0, 9),
	PBS_TX_PHY_REG(0, 10),
	PBS_RX_PHY_REG(0, 0),
	PBS_RX_PHY_REG(0, 1),
	PBS_RX_PHY_REG(0, 2),
	PBS_RX_PHY_REG(0, 3),
	PBS_RX_PHY_REG(0, 4),
	PBS_RX_PHY_REG(0, 5),
	PBS_RX_PHY_REG(0, 6),
	PBS_RX_PHY_REG(0, 7),
	PBS_RX_PHY_REG(0, 8),
	PBS_RX_PHY_REG(0, 9),
	PBS_RX_PHY_REG(0, 10),
	PHY_CTRL_PHY_REG,
	ADLL_CFG0_PHY_REG,
	ADLL_CFG1_PHY_REG,
	PAD_ZRI_CAL_PHY_REG,
	PAD_ODT_CAL_PHY_REG,
	PAD_CFG_PHY_REG,
	TEST_ADLL_PHY_REG,
	VREF_PHY_REG(0, 0),
	VREF_PHY_REG(0, 1),
	VREF_PHY_REG(0, 2),
	VREF_PHY_REG(0, 3),
	VREF_PHY_REG(0, 4),
	VREF_PHY_REG(0, 5),
	VREF_PHY_REG(0, 6),
	VREF_PHY_REG(0, 7),
	VREF_PHY_REG(0, 8),
	VREF_PHY_REG(0, 9),
	VREF_PHY_REG(0, 10)
};

static u32 ctrl_phy_regs_list[CTRL_PHY_REGS_NUM] = {
	WL_PHY_REG(0),
	PHY_CTRL_PHY_REG,
	ADLL_CFG0_PHY_REG,
	ADLL_CFG1_PHY_REG,
	PAD_ZRI_CAL_PHY_REG,
	PAD_ODT_CAL_PHY_REG,
	PAD_CFG_PHY_REG
};

static u32 dunit_4phy_regs_list[DUNIT_4PHY_REGS_NUM] = {
	DUNIT_ODT_CTRL_REG,
	MAIN_PADS_CAL_MACH_CTRL_REG,
	RD_DATA_SMPL_DLYS_REG,
	RD_DATA_RDY_DLYS_REG,
	DDR4_MR6_REG
};

static u32 mc6_4phy_regs_list[MC6_4PHY_REGS_NUM] = {
	MC6_RD_DPATH_CTRL_REG,
	MC6_CH0_PHY_CTRL1_REG,
	MC6_CH0_PHY_RL_CTRL_B0_REG(0)
};

static int phy_regs_print(void)
{
	u32 i, addr, data;
	int phy;

	/* print data phy registers */
	printf("static struct data_phy_reg_data dphy_regs_data[%s] = {\n",
	       "DATA_PHY_REGS_NUM");
	for (i = 0; i < DATA_PHY_REGS_NUM; i++) {
		addr = data_phy_regs_list[i];
		printf("\t{0x%x, {", addr);
		for (phy = 0; phy < MAX_DATA_PHY_NUM; phy++) {
			ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, phy, DDR_PHY_DATA, addr, &data);
			if (phy < (MAX_DATA_PHY_NUM - 1))
				printf("0x%x, ", data);
			else
				printf("0x%x}", data);
		}
		if (i < (DATA_PHY_REGS_NUM - 1))
			printf(" },\n");
		else
			printf(" }\n");
	}
	printf("};\n\n");

	/* print control phy registers */
	printf("static struct ctrl_phy_reg_data cphy_regs_data[%s] = {\n",
	       "CTRL_PHY_REGS_NUM");
	for (i = 0; i < CTRL_PHY_REGS_NUM; i++) {
		addr = ctrl_phy_regs_list[i];
		printf("\t{0x%x, {", addr);
		for (phy = 0; phy < MAX_CTRL_PHY_NUM; phy++) {
			ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, phy, DDR_PHY_CONTROL, addr, &data);
			if (phy < (MAX_CTRL_PHY_NUM - 1))
				printf("0x%x, ", data);
			else
				printf("0x%x}", data);
		}
		if (i < (CTRL_PHY_REGS_NUM - 1))
			printf(" },\n");
		else
			printf(" }\n");
	}
	printf("};\n\n");

	printf("static struct phy_regs_config phy_regs_cfg = {\n");
	printf("\t.dnum = %s,\n", "DATA_PHY_REGS_NUM");
	printf("\t.dphy_regs_data = dphy_regs_data,\n");
	printf("\t.cnum = %s,\n", "CTRL_PHY_REGS_NUM");
	printf("\t.cphy_regs_data = cphy_regs_data\n");
	printf("};\n\n");

	return MV_OK;
}

static int mc4phy_regs_print(void)
{
	u32 i, addr, data;

	printf("static struct mc_reg_data mc4phy_regs_data[%s] = {\n",
	       "DUNIT_4PHY_REGS_NUM + MC6_4PHY_REGS_NUM");
	for (i = 0; i < DUNIT_4PHY_REGS_NUM; i++) {
		addr = dunit_4phy_regs_list[i] + DUNIT_BASE_ADDR;
		data = reg_read(addr);
		if (i < (DUNIT_4PHY_REGS_NUM - 1)) {
			printf("\t{0x%x, 0x%x},\n", addr, data);
		} else {
			if (MC6_4PHY_REGS_NUM > 0)
				printf("\t{0x%x, 0x%x},\n", addr, data);
			else
				printf("\t{0x%x, 0x%x}\n", addr, data);
		}
	}

	for (i = 0; i < MC6_4PHY_REGS_NUM; i++) {
		addr = mc6_4phy_regs_list[i];
		data = reg_read(addr);
		if (i < (MC6_4PHY_REGS_NUM - 1))
			printf("\t{0x%x, 0x%x},\n", addr, data);
		else
			printf("\t{0x%x, 0x%x}\n", addr, data);
	}
	printf("};\n\n");

	printf("static struct mc_regs_config mc4phy_regs_cfg = {\n");
	printf("\t.num = %s,\n", "DUNIT_4PHY_REGS_NUM + MC6_4PHY_REGS_NUM");
	printf("\t.regs_data = mc4phy_regs_data\n");
	printf("};\n\n");

	return MV_OK;
}

int mv_ddr_phy_static_print(void)
{
	phy_regs_print();
	mc4phy_regs_print();

	return MV_OK;
}
#else /* not A80X0 */
int mv_ddr_phy_static_print(void)
{
	return MV_OK;
}
#endif /* A80X0 */
#endif /* CONFIG_PHY_STATIC_PRINT */

#if defined(CONFIG_PHY_STATIC) /* configure phy statically */
#if defined(A80X0)
struct data_phy_reg_data {
	u32 addr;
	u16 data[MAX_DATA_PHY_NUM];
};

struct ctrl_phy_reg_data {
	u32 addr;
	u16 data[MAX_CTRL_PHY_NUM];
};

struct phy_regs_config {
	u32 dnum;
	struct data_phy_reg_data *dphy_regs_data;
	u32 cnum;
	struct ctrl_phy_reg_data *cphy_regs_data;
};

static struct data_phy_reg_data dphy_regs_data[DATA_PHY_REGS_NUM] = {
	{0x0, {0x2c17, 0x5441, 0x7c49, 0x9c51, 0x7487, 0x888e, 0x2097, 0x3098, 0x445a} },
	{0x1, {0x30, 0x1a, 0x21, 0x2a, 0x20, 0x26, 0x32, 0x31, 0x33} },
	{0x2, {0x1d, 0x42, 0x4a, 0x52, 0x82, 0x8a, 0x93, 0x9f, 0x55} },
	{0x3, {0xa, 0xa, 0xa, 0xa, 0xa, 0xb, 0xb, 0xb, 0xc} },
	{0x10, {0x0, 0x6, 0x7, 0x8, 0xc, 0xa, 0x6, 0xb, 0x8} },
	{0x11, {0x5, 0x0, 0x0, 0x0, 0x3, 0x4, 0x0, 0x5, 0x0} },
	{0x12, {0x8, 0x0, 0xa, 0x0, 0x6, 0x0, 0x0, 0xc, 0x8} },
	{0x13, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0} },
	{0x14, {0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f} },
	{0x15, {0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f} },
	{0x16, {0x0, 0x1e, 0x2, 0xa, 0x2, 0x0, 0x0, 0x0, 0x0} },
	{0x17, {0x1f, 0x3, 0xf, 0xa, 0xc, 0xa, 0x1f, 0x0, 0xa} },
	{0x18, {0x5, 0xa, 0x2, 0x5, 0x0, 0x5, 0x4, 0x8, 0x3} },
	{0x19, {0x7, 0xc, 0xc, 0xa, 0x8, 0x0, 0x0, 0x0, 0xa} },
	{0x1a, {0xb, 0x2, 0x4, 0x3, 0x0, 0x0, 0x0, 0x6, 0x1} },
	{0x50, {0xa, 0xf, 0x14, 0x11, 0x12, 0x16, 0x14, 0x16, 0x10} },
	{0x51, {0x10, 0xa, 0xc, 0xa, 0x12, 0x16, 0x11, 0x10, 0xa} },
	{0x52, {0x14, 0xa, 0x12, 0xc, 0xf, 0xe, 0x11, 0x16, 0x12} },
	{0x53, {0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa} },
	{0x54, {0xb, 0x8, 0xb, 0xa, 0x9, 0xb, 0xa, 0xb, 0x9} },
	{0x55, {0xb, 0x8, 0xb, 0xa, 0x9, 0xb, 0xa, 0xb, 0x9} },
	{0x56, {0xc, 0x11, 0xa, 0x12, 0xc, 0xe, 0x14, 0x10, 0xa} },
	{0x57, {0x12, 0xc, 0x12, 0x12, 0x10, 0x14, 0x14, 0xc, 0x12} },
	{0x58, {0xf, 0x11, 0xe, 0xe, 0xa, 0x14, 0x12, 0x12, 0xa} },
	{0x59, {0x10, 0x11, 0x16, 0x14, 0x12, 0xa, 0xc, 0xa, 0x12} },
	{0x5a, {0x16, 0xc, 0xf, 0xf, 0xa, 0xc, 0xa, 0x14, 0xc} },
	{0x90, {0x6002, 0x6002, 0x6002, 0x6002, 0x6002, 0x6002, 0x6002, 0x6002, 0x6002} },
	{0x92, {0x1503, 0x1503, 0x1503, 0x1503, 0x1503, 0x1503, 0x1503, 0x1503, 0x1503} },
	{0x93, {0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50} },
	{0xa4, {0x70e, 0x70e, 0x70e, 0x70e, 0x70e, 0x70e, 0x70e, 0x70e, 0x70e} },
	{0xa6, {0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200} },
	{0xa8, {0x42a, 0x42a, 0x42a, 0x42a, 0x42a, 0x42a, 0x42a, 0x42a, 0x42a} },
	{0xbf, {0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1} },
	{0xd0, {0x20, 0x20, 0x20, 0x1f, 0x1f, 0x20, 0x20, 0x20, 0x20} },
	{0xd1, {0x20, 0x20, 0x20, 0x1f, 0x1f, 0x20, 0x20, 0x20, 0x20} },
	{0xd2, {0x20, 0x20, 0x20, 0x1f, 0x1f, 0x20, 0x20, 0x20, 0x20} },
	{0xd3, {0x20, 0x20, 0x20, 0x1f, 0x1f, 0x20, 0x20, 0x20, 0x20} },
	{0xd4, {0x20, 0x20, 0x20, 0x1f, 0x1f, 0x20, 0x20, 0x20, 0x20} },
	{0xd5, {0x20, 0x20, 0x20, 0x1f, 0x1f, 0x20, 0x20, 0x20, 0x20} },
	{0xd6, {0x20, 0x20, 0x20, 0x1f, 0x1f, 0x20, 0x20, 0x20, 0x20} },
	{0xd7, {0x20, 0x20, 0x20, 0x1f, 0x1f, 0x20, 0x20, 0x20, 0x20} },
	{0xd8, {0x20, 0x20, 0x20, 0x1f, 0x1f, 0x20, 0x20, 0x20, 0x20} },
	{0xd9, {0x20, 0x20, 0x20, 0x1f, 0x1f, 0x20, 0x20, 0x20, 0x20} },
	{0xda, {0x20, 0x20, 0x20, 0x1f, 0x1f, 0x20, 0x20, 0x20, 0x20} }
};

static struct ctrl_phy_reg_data cphy_regs_data[CTRL_PHY_REGS_NUM] = {
	{0x0, {0x8, 0x8, 0x8} },
	{0x90, {0x2002, 0x2002, 0x2002} },
	{0x92, {0x1503, 0x1503, 0x1503} },
	{0x93, {0x50, 0x50, 0x50} },
	{0xa4, {0x70c, 0x70c, 0x70c} },
	{0xa6, {0xd34, 0xd34, 0xd34} },
	{0xa8, {0x400, 0x400, 0x400} }
};

static struct phy_regs_config phy_regs_cfg = {
	.dnum = DATA_PHY_REGS_NUM,
	.dphy_regs_data = dphy_regs_data,
	.cnum = CTRL_PHY_REGS_NUM,
	.cphy_regs_data = cphy_regs_data
};

static int phy_regs_set(void)
{
	u32 i, addr, data;
	int phy;

	for (i = 0; i < phy_regs_cfg.dnum; i++) {
		addr = phy_regs_cfg.dphy_regs_data[i].addr;
		for (phy = 0; phy < MAX_DATA_PHY_NUM; phy++) {
			data = phy_regs_cfg.dphy_regs_data[i].data[phy];
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST,
					   0, ACCESS_TYPE_UNICAST,
					   phy, DDR_PHY_DATA, addr, data);
		}
	}

	for (i = 0; i < phy_regs_cfg.cnum; i++) {
		addr = phy_regs_cfg.cphy_regs_data[i].addr;
		for (phy = 0; phy < MAX_CTRL_PHY_NUM; phy++) {
			data = phy_regs_cfg.cphy_regs_data[i].data[phy];
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST,
					   0, ACCESS_TYPE_UNICAST,
					   phy, DDR_PHY_CONTROL, addr, data);
		}

	}

	return MV_OK;
}

static struct mc_reg_data mc4phy_regs_data[DUNIT_4PHY_REGS_NUM + MC6_4PHY_REGS_NUM] = {
	{0x1149c, 0x30f},
	{0x114cc, 0x80012015},
	{0x11538, 0xd},
	{0x1153c, 0x15},
	{0x11918, 0x808},
	{0x20064, 0x608},
	{0x21000, 0x90},
	{0x21180, 0x900}
};

static struct mc_regs_config mc4phy_regs_cfg = {
	.num = DUNIT_4PHY_REGS_NUM + MC6_4PHY_REGS_NUM,
	.regs_data = mc4phy_regs_data
};

static int mc4phy_regs_set(void)
{
	u32 i, addr, data;

	for (i = 0; i < mc4phy_regs_cfg.num; i++) {
		addr = mc4phy_regs_cfg.regs_data[i].addr;
		data = mc4phy_regs_cfg.regs_data[i].data;
		reg_write(addr, data);
	}

	return MV_OK;
}

int mv_ddr_phy_static_config(void)
{
	phy_regs_set();
	mc4phy_regs_set();

	return MV_OK;
}
#else /* not A80X0 */
int mv_ddr_phy_static_config(void)
{
	return MV_OK;
}
#endif /* A80X0 */

#endif /* CONFIG_PHY_STATIC */
#endif /* CONFIG_*_STATIC or CONFIG_*_STATIC_PRINT */
