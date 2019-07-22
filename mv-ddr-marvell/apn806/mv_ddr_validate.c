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

#include "ddr3_init.h"
#include "mv_ddr4_mpr_pda_if.h"
#include "mv_ddr_validate.h"
#include "mv_ddr_mc6.h"
#include "mv_ddr_xor_v2.h"
#include "mv_ddr_regs.h"
#include "../../drivers/marvell/thermal.h"

#undef DBG_CPU_SWEEP_TEST
#undef DBG_CPU_SWEEP_TEST_TX

/* temperature correction steps */
#define VREF_TEMP_CORR_STEP1	20
#define VREF_TEMP_CORR_STEP2	10
#define VREF_TEMP_CORR_STEP3	0
#define VREF_TEMP_CORR_STEP4	(-15)
#define CRX_TEMP_CORR_STEP1	50
#define CRX_TEMP_CORR_STEP2	25
#define CRX_TEMP_CORR_STEP3	0
#define CRX_TEMP_CORR_STEP4	(-15)

#define MAX_VREF_RANGE		64
#define MAX_ADLL_RANGE		32
#define VREF_CORR_FACTOR	4

#define DBG_DMA_DATA_SIZE		0x2000		/* 4KB; can be modified with proper alignment */
#define DBG_DMA_DESC_NUM		128
#define DBG_DMA_ENG_NUM			4
#define DBG_DMA_SRC_ADDR		(1 << 20)	/* can be modified with proper alignment */
#define DBG_DMA_DST_ADDR		(10 << 20)	/* can be modified with proper alignment */
#define DBG_MAX_CS_NUM			2
#if ((DBG_DMA_DST_ADDR) < ((DBG_DMA_SRC_ADDR) + (DBG_DMA_DATA_SIZE)))
#error "dma src and dst addr too close to each other..."
#endif

static uint64_t dma_src[DBG_MAX_CS_NUM][DBG_DMA_ENG_NUM], dma_dst[DBG_MAX_CS_NUM][DBG_DMA_ENG_NUM];
extern u8 dq_vref_vec[MAX_BUS_NUM];	/* stability support */
extern u8 rx_eye_hi_lvl[MAX_BUS_NUM];	/* vertical adjustment support */
extern u8 rx_eye_lo_lvl[MAX_BUS_NUM];	/* vertical adjustment support */

/* dma pattern */
uint64_t dma_pattern[] = {
#if defined(CONFIG_64BIT)
	0xf7f7f7f7f7f7f7f7,
	0x0808080808080808,
	0xf7f7f7f7f7f7f7f7,
	0xfbfbfbfbfbfbfbfb,
	0x0404040404040404,
	0x7f7f7f7f7f7f7f7f,
	0x8080808080808080,
	0x4040404040404040,
	0xbfbfbfbfbfbfbfbf,
	0x4040404040404040,
	0x2020202020202020,
	0xdfdfdfdfdfdfdfdf,
	0x2020202020202020,
	0x1010101010101010,
	0xefefefefefefefef,
	0x1010101010101010,
	0x0808080808080808,
	0xf7f7f7f7f7f7f7f7,
	0x0808080808080808,
	0x0404040404040404,
	0xfbfbfbfbfbfbfbfb,
	0x0404040404040404,
	0x0202020202020202,
	0xfdfdfdfdfdfdfdfd,
	0x0202020202020202,
	0x0101010101010101,
	0xfefefefefefefefe,
	0x0101010101010101,
	0x8080808080808080,
	0x7f7f7f7f7f7f7f7f,
	0x8080808080808080,
	0x4040404040404040,
	0xbfbfbfbfbfbfbfbf,
	0x4040404040404040,
	0x2020202020202020,
	0xdfdfdfdfdfdfdfdf,
	0x2020202020202020,
	0x1010101010101010,
	0xefefefefefefefef,
	0x1010101010101010,
	0x0808080808080808,
	0xf7f7f7f7f7f7f7f7,
	0x0808080808080808,
	0x0404040404040404,
	0xfbfbfbfbfbfbfbfb,
	0x0404040404040404,
	0x0202020202020202,
	0xfdfdfdfdfdfdfdfd,
	0x0202020202020202,
	0x0101010101010101,
	0xfefefefefefefefe,
	0x0101010101010101,
	0x8080808080808080,
	0x7f7f7f7f7f7f7f7f,
	0x8080808080808080,
	0x4040404040404040,
	0xbfbfbfbfbfbfbfbf,
	0x4040404040404040,
	0x2020202020202020,
	0xdfdfdfdfdfdfdfdf,
	0x2020202020202020,
	0x1010101010101010,
	0xefefefefefefefef,
	0x1010101010101010,
	0x0808080808080808,
	0xf7f7f7f7f7f7f7f7,
	0x0808080808080808,
	0x0404040404040404,
	0xfbfbfbfbfbfbfbfb,
	0xffffffffffffffff,
	0x0000000000000000,
	0xffffffffffffffff,
	0x0000000000000000,
	0xffffffffffffffff,
	0x0000000000000000,
	0xffffffffffffffff,
	0x0000000000000000,
	0x0000000000000000,
	0xffffffffffffffff,
	0x0000000000000000,
	0xffffffffffffffff,
	0x0000000000000000,
	0xffffffffffffffff,
	0x0000000000000000,
	0xffffffffffffffff,
	0xaaaaaaaaaaaaaaaa,
	0x5555555555555555,
	0xaaaaaaaaaaaaaaaa,
	0x5555555555555555,
	0xaaaaaaaaaaaaaaaa,
	0x5555555555555555,
	0xaaaaaaaaaaaaaaaa,
	0x5555555555555555,
	0x5555555555555555,
	0xaaaaaaaaaaaaaaaa,
	0x5555555555555555,
	0xaaaaaaaaaaaaaaaa,
	0x5555555555555555,
	0xaaaaaaaaaaaaaaaa,
	0x5555555555555555,
	0xaaaaaaaaaaaaaaaa
#else /* !CONFIG_64BIT */
	0xf7f7f7f780808080,
	0xf7f7f7f7fbfbfbfb,
	0x404040407f7f7f7f,
	0x8080808040404040,
	0xbfbfbfbf40404040,
	0x20202020dfdfdfdf,
	0x2020202010101010,
	0xefefefef10101010,
	0x80808080f7f7f7f7,
	0x8080808040404040,
	0xfbfbfbfb40404040,
	0x20202020fdfdfdfd,
	0x2020202010101010,
	0xfefefefe10101010,
	0x808080807f7f7f7f,
	0x8080808040404040,
	0xbfbfbfbf40404040,
	0x20202020dfdfdfdf,
	0x2020202010101010,
	0xefefefef10101010,
	0x80808080f7f7f7f7,
	0x8080808040404040,
	0xfbfbfbfb40404040,
	0x20202020fdfdfdfd,
	0x2020202010101010,
	0xfefefefe10101010,
	0x808080807f7f7f7f,
	0x8080808040404040,
	0xbfbfbfbf40404040,
	0x20202020dfdfdfdf,
	0x2020202010101010,
	0xefefefef10101010,
	0x80808080f7f7f7f7,
	0x8080808040404040,
	0xffffffff00000000,
	0xffffffff00000000,
	0xffffffff00000000,
	0xffffffff00000000,
	0x00000000ffffffff,
	0x00000000ffffffff,
	0x00000000ffffffff,
	0x00000000ffffffff,
	0xaaaaaaaa55555555,
	0xaaaaaaaa55555555,
	0xaaaaaaaa55555555,
	0xaaaaaaaa55555555,
	0x55555555aaaaaaaa,
	0x55555555aaaaaaaa,
	0x55555555aaaaaaaa,
	0x55555555aaaaaaaa

#endif /* CONFIG_64BIT */
};

