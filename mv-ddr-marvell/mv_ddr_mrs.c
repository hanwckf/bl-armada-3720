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

/*
 * Based on Proposed DDR4 Full spec update (79-4B), Item No. 1716.78C
 */

/* MR0 WR (Write Recovery) and RTP (read to precharge), [13, 11:9] bits */
#define MV_DDR_MR0_WR10_RTP5	0x0	/* 0b00_0000_0000_0000 */
#define MV_DDR_MR0_WR12_RTP6	0x200	/* 0b00_0010_0000_0000 */
#define MV_DDR_MR0_WR14_RTP7	0x400	/* 0b00_0100_0000_0000 */
#define MV_DDR_MR0_WR16_RTP8	0x600	/* 0b00_0110_0000_0000 */
#define MV_DDR_MR0_WR18_RTP9	0x800	/* 0b00_1000_0000_0000 */
#define MV_DDR_MR0_WR20_RTP10	0xa00	/* 0b00_1010_0000_0000 */
#define MV_DDR_MR0_WR24_RTP12	0xc00	/* 0b00_1100_0000_0000 */
#define MV_DDR_MR0_WR22_RTP11	0xe00	/* 0b00_1110_0000_0000 */
#define MV_DDR_MR0_WR26_RTP13	0x2000	/* 0b10_0000_0000_0000 */

int mv_ddr_mr0_wr_get(unsigned int wr, unsigned int *mr0_wr)
{
	switch (wr) {
	case 10:
		*mr0_wr = MV_DDR_MR0_WR10_RTP5;
		break;
	case 12:
		*mr0_wr = MV_DDR_MR0_WR12_RTP6;
		break;
	case 14:
		*mr0_wr = MV_DDR_MR0_WR14_RTP7;
		break;
	case 16:
		*mr0_wr = MV_DDR_MR0_WR16_RTP8;
		break;
	case 18:
		*mr0_wr = MV_DDR_MR0_WR18_RTP9;
		break;
	case 20:
		*mr0_wr = MV_DDR_MR0_WR20_RTP10;
		break;
	case 24:
		*mr0_wr = MV_DDR_MR0_WR24_RTP12;
		break;
	case 22:
		*mr0_wr = MV_DDR_MR0_WR22_RTP11;
		break;
	case 26:
		*mr0_wr = MV_DDR_MR0_WR26_RTP13;
		break;
	default:
		printf("error: %s: unsupported t_wr value found\n", __func__);
		return -1;
	}

	return 0;
}

/* MR0 CL, [12, 6:4, 2] bits */
#define MV_DDR_MR0_CL9		0x0	/* 0b0_0000_0000_0000 */
#define MV_DDR_MR0_CL10		0x4	/* 0b0_0000_0000_0100 */
#define MV_DDR_MR0_CL11		0x10	/* 0b0_0000_0001_0000 */
#define MV_DDR_MR0_CL12		0x14	/* 0b0_0000_0001_0100 */
#define MV_DDR_MR0_CL13		0x20	/* 0b0_0000_0010_0000 */
#define MV_DDR_MR0_CL14		0x24	/* 0b0_0000_0010_0100 */
#define MV_DDR_MR0_CL15		0x30	/* 0b0_0000_0011_0000 */
#define MV_DDR_MR0_CL16		0x34	/* 0b0_0000_0011_0100 */
#define MV_DDR_MR0_CL18		0x40	/* 0b0_0000_0100_0000 */
#define MV_DDR_MR0_CL20		0x44	/* 0b0_0000_0100_0100 */
#define MV_DDR_MR0_CL22		0x50	/* 0b0_0000_0101_0000 */
#define MV_DDR_MR0_CL24		0x54	/* 0b0_0000_0101_0100 */
#define MV_DDR_MR0_CL23		0x60	/* 0b0_0000_0110_0000 */
#define MV_DDR_MR0_CL17		0x64	/* 0b0_0000_0110_0100 */
#define MV_DDR_MR0_CL19		0x70	/* 0b0_0000_0111_0000 */
#define MV_DDR_MR0_CL21		0x74	/* 0b0_0000_0111_0100 */
#define MV_DDR_MR0_CL25		0x1000	/* 0b1_0000_0000_0000 */
#define MV_DDR_MR0_CL26		0x1004	/* 0b1_0000_0000_0100 */
#define MV_DDR_MR0_CL27		0x1010	/* 0b1_0000_0001_0000 */
#define MV_DDR_MR0_CL28		0x1014	/* 0b1_0000_0001_0100 */
#define MV_DDR_MR0_CL29		0x1020	/* 0b1_0000_0010_0000 */
#define MV_DDR_MR0_CL30		0x1024	/* 0b1_0000_0010_0100 */
#define MV_DDR_MR0_CL31		0x1030	/* 0b1_0000_0011_0000 */
#define MV_DDR_MR0_CL32		0x1034	/* 0b1_0000_0011_0100 */

