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

#ifndef _MV_DDR_MRS_H
#define _MV_DDR_MRS_H

/*
 * Based on Proposed DDR4 Full spec update (79-4B), Item No. 1716.78C
 */

/* MR1 DIC, [2:1] bits */
#define MV_DDR_MR1_DIC_OFFS		1

/* MR1 RTT_NOM, [10:8] bits */
#define MV_DDR_MR1_RTT_NOM_OFFS		8
#define MV_DDR_MR1_RTT_NOM_MASK		0x7
#define MV_DDR_MR1_RTT_NOM_DISABLE	0x0	/* 0b000_0000_0000 */
#define MV_DDR_MR1_RTT_NOM_RZQ_DIV4	0x100	/* 0b001_0000_0000; 60-Ohm */
#define MV_DDR_MR1_RTT_NOM_RZQ_DIV2	0x200	/* 0b010_0000_0000; 120-Ohm */
#define MV_DDR_MR1_RTT_NOM_RZQ_DIV6	0x300	/* 0b011_0000_0000; 40-Ohm */
#define MV_DDR_MR1_RTT_NOM_RZQ_DIV1	0x400	/* 0b100_0000_0000; 240-Ohm */
#define MV_DDR_MR1_RTT_NOM_RZQ_DIV5	0x500	/* 0b101_0000_0000; 48-Ohm */
#define MV_DDR_MR1_RTT_NOM_RZQ_DIV3	0x600	/* 0b110_0000_0000; 80-Ohm */
#define MV_DDR_MR1_RTT_NOM_RZQ_DIV7	0x700	/* 0b111_0000_0000; 34-Ohm */

/* MR2 RTT_WR, [11:9] bits */
#define MV_DDR_MR2_RTT_WR_OFFS		9
#define MV_DDR_MR2_RTT_WR_MASK		0x7
#define MV_DDR_MR2_RTT_WR_DYN_ODT_OFF	0x0	/* 0b0000_0000_0000 */
#define MV_DDR_MR2_RTT_WR_RZQ_DIV2	0x200	/* 0b0010_0000_0000; 120-Ohm */
#define MV_DDR_MR2_RTT_WR_RZQ_DIV1	0x400	/* 0b0100_0000_0000; 240-Ohm */
#define MV_DDR_MR2_RTT_WR_HIZ		0x600	/* 0b0110_0000_0000 */
#define MV_DDR_MR2_RTT_WR_RZQ_DIV3	0x800	/* 0b1000_0000_0000; 80-Ohm */

/* MR5 ODT Input Buffer during Power Down mode, bit 5 */
#define MV_DDR_MR5_PD_ODT_IBUF_OFFS	5
#define MV_DDR_MR5_PD_ODT_IBUF_MASK	0x1
#define MV_DDR_MR5_PD_ODT_IBUF_ENA	0
#define MV_DDR_MR5_PD_ODT_IBUF_DIS	1

/* MR5 Data Mask, bit 10 */
#define MV_DDR_MR5_DM_OFFS		10
#define MV_DDR_MR5_DM_MASK		0x1
#define MV_DDR_MR5_DM_ENA		1
#define MV_DDR_MR5_DM_DIS		0

/* MR5 RTT_PARK, [8:6] bits */
#define MV_DDR_MR5_RTT_PARK_OFFS	6
#define MV_DDR_MR5_RTT_PARK_MASK	0x7
#define MV_DDR_MR5_RTT_PARK_DISABLE	0x0	/* 0b0_0000_0000 */
#define MV_DDR_MR5_RTT_PARK_RZQ_DIV4	0x40	/* 0b0_0100_0000; 60-Ohm */
#define MV_DDR_MR5_RTT_PARK_RZQ_DIV2	0x80	/* 0b0_1000_0000; 120-Ohm */
#define MV_DDR_MR5_RTT_PARK_RZQ_DIV6	0xc0	/* 0b0_1100_0000; 40-Ohm */
#define MV_DDR_MR5_RTT_PARK_RZQ_DIV1	0x100	/* 0b1_0000_0000; 240-Ohm */
#define MV_DDR_MR5_RTT_PARK_RZQ_DIV5	0x140	/* 0b1_0100_0000; 48-Ohm */
#define MV_DDR_MR5_RTT_PARK_RZQ_DIV3	0x180	/* 0b1_1000_0000; 80-Ohm */
#define MV_DDR_MR5_RTT_PARK_RZQ_DIV7	0x1c0	/* 0b1_1100_0000; 34-Ohm */

/* MR6 VrefDQ Training Enable, bit 7 */
#define MV_DDR_MR6_VREFDQ_TRNG_ENA_OFFS	7
#define MV_DDR_MR6_VREFDQ_TRNG_ENA_MASK	0x1
#define MV_DDR_MR6_VREFDQ_TRNG_ENA	1	/* training mode */
#define MV_DDR_MR6_VREFDQ_TRNG_DIS	0	/* normal operation mode */

/* MR6 VrefDQ Training Range, bit 6 */
#define MV_DDR_MR6_VREFDQ_TRNG_RNG_OFFS	6
#define MV_DDR_MR6_VREFDQ_TRNG_RNG_MASK	0x1
#define MV_DDR_MR6_VREFDQ_TRNG_RNG1	0	/* range 1 */
#define MV_DDR_MR6_VREFDQ_TRNG_RNG2	1	/* range 2 */

/* MR6 VrefDQ Training Range Values, [5:0] bits */
#define MV_DDR_MR6_VREFDQ_TRNG_VAL_OFFS	0
#define MV_DDR_MR6_VREFDQ_TRNG_VAL_MASK	0x3f
#define MV_DDR_MR6_VREFDQ_TRNG_VAL	0x9	/* range 1: 65.85%; range 2: 50.85% */

int mv_ddr_mr0_wr_get(unsigned int wr, unsigned int *mr0_wr);
int mv_ddr_mr0_cl_get(unsigned int cl, unsigned int *mr0_cl);
int mv_ddr_mr2_cwl_get(unsigned int cwl, unsigned int *mr2_cwl);
int mv_ddr_mr6_tccdl_get(unsigned int tccdl, unsigned int *mr6_tccdl);

#endif /* _MV_DDR_MRS_H */