static int new_vref_calc(u8 vref_low, u8 vref_hi, u32 *new_vref)
{
	u32 curr_vref;
	u32 eye_vref_height;
	u32 vref_corr;

	if (vref_hi < vref_low) {
		printf("%s: incorrect input parameters found\n", __func__);
		return 1; /* fail */
	}

	eye_vref_height = vref_hi - vref_low;
	curr_vref = vref_low + (eye_vref_height / 2);
	vref_corr = (eye_vref_height * eye_vref_height) /
		    (VREF_CORR_FACTOR * MAX_VREF_RANGE);
	*new_vref = curr_vref - vref_corr;

	return 0;
}
static u32 vref_temp_corr_calc(int cdeg)
{
	u32 corr;

	if (cdeg > VREF_TEMP_CORR_STEP1)
		corr = 10;
	else if (cdeg > VREF_TEMP_CORR_STEP2)
		corr = 6;
	else if (cdeg > VREF_TEMP_CORR_STEP3)
		corr = 3;
	else if (cdeg > VREF_TEMP_CORR_STEP4)
		corr = 1;
	else
		corr = 0;

	return corr;
}

static u32 crx_temp_corr_calc(int cdeg)
{
	u32 corr;

	if (cdeg > CRX_TEMP_CORR_STEP1)
		corr = 2;
	else if (cdeg > CRX_TEMP_CORR_STEP2)
		corr = 1;
	else if (cdeg > CRX_TEMP_CORR_STEP3)
		corr = 0;
	else if (cdeg > CRX_TEMP_CORR_STEP4)
		corr = 0;
	else
		corr = 0;

	return corr;
}

static uint64_t dma_gap_calc(uint64_t size)
{
	switch (size) {
	case _1G:
	case _2G:
		return _256M;
	case _4G:
	case _8G:
		return _512M;
	default:
		return 0;
	}
}

static int dma_offs_config(uint64_t dma_src[][DBG_DMA_ENG_NUM],
			   uint64_t dma_dst[][DBG_DMA_ENG_NUM],
			   u32 cs)
{
	u32 dma_id;
	uint64_t dma_gap;
	uint64_t mem_size = mv_ddr_mem_sz_per_cs_get();

	dma_gap = dma_gap_calc(mem_size);

	for (dma_id = 0; dma_id < DBG_DMA_ENG_NUM; dma_id++) {
		dma_src[cs][dma_id] = DBG_DMA_SRC_ADDR + (dma_id * dma_gap) + (cs * mem_size);
		dma_dst[cs][dma_id] = DBG_DMA_DST_ADDR + (dma_id * dma_gap) + (cs * mem_size);
	}

	return 0;
}

static int dma_test(uint64_t dma_src[][DBG_DMA_ENG_NUM],
		    uint64_t dma_dst[][DBG_DMA_ENG_NUM],
		    u32 cs)
{
	int res = 0;

	mv_ddr_dma_memcpy(dma_src[cs], dma_dst[cs], DBG_DMA_DATA_SIZE, DBG_DMA_ENG_NUM, DBG_DMA_DESC_NUM);
	res = mv_ddr_dma_memcmp(dma_src[cs], dma_dst[cs], DBG_DMA_DATA_SIZE, DBG_DMA_ENG_NUM, DBG_DMA_DESC_NUM);

#ifdef DBG_PRINT
	if (res)
		printf("dma memcmp fail(%d)\n", res);
	else
		printf("dma memcmp pass\n");
#endif

	return res;
}

static int xor_search_1d_1e(enum hws_edge_compare edge, enum hws_search_dir search_dir,
			    u32 step, u32 init_val, u32 end_val,
			    u16 byte_num, enum search_element element)
{
	int result;
	int bs_left = (int)init_val;
	int bs_middle;
	int bs_right = (int)end_val;
	int bs_found = -1;
	u32 reg_addr = (element == CRX) ? CRX_PHY_BASE : CTX_PHY_BASE;

	reg_addr += (4 * effective_cs);

	if (search_dir == HWS_LOW2HIGH) {
		while (bs_left <= bs_right) {
			bs_middle = (bs_left + bs_right) / 2;

			if (element == REC_CAL) {
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num,
						   DDR_PHY_DATA, VREF_BCAST_PHY_REG(effective_cs), bs_middle);
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num,
						   DDR_PHY_DATA, VREF_PHY_REG(effective_cs, 4), bs_middle);
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num,
						   DDR_PHY_DATA, VREF_PHY_REG(effective_cs, 5), bs_middle);
			} else {
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num,
						   DDR_PHY_DATA, reg_addr, bs_middle);
			}

			/* reset phy fifo pointer after rx adll or rx cal change */
			ddr3_tip_reset_fifo_ptr(0);

			if (byte_num == 8)
				reg_write(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG, 0x0);

			result = dma_test(dma_src, dma_dst, effective_cs);

			if (byte_num == 8)
				result = reg_read(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG);

			/*
			 * continue to search for the leftmost fail if pass found in pass-to-fail case or
			 * for the leftmost pass if fail found in fail-to-pass case
			 */
			if ((result == 0 && edge == EDGE_PF) ||
			    (result != 0 && edge == EDGE_FP))
				bs_left = bs_middle + 1;

			/*
			 * save recently found fail in pass-to-fail case or
			 * recently found pass in fail-to-pass case
			 */
			if ((result != 0 && edge == EDGE_PF) ||
			    (result == 0 && edge == EDGE_FP)) {
				bs_found = bs_middle;
				bs_right = bs_middle - 1;
			}
		}
	} else { /* search_dir == HWS_HIGH2LOW */
		while (bs_left >= bs_right) {
			bs_middle = (bs_left + bs_right) / 2;

			if (element == REC_CAL) {
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num,
						   DDR_PHY_DATA, VREF_BCAST_PHY_REG(effective_cs), bs_middle);
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num,
						   DDR_PHY_DATA, VREF_PHY_REG(effective_cs, 4), bs_middle);
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num,
						   DDR_PHY_DATA, VREF_PHY_REG(effective_cs, 5), bs_middle);
			} else {
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num,
						   DDR_PHY_DATA, reg_addr, bs_middle);
			}

			/* reset phy fifo pointer after rx adll or rx cal change */
			ddr3_tip_reset_fifo_ptr(0);

			if (byte_num == 8)
				reg_write(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG, 0x0);

			result = dma_test(dma_src, dma_dst, effective_cs);

			if (byte_num == 8)
				result = reg_read(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG);

			/*
			 * continue to search for the leftmost fail if pass found in pass-to-fail case or
			 * for the leftmost pass if fail found in fail-to-pass case
			 */
			if ((result == 0 && edge == EDGE_PF) ||
			    (result != 0 && edge == EDGE_FP))
				bs_left = bs_middle - 1;

			/*
			 * save recently found fail in pass-to-fail case or
			 * recently found pass in fail-to-pass case
			 */
			if ((result != 0 && edge == EDGE_PF) ||
			    (result == 0 && edge == EDGE_FP)) {
				bs_found = bs_middle;
				bs_right = bs_middle + 1;
			}
		}
	}

	if (bs_found >= 0)
		result = bs_found;
	else if (edge == EDGE_PF)
		result = bs_right;

	else
		result = 255;

#ifdef DBG_PRINT
	printf("%d, %d, %d\n", init_val, end_val, result);
#endif
	return result;
}