int mv_ddr_mr0_cl_get(unsigned int cl, unsigned int *mr0_cl)
{
	switch (cl) {
	case 9:
		*mr0_cl = MV_DDR_MR0_CL9;
		break;
	case 10:
		*mr0_cl = MV_DDR_MR0_CL10;
		break;
	case 11:
		*mr0_cl = MV_DDR_MR0_CL11;
		break;
	case 12:
		*mr0_cl = MV_DDR_MR0_CL12;
		break;
	case 13:
		*mr0_cl = MV_DDR_MR0_CL13;
		break;
	case 14:
		*mr0_cl = MV_DDR_MR0_CL14;
		break;
	case 15:
		*mr0_cl = MV_DDR_MR0_CL15;
		break;
	case 16:
		*mr0_cl = MV_DDR_MR0_CL16;
		break;
	case 18:
		*mr0_cl = MV_DDR_MR0_CL18;
		break;
	case 20:
		*mr0_cl = MV_DDR_MR0_CL20;
		break;
	case 22:
		*mr0_cl = MV_DDR_MR0_CL22;
		break;
	case 24:
		*mr0_cl = MV_DDR_MR0_CL24;
		break;
	case 23:
		*mr0_cl = MV_DDR_MR0_CL23;
		break;
	case 17:
		*mr0_cl = MV_DDR_MR0_CL17;
		break;
	case 19:
		*mr0_cl = MV_DDR_MR0_CL19;
		break;
	case 21:
		*mr0_cl = MV_DDR_MR0_CL21;
		break;
	case 25:
		*mr0_cl = MV_DDR_MR0_CL25;
		break;
	case 26:
		*mr0_cl = MV_DDR_MR0_CL26;
		break;
	case 27:
		*mr0_cl = MV_DDR_MR0_CL27;
		break;
	case 28:
		*mr0_cl = MV_DDR_MR0_CL28;
		break;
	case 29:
		*mr0_cl = MV_DDR_MR0_CL29;
		break;
	case 30:
		*mr0_cl = MV_DDR_MR0_CL30;
		break;
	case 31:
		*mr0_cl = MV_DDR_MR0_CL31;
		break;
	case 32:
		*mr0_cl = MV_DDR_MR0_CL32;
		break;
	default:
		printf("error: %s: unsupported cl value found\n", __func__);
		return -1;
	}

	return 0;
}

/* MR2 CWL, [5:3] bits */
#define MV_DDR_MR2_CWL9		0x0	/* 0b0000_0000 */
#define MV_DDR_MR2_CWL10	0x8	/* 0b0000_1000 */
#define MV_DDR_MR2_CWL11	0x10	/* 0b0001_0000 */
#define MV_DDR_MR2_CWL12	0x18	/* 0b0001_1000 */
#define MV_DDR_MR2_CWL14	0x20	/* 0b0010_0000 */
#define MV_DDR_MR2_CWL16	0x28	/* 0b0010_1000 */
#define MV_DDR_MR2_CWL18	0x30	/* 0b0011_0000 */
#define MV_DDR_MR2_CWL20	0x38	/* 0b0011_1000 */

int mv_ddr_mr2_cwl_get(unsigned int cwl, unsigned int *mr2_cwl)
{
	switch (cwl) {
	case 9:
		*mr2_cwl = MV_DDR_MR2_CWL9;
		break;
	case 10:
		*mr2_cwl = MV_DDR_MR2_CWL10;
		break;
	case 11:
		*mr2_cwl = MV_DDR_MR2_CWL11;
		break;
	case 12:
		*mr2_cwl = MV_DDR_MR2_CWL12;
		break;
	case 14:
		*mr2_cwl = MV_DDR_MR2_CWL14;
		break;
	case 16:
		*mr2_cwl = MV_DDR_MR2_CWL16;
		break;
	case 18:
		*mr2_cwl = MV_DDR_MR2_CWL18;
		break;
	case 20:
		*mr2_cwl = MV_DDR_MR2_CWL20;
		break;
	default:
		printf("error: %s: unsupported cwl value found\n", __func__);
		return -1;
	}

	return 0;
}

/* MR6 tCCD_L, [12:10] bits */
#define MV_DDR_MR6_TCCDL_OFFS		10
#define MV_DDR_MR6_TCCDL_MASK		0x7
#define MV_DDR_MR6_TCCDL4		0x0
#define MV_DDR_MR6_TCCDL5		0x1
#define MV_DDR_MR6_TCCDL6		0x2
#define MV_DDR_MR6_TCCDL7		0x3
#define MV_DDR_MR6_TCCDL8		0x4
int mv_ddr_mr6_tccdl_get(unsigned int tccdl, unsigned int *mr6_tccdl)
{
	switch (tccdl) {
	case 4:
		*mr6_tccdl = MV_DDR_MR6_TCCDL4 << MV_DDR_MR6_TCCDL_OFFS;
		break;
	case 5:
		*mr6_tccdl = MV_DDR_MR6_TCCDL5 << MV_DDR_MR6_TCCDL_OFFS;
		break;
	case 6:
		*mr6_tccdl = MV_DDR_MR6_TCCDL6 << MV_DDR_MR6_TCCDL_OFFS;
		break;
	case 7:
		*mr6_tccdl = MV_DDR_MR6_TCCDL7 << MV_DDR_MR6_TCCDL_OFFS;
		break;
	case 8:
		*mr6_tccdl = MV_DDR_MR6_TCCDL8 << MV_DDR_MR6_TCCDL_OFFS;
		break;
	default:
		printf("error: %s: unsupported t_ccd_l value found\n", __func__);
		return -1;
	}

	return 0;
}