static int xor_search_1d_2e(enum hws_edge_compare search_concept, u32 step, u32 init_val, u32 end_val,
			    u16 byte_num, enum search_element element, u8 *vw_vector)
{
	u32 reg_addr = (element == CRX) ? CRX_PHY_BASE : CTX_PHY_BASE;
	u32 reg_data;
	reg_addr += (4 * effective_cs);
	vw_vector[0] = end_val;
	vw_vector[1] = init_val;

	if (element == REC_CAL) {
		ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
				  VREF_BCAST_PHY_REG(effective_cs), &reg_data);
	} else if (element == CRX) {
		ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
				  CRX_PHY_REG(effective_cs), &reg_data);
	} else if (element == CTX) {
		ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
				  CTX_PHY_REG(effective_cs), &reg_data);
	}

	/* xor_search_1d_1e returns the edge transition value of the elements */
	if (search_concept == EDGE_FP) {
		vw_vector[0] = xor_search_1d_1e(EDGE_FP, HWS_LOW2HIGH, step, init_val, end_val,
						byte_num, element);

	vw_vector[1] = xor_search_1d_1e(EDGE_FP, HWS_HIGH2LOW, step, end_val, init_val,
					byte_num, element);

		if (element == CRX) {
			if (vw_vector[0] == 255) {
				vw_vector[0] = xor_search_1d_1e(EDGE_PF, HWS_HIGH2LOW, step,
								(vw_vector[1] * 2 - 4),
								init_val, byte_num, element);

			} else {
				vw_vector[1] = xor_search_1d_1e(EDGE_PF, HWS_LOW2HIGH, step,
								(vw_vector[0] -
								(end_val - vw_vector[0]) + 4),
								end_val, byte_num, element);
			}

		}
	} else {
		vw_vector[1] = xor_search_1d_1e(EDGE_PF, HWS_LOW2HIGH, step, reg_data, end_val,
						byte_num, element);
#ifdef DBG_PRINT
		printf("%s: vw_vector[1] = %d\n", __func__, vw_vector[1]);
#endif
		vw_vector[0] = xor_search_1d_1e(EDGE_PF, HWS_HIGH2LOW, step, reg_data, init_val,
						byte_num, element);
#ifdef DBG_PRINT
		printf("%s: vw_vector[0] = %d\n", __func__, vw_vector[0]);
#endif
		if ((vw_vector[1] == reg_data) && (vw_vector[0] == reg_data)) {
			vw_vector[1] = 255;
			vw_vector[0] = 255;
		} else {
			vw_vector[1] = vw_vector[1]; /* -step */
			vw_vector[0] = vw_vector[0]; /* +step */
		}
#ifdef DBG_PRINT
		printf("%s: vw_vector[0] = %d\n", __func__, vw_vector[0]);
		printf("%s: vw_vector[1] = %d\n", __func__, vw_vector[1]);
#endif
	}

	if (element == REC_CAL) {
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
				   VREF_BCAST_PHY_REG(effective_cs), reg_data);
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
				   VREF_PHY_REG(effective_cs, 4), reg_data);
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
				   VREF_PHY_REG(effective_cs, 5), reg_data);
	} else if (element == CRX) {
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
				   CRX_PHY_REG(effective_cs), reg_data);
	} else if (element == CTX) {
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
				   CTX_PHY_REG(effective_cs), reg_data);
	}

	if ((vw_vector[1] == 255) || (vw_vector[0] == 255)) {
		return 255;
	} else {
		if (vw_vector[1] < vw_vector[0]) {
			if (search_concept == EDGE_PF) {
#ifdef DBG_PRINT
				printf("%s: end_val %d, init_val %d, step %d, nominal %d\n",
				       __func__, end_val, init_val, step, reg_data);
#endif
			}
			return 0;
		} else {
			return vw_vector[1] - vw_vector[0];
		}
	}

	return 0;
}

static int xor_search_2d_1e(enum hws_edge_compare edge,
			    enum search_element element1, enum hws_search_dir search_dir1,
			    u32 step1, u32 init_val1, u32 end_val1,
			    enum search_element element2, enum hws_search_dir search_dir2,
			    u32 step2, u32 init_val2, u32 end_val2, u16 byte_num)
{
	int result = (edge == EDGE_PF) ? 0 : 1; /* start from 0 (pass), if pass */
	int sign_step1 = (search_dir1 == HWS_LOW2HIGH) ? step1 : -step1;
	int param1 = (int)init_val1;
	u32 steps_num1 = (search_dir1 == HWS_LOW2HIGH) ? (end_val1 - init_val1) / step1 :
							 (init_val1 - end_val1) / step1;
	int sign_step2 = (search_dir2 == HWS_LOW2HIGH) ? step2 : -step2;
	int param2 = (int)init_val2;
	u32 steps_num2 = (search_dir2 == HWS_LOW2HIGH) ? (end_val2 - init_val2) / step2 :
							 (init_val2 - end_val2) / step2;
	u32 steps_num = (steps_num1 < steps_num2) ? steps_num1 : steps_num2;
	u32 step_idx = 0;
	u32 reg_addr1 = CRX_PHY_BASE;
	reg_addr1 += (4 * effective_cs);

#ifdef DBG_PRINT
	printf("%s: byte_num %d, param1 %d, param2 %d\n", __func__, byte_num, param1, param2);
#endif

	for (step_idx = 0; step_idx <= steps_num; step_idx++) {
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
				   VREF_BCAST_PHY_REG(effective_cs), param2);
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
				   VREF_PHY_REG(effective_cs, 4), param2);
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
				   VREF_PHY_REG(effective_cs, 5), param2);
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
				   reg_addr1, param1);

		/* reset phy fifo pointer after rx adll or rx cal change */
		ddr3_tip_reset_fifo_ptr(0);

		if (byte_num == 8)
			reg_write(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG, 0x0);

		result = dma_test(dma_src, dma_dst, effective_cs);

		if (byte_num == 8)
			result = reg_read(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG);

#ifdef DBG_PRINT
		printf("%s: param1 %d, param2 %d -> %d\n", __func__, param1, param2, result);
#endif
		if (((result != 0) && (edge == EDGE_PF)) || ((result == 0) && (edge == EDGE_FP))) {
#ifdef DBG_PRINT
			printf("%s: param1 %d, param2 %d\n", __func__, param1, param2);
#endif
			return step_idx;
		}

		param1 += sign_step1;
		param2 += sign_step2;
	}

	return 255; /* search on positive numbers; -1 represents fail */
}

static int xor_search_2d_2e(enum hws_edge_compare edge,
			    enum search_element element1, enum hws_search_dir search_dir1,
			    u32 step1, u32 init_val1, u32 end_val1,
			    enum  search_element element2, enum hws_search_dir search_dir2,
			    u32 step2, u32 init_val2, u32 end_val2,
			    u16 byte_num, u8 *vw_vector)
{
	u32 reg_addr = CRX_PHY_BASE;
	u32 reg_data1, reg_data2;
	reg_addr += (4 * effective_cs);
	vw_vector[0] = end_val1;
	vw_vector[1] = init_val1;
	/* read nominal search element */
	ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
			  VREF_BCAST_PHY_REG(effective_cs), &reg_data2);
	ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
			  CRX_PHY_REG(effective_cs), &reg_data1);

#ifdef DBG_PRINT
	printf("%s: byte_num %d, nominal %d, %d\n", __func__, byte_num, reg_data1, reg_data2);
#endif
	vw_vector[0] = xor_search_2d_1e(EDGE_FP, element1, search_dir1, step1, init_val1, end_val1,
					element2, search_dir2, step2, init_val2, end_val2, byte_num);
	search_dir2 = (search_dir2 == HWS_LOW2HIGH) ? HWS_HIGH2LOW : HWS_LOW2HIGH;
	search_dir1 = (search_dir1 == HWS_LOW2HIGH) ? HWS_HIGH2LOW : HWS_LOW2HIGH;
	ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
			  VREF_BCAST_PHY_REG(effective_cs), &reg_data2);
	ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
			  CRX_PHY_REG(effective_cs), &reg_data1);
#ifdef DBG_PRINT
	printf("%s: byte_num %d, nominal %d, %d\n", __func__, byte_num, reg_data1, reg_data2);
#endif
	vw_vector[1] = xor_search_2d_1e(EDGE_FP, element1, search_dir1, step1, end_val1, init_val1,
					element2, search_dir2, step2, end_val2, init_val2, byte_num);
	ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
			   VREF_BCAST_PHY_REG(effective_cs), reg_data2);
	ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
			   VREF_PHY_REG(effective_cs, 4), reg_data2);
	ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
			   VREF_PHY_REG(effective_cs, 5), reg_data2);
	ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, byte_num, DDR_PHY_DATA,
			   CRX_PHY_REG(effective_cs), reg_data1);

	if ((vw_vector[1] == 255) || (vw_vector[0] == 255))
		return 255;
	else
		return vw_vector[1] - vw_vector[0];

	return 0;
}

#if defined(DBG_CPU_SWEEP_TEST)
/*
 * repeat:	number of test repetitions
 * test_len:	length of the memory test; min is 0x10
 * dir:		0 - tx, 1 - rx
 * mode:	opt; subphy num or 0xff for all subphys
 */
static int cpu_sweep_test(u32 repeat, u32 dir, u32 mode)
{
	/*
	 * test_len:	0 - CPU only,
	 *		1 - xor 128M at 128K,
	 *		2 - partial write 1M at 1K + 1/2/3,
	 *		3 - xor 1G at 16M
	 */

	u32 sphy = 0, start_sphy = 0, end_sphy = 0, duty_cycle = 0;
	u32 adll = 0;
	u32 res = 0;
	u32 vector_vtap[MAX_BUS_NUM] = {0};
	int duty_cycle_idx;
	u32 adll_value = 0;
	int reg = (dir == 0) ? CTX_PHY_BASE : CRX_PHY_BASE;
	int step = 3;
	int max_v = (dir == 0) ? 73 : 63;
	enum hws_access_type sphy_access;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	u32 adll_reg_val, vtap_reg_val;
	u32 mux_reg[MAX_INTERFACE_NUM];

	if (mode == 0xff) {
		/* per sphy */
		start_sphy = 0;
		end_sphy = octets_per_if_num - 1;
		sphy_access = ACCESS_TYPE_UNICAST;
	} else if (mode == 0x1ff) {
		/* per sphy */
		start_sphy = 0;
		end_sphy = octets_per_if_num;
		sphy_access = ACCESS_TYPE_UNICAST;
	} else { /* TODO: need to check mode for valid sphy number */
		start_sphy = mode;
		end_sphy = mode + 1;
		sphy_access = ACCESS_TYPE_UNICAST;
	}

	if (mv_ddr_is_ecc_ena() && (start_sphy != 8))
		reg_bit_clrset(MC6_BASE + MC6_RAS_CTRL_REG, 0x0 << ECC_EN_OFFS, ECC_EN_MASK << ECC_EN_OFFS);

	/* choose cs for tip controller */
	ddr3_tip_if_write(0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ODPG_DATA_CTRL_REG,
			  effective_cs << ODPG_DATA_CS_OFFS,
			  ODPG_DATA_CS_MASK << ODPG_DATA_CS_OFFS);

	ddr3_tip_if_read(0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DUAL_DUNIT_CFG_REG,
				mux_reg, MASK_ALL_BITS);

	for (sphy = start_sphy; sphy < end_sphy; sphy++) {
		if (dir == 1) { /* dir 0 saved in dq_vref_vec array */
			ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, sphy, DDR_PHY_DATA,
					  VREF_BCAST_PHY_REG(effective_cs), &vtap_reg_val);
			vector_vtap[sphy] = vtap_reg_val;
		}

		/* save adll position and voltage */
		ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, start_sphy, DDR_PHY_DATA,
				  reg + effective_cs * 0x4, &adll_reg_val);
		printf("byte %d, nominal adll %d, rc %d\n",
		       sphy, adll_reg_val, vtap_reg_val);
		for (duty_cycle = 0; duty_cycle < max_v; duty_cycle += step) {
			if (dir == 0) {
				/*set mux to d-unit*/
				ddr3_tip_if_write(0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DUAL_DUNIT_CFG_REG,
						1 << 6, 1 << 6);
				duty_cycle_idx = duty_cycle;
				/* insert dram to vref training mode */
				mv_ddr4_vref_training_mode_ctrl(0, 0, ACCESS_TYPE_MULTICAST, 1);
				/* set new vref training value in dram */
				mv_ddr4_vref_tap_set(0, 0, ACCESS_TYPE_MULTICAST, duty_cycle_idx,
						     MV_DDR4_VREF_TAP_START);
				/* close vref range */
				mv_ddr4_vref_tap_set(0, 0, ACCESS_TYPE_MULTICAST, duty_cycle_idx,
						     MV_DDR4_VREF_TAP_END);
				/* set mux to mc6 */
				ddr3_tip_if_write(0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DUAL_DUNIT_CFG_REG,
						mux_reg[0], MASK_ALL_BITS);

			} else {
				/* set new receiver dc training value in dram */
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
						    VREF_BCAST_PHY_REG(effective_cs), duty_cycle);
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
						   VREF_PHY_REG(effective_cs, 4), duty_cycle);
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
						   VREF_PHY_REG(effective_cs, 5), duty_cycle);
			}

			for (adll = 0; adll < ADLL_LENGTH; adll++) {
				res = 0;
				adll_value = (dir == 0) ? (adll * 2) : adll;
				ddr3_tip_bus_write(0, ACCESS_TYPE_MULTICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
						   reg + effective_cs * 0x4, adll_value);

				if (sphy == 8)
					reg_write(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG, 0x0);

				res = dma_test(dma_src, dma_dst, effective_cs);

				if (sphy == 8)
					res = reg_read(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG);

				if (dir == 0) {
					if (((dq_vref_vec[sphy] <= duty_cycle) &&
					     (dq_vref_vec[sphy] >= (duty_cycle - step))) &&
					    (adll_reg_val >= adll_value) &&
					    (adll_reg_val <= (adll_value + 2))) {
						res = -1;
					}
				} else {
					if (((vtap_reg_val <= duty_cycle) &&
					     (vtap_reg_val >= (duty_cycle - step))) &&
					    (adll_reg_val == adll)) {
						res = -1;
					}
				}
				printf("%d, ", res);
			}
			printf("\n");
		} /* end of dutye cycle */
		if (dir == 1) {
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
					   VREF_BCAST_PHY_REG(effective_cs), vector_vtap[sphy]);
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
					   VREF_PHY_REG(effective_cs, 4), vector_vtap[sphy]);
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
					   VREF_PHY_REG(effective_cs, 5), vector_vtap[sphy]);
		} else {
			/*set mux to d-unit*/
			ddr3_tip_if_write(0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DUAL_DUNIT_CFG_REG,
					1 << 6, 1 << 6);
			duty_cycle_idx = duty_cycle;
			/* insert dram to vref training mode */
			mv_ddr4_vref_training_mode_ctrl(0, 0, ACCESS_TYPE_MULTICAST, 1);
			/* set new vref training value in dram */
			mv_ddr4_vref_tap_set(0, 0, ACCESS_TYPE_MULTICAST, dq_vref_vec[sphy], MV_DDR4_VREF_TAP_START);
			/* close vref range*/
			mv_ddr4_vref_tap_set(0, 0, ACCESS_TYPE_MULTICAST, dq_vref_vec[sphy],  MV_DDR4_VREF_TAP_END);
			/* insert dram to vref training mode */
			mv_ddr4_vref_training_mode_ctrl(0, 0, ACCESS_TYPE_MULTICAST, 0);
			/* set mux to mc6 */
			ddr3_tip_if_write(0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DUAL_DUNIT_CFG_REG,
						 mux_reg[0], MASK_ALL_BITS);
		}
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
				  reg + effective_cs * 0x4, adll_reg_val);
	}

	/* rewrite adll and voltage nominal values */
	ddr3_tip_bus_write(0, ACCESS_TYPE_MULTICAST, 0, sphy_access, end_sphy - 1, DDR_PHY_DATA,
			   reg + effective_cs * 0x4, adll_reg_val);
	if (mv_ddr_is_ecc_ena() && (start_sphy != 8))
		reg_bit_clrset(MC6_BASE + MC6_RAS_CTRL_REG, 0x1 << ECC_EN_OFFS, ECC_EN_MASK << ECC_EN_OFFS);

	ddr3_tip_reset_fifo_ptr(0);


	return 0;
}
#endif /* DBG_CPU_SWEEP_TEST */

static int vertical_adjust(int ena_mask, u8 byte_num)
{
	u8 sphy = 0, start_sphy = 0, end_sphy = 0;
	u8 vw[2];
	int print_ena = 0;
	int step = (ena_mask == 0) ? 2 : 4;
	u8 opt_rc = 0;
	int fpf;
	int result;
	enum hws_access_type sphy_access;
	u32 reg_data = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	uint64_t xor_byte_mask = 0;
	uint64_t curr_dst;
	uint64_t val;
	int i, j;


	if (byte_num == 0xf) { /* 0xf - all bytes */
		start_sphy = 0;
		end_sphy = octets_per_if_num - 1;
	} else {
		start_sphy = byte_num;
		end_sphy = byte_num + 1;
	}
	sphy_access = ACCESS_TYPE_UNICAST;

	ddr3_tip_if_write(0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ODPG_DATA_CTRL_REG,
			  effective_cs << ODPG_DATA_CS_OFFS, ODPG_DATA_CS_MASK << ODPG_DATA_CS_OFFS);

	for (sphy = start_sphy; sphy < end_sphy; sphy++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sphy);
			ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, sphy, DDR_PHY_DATA,
					  VREF_BCAST_PHY_REG(effective_cs), &reg_data);
		if (print_ena == 1) {

#ifdef DBG_PRINT
			printf("cs[%d]: byte %d rc %d [%d,%d] ->",
			       effective_cs, sphy, reg_data, rx_eye_lo_lvl[sphy], rx_eye_hi_lvl[sphy]);
#endif
		}
		if (ena_mask == PER_IF) {
			xor_byte_mask = 0;
		} else if (ena_mask == PER_BYTE_RES1) {
			xor_byte_mask = (unsigned long long)0x0 | ((unsigned long long)0xff << (sphy * 8));
			xor_byte_mask = ~xor_byte_mask;
		} else if (ena_mask == PER_BYTE_RES0) {
			xor_byte_mask = (unsigned long long)0x0 | ((unsigned long long)0xff << (sphy * 8));
		} else if (ena_mask == PER_DQ0) { /* only dq0 toggles */
			xor_byte_mask = (unsigned long long)0x0 | ((unsigned long long)0x1 << (sphy * 8));
			xor_byte_mask = ~xor_byte_mask;
		}


		for (j = 0; j < DBG_DMA_ENG_NUM; j++) {
			/* build dma data unit from dma pattern and write to dram */
			for (i = 0, curr_dst = dma_src[effective_cs][j];
				 i < DBG_DMA_DATA_SIZE / sizeof(dma_pattern[0]);
				 i++, curr_dst += sizeof(dma_pattern[0])) {
				val = dma_pattern[i % (sizeof(dma_pattern) / sizeof(dma_pattern[0]))];
				writeq(curr_dst, (val | xor_byte_mask));
			}
		}


		if (byte_num == 8)
			reg_write(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG, 0x0);

		result = dma_test(dma_src, dma_dst, effective_cs);

		if (byte_num == 8)
			result = reg_read(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG);

		if (result == 0) {
			fpf = xor_search_1d_2e(EDGE_PF, step, rx_eye_lo_lvl[sphy], rx_eye_hi_lvl[sphy],
					       sphy, REC_CAL, vw);
		} else {
#ifdef DBG_PRINT
			printf("%s: failed on first try (fail to pass)\n", __func__);
#endif
			fpf = xor_search_1d_2e(EDGE_FP, step, rx_eye_lo_lvl[sphy], rx_eye_hi_lvl[sphy],
					       sphy, REC_CAL, vw);
		}

		if (fpf != 255) {
			opt_rc = (vw[1] + vw[0]) / 2;
		} else {
			if (print_ena == 1) {
#ifdef DBG_PRINT
				printf("%s: byte %d - no lock\n", __func__, sphy);
#endif
			}
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
					   VREF_BCAST_PHY_REG(effective_cs), reg_data);
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
					   VREF_PHY_REG(effective_cs, 4), reg_data);
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
					   VREF_PHY_REG(effective_cs, 5), reg_data);

			return 1;
		}

#ifdef DBG_PRINT
		printf("%s: opt rc %d, vw0 %d, vw1 %d\n", __func__, opt_rc, vw[0], vw[1]);
#endif
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
				   VREF_BCAST_PHY_REG(effective_cs), opt_rc);
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
				   VREF_PHY_REG(effective_cs, 4), opt_rc);
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
				   VREF_PHY_REG(effective_cs, 5), opt_rc);
	} /* end of sphy */

	ddr3_tip_reset_fifo_ptr(0);

	return 0;
}

static int horizontal_adjust(int ena_mask, u8 byte_num, u8 *valid_crx_matrix)
{
	u32 sphy = 0, start_sphy = 0, end_sphy = 0;
	u8 vw[2];
	int print_ena = 0;
	int step = (ena_mask == 0) ? 1 : 2;
	u8 opt_crx = 0;
	int fpf;
	int result;
	enum hws_access_type sphy_access;
	u32 reg_data;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	uint64_t xor_byte_mask = 0;
	uint64_t curr_dst;
	uint64_t val;
	int i, j;

	if (byte_num == 0xf) { /* 0xf - all bytes */
		start_sphy = 0;
		end_sphy = octets_per_if_num - 1;
	} else {
		start_sphy = byte_num;
		end_sphy = byte_num + 1;
	}
	sphy_access = ACCESS_TYPE_UNICAST;

	ddr3_tip_if_write(0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ODPG_DATA_CTRL_REG,
			  effective_cs << ODPG_DATA_CS_OFFS, ODPG_DATA_CS_MASK << ODPG_DATA_CS_OFFS);

	for (sphy = start_sphy; sphy < end_sphy; sphy++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sphy);
			ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, sphy, DDR_PHY_DATA,
					  CRX_PHY_REG(effective_cs), &reg_data);
		if (print_ena == 1) {

#ifdef DBG_PRINT
			printf("cs[%d]: byte %d crx %d ->", effective_cs, sphy, reg_data);
#endif
		}

		if (ena_mask == PER_IF) {
			xor_byte_mask = 0;
		} else if (ena_mask == PER_BYTE_RES1) {
			xor_byte_mask = (unsigned long long)0x0 | ((unsigned long long)0xff << (sphy * 8));
			xor_byte_mask = ~xor_byte_mask;
		} else if (ena_mask == PER_BYTE_RES0) {
			xor_byte_mask = (unsigned long long)0x0 | ((unsigned long long)0xff << (sphy * 8));
		} else if (ena_mask == PER_DQ0) { /* only dq0 toggles */
			xor_byte_mask = (unsigned long long)0x0 | ((unsigned long long)0x01 << (sphy * 8));
			xor_byte_mask = ~xor_byte_mask;
		}

		for (j = 0; j < DBG_DMA_ENG_NUM; j++) {
			/* build dma data unit from dma pattern and write to dram */
			for (i = 0, curr_dst = dma_src[effective_cs][j];
				 i < DBG_DMA_DATA_SIZE / sizeof(dma_pattern[0]);
				 i++, curr_dst += sizeof(dma_pattern[0])) {
				val = dma_pattern[i % (sizeof(dma_pattern) / sizeof(dma_pattern[0]))];
				writeq(curr_dst, (val | xor_byte_mask));
			}
		}

		if (sphy == 8)
			reg_write(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG, 0x0);

		result = dma_test(dma_src, dma_dst, effective_cs);

		if (sphy == 8)
			result = reg_read(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG);

		if (result == 0) {
			fpf = xor_search_1d_2e(EDGE_PF, step, 0, 31, sphy, CRX, vw);
		} else {
#ifdef DBG_PRINT
			printf("%s: failed on first try (fail to pass)\n", __func__);
#endif
			fpf = xor_search_1d_2e(EDGE_FP, step, 0, 31, sphy, CRX, vw);
		}

		if (fpf != 255) {
			opt_crx = (vw[1] + vw[0]) / 2;
			valid_crx_matrix[effective_cs * MAX_BUS_NUM * 2 + 2 * sphy] = vw[0];
			valid_crx_matrix[effective_cs * MAX_BUS_NUM * 2 + 2 * sphy + 1] = vw[1];
		} else {
			if (print_ena == 1) {
#ifdef DBG_PRINT
				printf("%s: byte %d - no lock\n", __func__, sphy);
#endif
			}

			return 1;
		}

#ifdef DBG_PRINT
		printf("%s: opt crx %d, vw0 %d, vw1 %d\n", __func__, opt_crx, vw[0], vw[1]);
#endif
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
				   CRX_PHY_REG(effective_cs), opt_crx);
	} /* end of sphy */

	ddr3_tip_reset_fifo_ptr(0);

	return 0;
}

static int diagonal_adjust(int ena_mask, u8 byte_num)
{
	u32 sphy = 0, start_sphy = 0, end_sphy = 0;
	u32 nom_adll = 0, nom_rc = 0, tmp_nom_adll = 0, tmp_nom_rc = 0;
	u32 step1, init_val1, end_val1, step2, init_val2, end_val2;
	u8 vw[2];
	enum hws_access_type sphy_access;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	int step;
	int final;
	uint64_t xor_byte_mask = 0;

	if (byte_num == 0xf) { /* 0xf - all bytes */
		start_sphy = 0;
		end_sphy = octets_per_if_num - 1;
	} else {
		start_sphy = byte_num;
		end_sphy = byte_num + 1;
	}
	sphy_access = ACCESS_TYPE_UNICAST;

	ddr3_tip_if_write(0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ODPG_DATA_CTRL_REG,
			  effective_cs << ODPG_DATA_CS_OFFS, ODPG_DATA_CS_MASK << ODPG_DATA_CS_OFFS);

	for (sphy = start_sphy; sphy < end_sphy; sphy++) {
		if (ena_mask == PER_IF) {
			xor_byte_mask = 0;
		} else if (ena_mask == PER_BYTE_RES1) {
			xor_byte_mask = (unsigned long long)0x0 | ((unsigned long long)0xff << (sphy * 8));
			xor_byte_mask = ~xor_byte_mask;
		} else if (ena_mask == PER_BYTE_RES0) {
			xor_byte_mask = (unsigned long long)0x0 | ((unsigned long long)0xff << (sphy * 8));
		} else if (ena_mask == PER_DQ0) { /* only dq0 toggles */
			xor_byte_mask = (unsigned long long)0x0 | ((unsigned long long)0x01 << (sphy * 8));
			xor_byte_mask = ~xor_byte_mask;
		}

		if (sphy == 8) {
#ifdef DBG_PRINT
			printf("%s: ecc byte: xor byte mask 0x%llx\n", __func__, xor_byte_mask);
#endif
		}

		/* read nominal start point */
		ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, sphy, DDR_PHY_DATA,
				  CRX_PHY_REG(effective_cs), &nom_adll);
		ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, sphy, DDR_PHY_DATA,
				  VREF_BCAST_PHY_REG(effective_cs), &nom_rc);
		/* define slope of the search */
		step1 = 1; /* adll step */
		step2 = 3; /* rc step */
		/*
		 * diag search limits are unknown
		 * the limits will be takne per adll-rc plane limits
		 * the step is the minimum between adll and rc steps
		 */
		step = ((nom_adll / step1) > (nom_rc / step2)) ? (nom_rc / step2) : (nom_adll / step1);
		init_val1 = nom_adll - step * step1;
		init_val2 = nom_rc - step * step2;
		step = (((31 - nom_adll) / step1) > ((63 - nom_rc) / step2)) ?
		       ((63 - nom_rc) / step2) : ((31 - nom_adll) / step1);
		end_val1 = nom_adll + step * step1;
		end_val2 = nom_rc + step * step2;
		if (ena_mask == 0) {
			xor_byte_mask = 0;
		} else {
			xor_byte_mask = (unsigned long long)0x0 | ((unsigned long long)0xff << (sphy * 8));
#ifdef DBG_PRINT
			printf("%s: xor byte mask = 0x%llx\n", __func__, xor_byte_mask);
#endif
			xor_byte_mask = ~xor_byte_mask;
#ifdef DBG_PRINT
			printf("%s: xor byte mask = 0x%llx\n", __func__, xor_byte_mask);
#endif
		}
		final = xor_search_2d_2e(EDGE_FPF, CRX,
					 HWS_LOW2HIGH, step1, init_val1, end_val1, REC_CAL,
					 HWS_LOW2HIGH, step2, init_val2, end_val2, sphy, vw);

		if (final != 255) {
			/* vw contains a number of steps done per search */
			tmp_nom_adll = ((vw[0] * step1 + init_val1) + (end_val1 - vw[1] * step1)) / 2;
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
					   CRX_PHY_REG(effective_cs), tmp_nom_adll);
			tmp_nom_rc = ((vw[0] * step2 + init_val2) + (end_val2 - vw[1] * step2)) / 2;
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
					   VREF_BCAST_PHY_REG(effective_cs), tmp_nom_rc);
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
					   VREF_PHY_REG(effective_cs, 4), tmp_nom_rc);
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
					   VREF_PHY_REG(effective_cs, 5), tmp_nom_rc);
#ifdef DBG_PRINT
			printf("cs[%d]: byte %d: first xor_search_2d_2e [%d, %d]->[%d,%d]\n",
			       effective_cs, sphy, nom_adll, nom_rc, tmp_nom_adll, tmp_nom_rc);
#endif
		} else {
			printf("%s: cs[%d]: byte %d: first xor_search_2d_2e failed\n",
			       __func__, effective_cs, sphy);
		}
	}

	for (sphy = start_sphy; sphy < end_sphy; sphy++) {
		ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, sphy, DDR_PHY_DATA,
				  CRX_PHY_REG(effective_cs), &nom_adll);
		ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, sphy, DDR_PHY_DATA,
				  VREF_BCAST_PHY_REG(effective_cs), &nom_rc);
		step1 = 1;
		step2 = 3;
		step = ((nom_adll / step1) > ((63 - nom_rc) / step2)) ? ((63 - nom_rc) / step2) : (nom_adll / step1);
		init_val1 = nom_adll - step * step1;
		init_val2 = nom_rc + step * step2;
		step = (((31 - nom_adll) / step1) > (nom_rc / step2)) ? (nom_rc / step2) : ((31 - nom_adll) / step1);
		end_val1 = nom_adll + step * step1;
		end_val2 = nom_rc - step * step2;

		if (ena_mask == 0) {
			xor_byte_mask = 0;
		} else {
			xor_byte_mask = (unsigned long long)0x0 | ((unsigned long long)0xff << (sphy * 8));
#ifdef DBG_PRINT
			printf("%s: xor byte mask = 0x%llx\n", __func__, xor_byte_mask);
#endif
			xor_byte_mask = ~xor_byte_mask;
#ifdef DBG_PRINT
			printf("%s: xor byte mask = 0x%llx\n", __func__, xor_byte_mask);
#endif
		}
		final = xor_search_2d_2e(EDGE_FPF, CRX,
					 HWS_LOW2HIGH, step1, init_val1, end_val1, REC_CAL,
					 HWS_HIGH2LOW, step2, init_val2, end_val2, sphy, vw);
#ifdef DBG_PRINT
		printf("%s: xor_search_2d_2e solution: %d [%d, %d]\n",
		       __func__, final, vw[0], vw[1]);
#endif
		if (final != 255) {
			tmp_nom_adll = ((vw[0] * step1 + init_val1) + (end_val1 - vw[1] * step1)) / 2;
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
					   CRX_PHY_REG(effective_cs), tmp_nom_adll);
			tmp_nom_rc = ((init_val2 - vw[0] * step2) + (end_val2 + vw[1] * step2)) / 2;
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
					   VREF_BCAST_PHY_REG(effective_cs), tmp_nom_rc);
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
					   VREF_PHY_REG(effective_cs, 4), tmp_nom_rc);
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, sphy_access, sphy, DDR_PHY_DATA,
					   VREF_PHY_REG(effective_cs, 5), tmp_nom_rc);
#ifdef DBG_PRINT
			printf("cs[%d]: byte %d: second xor_search_2d_2e [%d, %d]->[%d, %d]\n",
			       effective_cs, sphy, nom_adll, nom_rc, tmp_nom_adll, tmp_nom_rc);
#endif
		} else {
			printf("%s: cs[%d]: byte %d: second xor_search_2d_2e failed\n",
			       __func__, effective_cs, sphy);
		}
	}

	ddr3_tip_reset_fifo_ptr(0);

	return 0;
}

static int rx_adjust(u8 *valid_crx_matrix)
{
	int dbg_flag = 1;
	enum mask_def ena_mask = PER_BYTE_RES1;
	enum search_type search = VERTICAL;
	int result = 1;
	int alg_loop = 0;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	int start_byte = 0;
	int end_byte = octets_per_if_num;
	int byte;

	reg_bit_clrset(MC6_BASE + MC6_RAS_CTRL_REG, 0x0 << ECC_EN_OFFS, ECC_EN_MASK << ECC_EN_OFFS);
	reg_write(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG, 0x0);
	result = dma_test(dma_src, dma_dst, effective_cs);


#ifdef DBG_PRINT
		printf("%s: going to v-h first level search\n", __func__);
#endif
		for (byte = start_byte; byte < end_byte; byte++) {
			if (mv_ddr_is_ecc_ena() && (byte != 8))
				reg_bit_clrset(MC6_BASE + MC6_RAS_CTRL_REG,
						0x0 << ECC_EN_OFFS, ECC_EN_MASK << ECC_EN_OFFS);

			search = VERTICAL;
			for (alg_loop = 0; alg_loop < 4; alg_loop++) {
				if (search == VERTICAL)
					result = vertical_adjust(ena_mask, byte);
				else
					result = horizontal_adjust(ena_mask, byte, valid_crx_matrix);

				switch (alg_loop) {
				case 0:
					if (result == 0) { /* pass; go to horizontal search */
						search = HORIZONTAL;
						if (dbg_flag == 2) {
#ifdef DBG_PRINT
							printf("%s: byte %d, alg loop %d: pass\n",
							       __func__, byte, alg_loop);
#endif
						}
						alg_loop = 2;
						continue;
					} else { /* failed; try another mask */
						ena_mask = PER_BYTE_RES0;
						if (dbg_flag == 2) {
#ifdef DBG_PRINT
							printf("%s: byte %d, alg loop %d: failed\n",
							       __func__, byte, alg_loop);
#endif
						}
					}
					break;
				case 1:
					if (result == 0) { /* pass; go to horizontal search */
						search = HORIZONTAL;
						ena_mask = PER_BYTE_RES1;
						if (dbg_flag == 2) {
#ifdef DBG_PRINT
							printf("%s: byte %d, alg loop %d: pass\n",
							       __func__, byte, alg_loop);
#endif
						}
						alg_loop = 2;
						continue;
					} else { /* still failed; try horizontal search first */
						search = HORIZONTAL;
						ena_mask = PER_BYTE_RES1;
						if (dbg_flag == 2) {
#ifdef DBG_PRINT
							printf("%s: byte %d, alg loop %d: failed\n",
							       __func__, byte, alg_loop);
#endif
						}
					}
					break;
				case 2:
					if (result == 0) { /* pass; go to vertical search */
						search = VERTICAL;
						if (dbg_flag == 2) {
#ifdef DBG_PRINT
							printf("%s: byte %d, alg loop %d: pass\n",
							       __func__, byte, alg_loop);
#endif
						}
						alg_loop = 2;
						continue;
					} else { /* fail */
						printf("%s: byte %d, alg loop %d: failed; no coverage\n",
						       __func__, byte, alg_loop);
						return MV_FAIL;
					}
					break;
				case 3:
					if (result == 0) { /* passed vertical and horizontal searches */
						/* pass; got to next byte */
						if (dbg_flag == 2) {
#ifdef DBG_PRINT
							printf("%s: byte %d, alg loop %d: pass\n",
							       __func__, byte, alg_loop);
#endif
						}
					} else { /* fail */
						printf("%s: byte %d, alg loop %d: failed; no coverage\n",
						       __func__, byte, alg_loop);
						return MV_FAIL;
					}
					break;
				} /* end of switch */
			} /* end of alg loop */
			if (mv_ddr_is_ecc_ena() && (byte != 8))
				reg_bit_clrset(MC6_BASE + MC6_RAS_CTRL_REG,
						0x1 << ECC_EN_OFFS, ECC_EN_MASK << ECC_EN_OFFS);
		} /* end of byte_num loop */

#ifdef DBG_PRINT
	printf("%s: going to v-h second level search\n", __func__);
#endif

	for (byte = start_byte; byte < end_byte; byte++) {

		if (mv_ddr_is_ecc_ena() && (byte != 8))
			reg_bit_clrset(MC6_BASE + MC6_RAS_CTRL_REG, 0x0 << ECC_EN_OFFS, ECC_EN_MASK << ECC_EN_OFFS);

		reg_write(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG, 0x0);

		ena_mask = PER_IF;
		result = horizontal_adjust(ena_mask, byte, valid_crx_matrix);
		if (result != 0) {
#ifdef DBG_PRINT
			printf("%s: cs[%d]: byte %d, second level of horizontal adjust failed\n",
			       __func__, effective_cs, byte);
#endif
		}
		result = vertical_adjust(ena_mask, byte);
		if (result != 0) {
#ifdef DBG_PRINT
			printf("%s: cs[%d]: byte %d, second level of vertical adjust failed\n",
			       __func__, effective_cs, byte);
#endif
		}
		if (mv_ddr_is_ecc_ena() && (byte != 8))
			reg_bit_clrset(MC6_BASE + MC6_RAS_CTRL_REG, 0x1 << ECC_EN_OFFS, ECC_EN_MASK << ECC_EN_OFFS);
	}

	ena_mask = PER_IF;
#ifdef DBG_PRINT
	printf("%s: going to diagonal search\n", __func__);
#endif
	for (byte = start_byte; byte < end_byte; byte++) {

		if (mv_ddr_is_ecc_ena() && (byte != 8))
			reg_bit_clrset(MC6_BASE + MC6_RAS_CTRL_REG, 0x0 << ECC_EN_OFFS, ECC_EN_MASK << ECC_EN_OFFS);

		reg_write(MC6_BASE + MC6_CH0_ECC_1BIT_ERR_COUNTER_REG, 0x0);

		diagonal_adjust(ena_mask, byte);

		if (mv_ddr_is_ecc_ena() && (byte != 8))
			reg_bit_clrset(MC6_BASE + MC6_RAS_CTRL_REG, 0x1 << ECC_EN_OFFS, ECC_EN_MASK << ECC_EN_OFFS);
	}

	return 0;
}



int mv_ddr_validate(void)
{
	uint64_t curr_dst;
	uint64_t val;
	int i, j;
	u32 cs, sphy;
	unsigned int max_cs = mv_ddr_cs_num_get();
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	int soc_ver_id;
	u8 valid_crx_matrix[DBG_MAX_CS_NUM * MAX_BUS_NUM * 2] = {0};
	u32 new_vref = 0;
	struct tsen_config *tsen = marvell_thermal_config_get();
	int cdeg; /* temperature in celsius degrees */
	u32 corr;
	u32 curr_vref = 0;
	u32 curr_crx = 0;
	u32 new_crx = 0;

	soc_ver_id = mv_ddr_get_soc_revision_id();

	if (max_cs > DBG_MAX_CS_NUM) {
		printf("mv_ddr: error: DBG_MAX_CS_NUM limit (%d) reached\n",
		       DBG_MAX_CS_NUM);
		return 1; /* fail */
	}

	for (cs = 0; cs < max_cs; cs++) {
		dma_offs_config(dma_src, dma_dst, cs);

		for (j = 0; j < DBG_DMA_ENG_NUM; j++) {
			/* build dma data unit from dma pattern and write to dram */
			for (i = 0, curr_dst = dma_src[cs][j];
				 i < DBG_DMA_DATA_SIZE / sizeof(dma_pattern[0]);
				 i++, curr_dst += sizeof(dma_pattern[0])) {
				val = dma_pattern[i % (sizeof(dma_pattern) / sizeof(dma_pattern[0]))];
				writeq(curr_dst, val);
			}
		}
		/* copy dma data unit from src to dst */
		mv_ddr_dma_memcpy(dma_src[cs], dma_dst[cs], DBG_DMA_DATA_SIZE, DBG_DMA_ENG_NUM, DBG_DMA_DESC_NUM);
	}

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		/* vref adjust stage prior to rx_adjust call */
		for (sphy = 0; sphy < octets_per_if_num; sphy++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sphy);
			if (new_vref_calc(rx_eye_lo_lvl[sphy], rx_eye_hi_lvl[sphy], &new_vref)) {
				printf("new vref value calculation failed\n");
				return 1; /* fail */
			}
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, sphy,
					   DDR_PHY_DATA, VREF_BCAST_PHY_REG(effective_cs), new_vref);
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, sphy,
					   DDR_PHY_DATA, VREF_PHY_REG(effective_cs, 4), new_vref);
			ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, sphy,
					   DDR_PHY_DATA, VREF_PHY_REG(effective_cs, 5), new_vref);
		}

		/* rx adjust depends on effective_cs global var */
		rx_adjust(valid_crx_matrix);

		if (soc_ver_id != CHIP_VER_7K_B0 && soc_ver_id != CHIP_VER_8K_B0) {
			/* temperature adjustment stage after rx_adjust call */
			marvell_thermal_init(tsen);

			if (marvell_thermal_read(tsen, &cdeg)) {
				printf("temperature read failed\n");
				return 1; /* fail */
			}

#ifdef DBG_PRINT
			printf("%s: tsen val %d\n", __func__, cdeg);
#endif

			/* apply vertical and horizontal fix to the post rx_adjust sampling point */
			for (sphy = 0; sphy < octets_per_if_num; sphy++) {
				VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sphy);
				/* vref correction */
				ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, sphy, DDR_PHY_DATA,
						  VREF_BCAST_PHY_REG(effective_cs), &curr_vref);
				corr = vref_temp_corr_calc(cdeg);
				new_vref = (curr_vref >= corr) ? (curr_vref - corr) : 0;

				/* write new value to register */
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, sphy,
						   DDR_PHY_DATA, VREF_BCAST_PHY_REG(effective_cs), new_vref);
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, sphy,
						   DDR_PHY_DATA, VREF_PHY_REG(effective_cs, 4), new_vref);
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, sphy,
						   DDR_PHY_DATA, VREF_PHY_REG(effective_cs, 5), new_vref);

				/* crx correction */
				ddr3_tip_bus_read(0, 0, ACCESS_TYPE_UNICAST, sphy, DDR_PHY_DATA,
						  CRX_PHY_REG(effective_cs), &curr_crx);
				corr = crx_temp_corr_calc(cdeg);
				if ((curr_crx + corr) < MAX_ADLL_RANGE)
					new_crx = curr_crx + corr;
				else
					new_crx = MAX_ADLL_RANGE - 1;
				ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST, sphy, DDR_PHY_DATA,
						   CRX_PHY_REG(effective_cs), new_crx);
			}
		} /*end soc version id*/
	}

#if defined(DBG_CPU_SWEEP_TEST)
	int repeat = 1;
	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		/* print out rx stats */
		for (sphy = 0;  sphy < octets_per_if_num; sphy++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sphy);
			cpu_sweep_test(repeat, 0x1, sphy);
		}
#if defined(DBG_CPU_SWEEP_TEST_TX)
		/* print out tx stats */
		for (sphy = 0; sphy < octets_per_if_num; sphy++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sphy);
			cpu_sweep_test(repeat, 0x0, sphy);
		}
#endif /* DBG_CPU_SWEEP_TEST_TX */
	}
#endif /* DBG_CPU_SWEEP_TEST */

#if defined(DBG_DMA_TEST)
	for (cs = 0; cs < max_cs; cs++) {
		mv_ddr_dma_memcpy(dma_src[cs], dma_dst[cs], DBG_DMA_DATA_SIZE, DBG_DMA_ENG_NUM, DBG_DMA_DESC_NUM);
		if (mv_ddr_dma_memcmp(dma_src[cs], dma_dst[cs], DBG_DMA_DATA_SIZE, DBG_DMA_ENG_NUM, DBG_DMA_DESC_NUM))
			printf("dma memcmp fail\n");
		else
			printf("dma memcmp pass\n");
	}
#endif /* DBG_DMA_TEST */
	return 0;
}
