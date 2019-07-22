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

#if defined(MV_DDR_ATF) /* MARVELL ATF */
#include "mv_ddr_atf_wrapper.h"
#endif

#include "mv_ddr_xor_v2.h"

/* dma engine registers */
#define DMA_DESQ_BALR_OFF		0x000
#define DMA_DESQ_BAHR_OFF		0x004
#define DMA_DESQ_SIZE_OFF		0x008
#define DMA_DESQ_DONE_OFF		0x00c
#define DMA_DESQ_DONE_PENDING_MASK	0x7fff
#define DMA_DESQ_DONE_PENDING_SHIFT	0
#define DMA_DESQ_DONE_READ_PTR_MASK	0x1fff
#define DMA_DESQ_DONE_READ_PTR_SHIFT	16
#define DMA_DESQ_ARATTR_OFF		0x010
#define DMA_DESQ_ATTR_CACHE_MASK	0x3f3f
#define DMA_DESQ_ATTR_OUTER_SHAREABLE	0x202
#define DMA_DESQ_ATTR_CACHEABLE		0x3c3c
#define DMA_IMSG_CDAT_OFF		0x014
#define DMA_IMSG_THRD_OFF		0x018
#define DMA_IMSG_THRD_MASK		0x7fff
#define DMA_IMSG_THRD_SHIFT		0x0
#define DMA_DESQ_AWATTR_OFF		0x01c
/* same flags as DMA_DESQ_ARATTR_OFF */
#define DMA_DESQ_ALLOC_OFF		0x04c
#define DMA_DESQ_ALLOC_WRPTR_MASK	0xffff
#define DMA_DESQ_ALLOC_WRPTR_SHIFT	16
#define DMA_IMSG_BALR_OFF		0x050
#define DMA_IMSG_BAHR_OFF		0x054
#define DMA_DESQ_CTRL_OFF		0x100
#define DMA_DESQ_CTRL_32B		1
#define DMA_DESQ_CTRL_128B		7
#define DMA_DESQ_STOP_OFF		0x800
#define DMA_DESQ_STOP_QUEUE_RESET_OFFS	1
#define DMA_DESQ_STOP_QUEUE_RESET_ENA	1
#define DMA_DESQ_STOP_QUEUE_DIS_OFFS	0
#define DMA_DESQ_STOP_QUEUE_DIS_ENA	0
#define DMA_DESQ_DEALLOC_OFF		0x804
#define DMA_DESQ_ADD_OFF		0x808

/* dma engine global registers */
#define GLOB_BW_CTRL				0x10004
#define GLOB_BW_CTRL_NUM_OSTD_RD_SHIFT		0
#define GLOB_BW_CTRL_NUM_OSTD_RD_VAL		8
#define GLOB_BW_CTRL_NUM_OSTD_WR_SHIFT		8
#define GLOB_BW_CTRL_NUM_OSTD_WR_VAL		8
#define GLOB_BW_CTRL_RD_BURST_LEN_SHIFT		12
#define GLOB_BW_CTRL_RD_BURST_LEN_VAL		4
#define GLOB_BW_CTRL_RD_BURST_LEN_VAL_Z1	2
#define GLOB_BW_CTRL_WR_BURST_LEN_SHIFT		16
#define GLOB_BW_CTRL_WR_BURST_LEN_VAL		4
#define GLOB_BW_CTRL_WR_BURST_LEN_VAL_Z1	2
#define GLOB_PAUSE				0x10014
#define GLOB_PAUSE_AXI_TIME_DIS_VAL		0x8
#define GLOB_SYS_INT_CAUSE			0x10200
#define GLOB_SYS_INT_MASK			0x10204
#define GLOB_MEM_INT_CAUSE			0x10220
#define GLOB_MEM_INT_MASK			0x10224
#define GLOB_SECURE				0x10300
#define GLOB_SECURE_SECURE			0
#define GLOB_SECURE_UNSECURE			1
#define GLOB_SECURE_SECURE_OFF			0
#define GLOB_SECURE_SECURE_MASK			0x1

#define MV_XOR_V2_MIN_DESC_SIZE	32
#define MV_XOR_V2_EXT_DESC_SIZE	128

#define MV_XOR_V2_DESC_RESERVED_SIZE		12
#define MV_XOR_V2_DESC_BUFF_D_ADDR_SIZE		12

#define MV_XOR_V2_CMD_LINE_NUM_MAX_D_BUF	8

/*
 * descriptors queue size;
 * 128 descriptors per dma engine is the current minimum
 * to support a few dma engines operation
 */
#define MV_XOR_V2_MAX_DESC_NUM			128

/* dma engine base address */
#define MV_XOR_BASE		0x00400000
#define MV_XOR_ENGINE(n)	(MV_XOR_BASE + (n) * 0x20000)
#define MV_XOR_ENGINE_NUM	4

#define MV_XOR_MAX_BURST_SIZE		4	/* 256B read or write transfers */
#define MV_XOR_MAX_BURST_SIZE_MASK	0xff
#define MV_XOR_MAX_TRANSFER_SIZE	((UINT32_MAX) & ~MV_XOR_MAX_BURST_SIZE_MASK)

enum mv_xor_v2_desc_op_mode {
	DESC_OP_MODE_NOP,		/* 0: idle operation */
	DESC_OP_MODE_MEMCPY,		/* 1: pure-dma operation */
	DESC_OP_MODE_MEMSET,		/* 2: mem-fill operation */
	DESC_OP_MODE_MEMINIT,		/* 3: mem-init operation */
	DESC_OP_MODE_MEMCMP,		/* 4: mem-compare operation */
	DESC_OP_MODE_CRC32,		/* 5: crc32 calculation */
	DESC_OP_MODE_XOR,		/* 6: raid 5 (xor) operation */
	DESC_OP_MODE_RAID6,		/* 7: raid 6 p&q-generation */
	DESC_OP_MODE_RAID6_REC		/* 8: raid 6 recovery */
};

/*
 * struct mv_xor_v2_hw_desc - dma hardware descriptor
 * @desc_id: used by software; not affected by hardware
 * @flags: error and status flags
 * @crc32_result: crc32 calculation result
 * @desc_ctrl: operation mode and control flags
 * @buff_size: amount of bytes to be processed
 * @fill_pattern_src_addr: fill-pattern or source-address and
 * aw-attributes
 */
struct mv_xor_v2_hw_desc {
	u16 desc_id;

/* definitions for flags */
#define DESC_BYTE_CMP_STATUS_OFFS	9
#define DESC_BYTE_CMP_STATUS_MASK	0x1
#define DESC_BYTE_CMP_STATUS_FAIL	0
#define DESC_BYTE_CMP_STATUS_OK		1

	u16 flags;
	u32 crc32_result;
	u32 desc_ctrl;

/* definitions for desc_ctrl */
#define DESC_BYTE_CMP_CRC_FIRST_SHIFT	19
#define DESC_BYTE_CMP_CRC_FIRST_ENA	1
#define DESC_BYTE_CMP_CRC_LAST_SHIFT	20
#define DESC_BYTE_CMP_CRC_LAST_ENA	1
#define DESC_NUM_ACTIVE_D_BUF_SHIFT	22
#define DESC_OP_MODE_SHIFT		28

	u32 buff_size;
	u32 fill_pattern_src_addr_lo;
	u32 fill_pattern_src_addr_hi;
	u32 fill_pattern_dst_addr_lo;
	u32 fill_pattern_dst_addr_hi;
};

enum {
	DMA_ENG_0,
	DMA_ENG_1,
	DMA_ENG_2,
	DMA_ENG_3
};

/* locate dma descriptors queue in sram with 256B alignment per hw requirement */
static struct mv_xor_v2_hw_desc qmem[MV_XOR_ENGINE_NUM][MV_XOR_V2_MAX_DESC_NUM] __aligned(0x100);

static void mv_xor_v2_init(u32 xor_id)
{
	u32 reg_val;
	u32 xor_base = MV_XOR_ENGINE(xor_id);

	/* set descriptor size to dma engine */
	reg_write(xor_base + DMA_DESQ_CTRL_OFF, DMA_DESQ_CTRL_32B);

	/* set descriptors queue size to dma engine */
	reg_write(xor_base + DMA_DESQ_SIZE_OFF, MV_XOR_V2_MAX_DESC_NUM);

	/*
	 * enable secure mode in dma engine xorg secure reg to align
	 * dma operations between sram and dram because of secure mode
	 * of sram at ble stage
	 */
	reg_bit_clrset(xor_base + GLOB_SECURE, GLOB_SECURE_SECURE << GLOB_SECURE_SECURE_OFF,
		       GLOB_SECURE_SECURE_MASK << GLOB_SECURE_SECURE_OFF);

	/* set descriptors queue address to dma engine */
	reg_write(xor_base + DMA_DESQ_BALR_OFF, (u32)((uint64_t)qmem[xor_id] & 0xffffffff));
	reg_write(xor_base + DMA_DESQ_BAHR_OFF, (u32)((uint64_t)qmem[xor_id] >> 32));

	/*
	 * set attributes for reading and writing data buffers to:
	 * - outer-shareable - snoops to be performed on cpu caches;
	 * - enable cacheable - bufferable, modifiable, other allocate, and allocate.
	 */
	reg_val = reg_read(xor_base + DMA_DESQ_ARATTR_OFF);
	reg_val &= ~DMA_DESQ_ATTR_CACHE_MASK;
	reg_val |= DMA_DESQ_ATTR_OUTER_SHAREABLE | DMA_DESQ_ATTR_CACHEABLE;
	reg_write(xor_base + DMA_DESQ_ARATTR_OFF, reg_val);

	reg_val = reg_read(xor_base + DMA_DESQ_AWATTR_OFF);
	reg_val &= ~DMA_DESQ_ATTR_CACHE_MASK;
	reg_val |= DMA_DESQ_ATTR_OUTER_SHAREABLE | DMA_DESQ_ATTR_CACHEABLE;
	reg_write(xor_base + DMA_DESQ_AWATTR_OFF, reg_val);

	/*
	 * bandwidth control to optimize dma performance:
	 * - set write/read burst lengths to maximum write/read transactions;
	 * - set outstanding write/read data requests to maximum value.
	 */
	reg_val = (GLOB_BW_CTRL_NUM_OSTD_RD_VAL << GLOB_BW_CTRL_NUM_OSTD_RD_SHIFT) |
		  (GLOB_BW_CTRL_NUM_OSTD_WR_VAL << GLOB_BW_CTRL_NUM_OSTD_WR_SHIFT) |
		  (MV_XOR_MAX_BURST_SIZE << GLOB_BW_CTRL_RD_BURST_LEN_SHIFT) |
		  (MV_XOR_MAX_BURST_SIZE << GLOB_BW_CTRL_WR_BURST_LEN_SHIFT);
	reg_write(xor_base + GLOB_BW_CTRL, reg_val);

	/* disable axi timer feature */
	reg_val = reg_read(xor_base + GLOB_PAUSE);
	reg_val |= GLOB_PAUSE_AXI_TIME_DIS_VAL;
	reg_write(xor_base + GLOB_PAUSE, reg_val);

	/* clear all previous pending interrupt indications, prior to using engine */
	reg_write(xor_base + GLOB_SYS_INT_CAUSE, 0);

	/* enable dma engine */
	reg_write(xor_base + DMA_DESQ_STOP_OFF, DMA_DESQ_STOP_QUEUE_DIS_ENA << DMA_DESQ_STOP_QUEUE_DIS_OFFS);
}

static void mv_xor_v2_desc_prep(u32 xor_id, u32 desc_id, enum mv_xor_v2_desc_op_mode op_mode,
				uint64_t src, uint64_t dst, uint64_t size, uint64_t data)
{
	struct mv_xor_v2_hw_desc *desc = &qmem[xor_id][desc_id];

	memset((void *)desc, 0, sizeof(*desc));
	desc->desc_id = desc_id;
	desc->buff_size = size;

	switch (op_mode) {
	case DESC_OP_MODE_MEMSET:
		desc->desc_ctrl = DESC_OP_MODE_MEMSET << DESC_OP_MODE_SHIFT;
		desc->fill_pattern_src_addr_lo = (u32)data;
		desc->fill_pattern_src_addr_hi = (u32)(data >> 32);
		desc->fill_pattern_dst_addr_lo = (u32)dst;
		desc->fill_pattern_dst_addr_hi = (u32)(dst >> 32);
		break;
	case DESC_OP_MODE_MEMCPY:
		desc->desc_ctrl = DESC_OP_MODE_MEMCPY << DESC_OP_MODE_SHIFT;
		desc->fill_pattern_src_addr_lo = (u32)src;
		desc->fill_pattern_src_addr_hi = (u32)(src >> 32);
		desc->fill_pattern_dst_addr_lo = (u32)dst;
		desc->fill_pattern_dst_addr_hi = (u32)(dst >> 32);
		break;
	case DESC_OP_MODE_MEMCMP:
		desc->desc_ctrl = (DESC_OP_MODE_MEMCMP << DESC_OP_MODE_SHIFT) |
				  (DESC_BYTE_CMP_CRC_FIRST_ENA << DESC_BYTE_CMP_CRC_FIRST_SHIFT) |
				  (DESC_BYTE_CMP_CRC_LAST_ENA << DESC_BYTE_CMP_CRC_LAST_SHIFT);
		desc->fill_pattern_src_addr_lo = (u32)src;
		desc->fill_pattern_src_addr_hi = (u32)(src >> 32);
		desc->fill_pattern_dst_addr_lo = (u32)dst;
		desc->fill_pattern_dst_addr_hi = (u32)(dst >> 32);
		break;
	default:
		printf("mv_ddr: dma: unsupported operation mode\n");
		return;
	}
}

static void mv_xor_v2_enqueue(u32 xor_id, u32 desc_num)
{
	u32 xor_base = MV_XOR_ENGINE(xor_id);

	/* write a number of new descriptors in the descriptors queue */
	reg_write(xor_base + DMA_DESQ_ADD_OFF, desc_num);
}

static u32 mv_xor_v2_done(u32 xor_id)
{
	u32 xor_base = MV_XOR_ENGINE(xor_id);

	/* return a number of completed descriptors */
	return reg_read(xor_base + DMA_DESQ_DONE_OFF) & DMA_DESQ_DONE_PENDING_MASK;
}

static void mv_xor_v2_finish(u32 xor_id)
{
	u32 xor_base = MV_XOR_ENGINE(xor_id);

	/* reset dma engine */
	reg_write(xor_base + DMA_DESQ_STOP_OFF, DMA_DESQ_STOP_QUEUE_RESET_ENA << DMA_DESQ_STOP_QUEUE_RESET_OFFS);

	/* clear all pending interrupt indications, to release DMA engine in a clean state */
	reg_write(xor_base + GLOB_SYS_INT_CAUSE, 0);

	/*
	 * disable secure mode in dma engine xorg secure reg to return dma
	 * to the state (unsecure) prior to mv_xor_v2_init() call
	 */
	reg_bit_clrset(xor_base + GLOB_SECURE, GLOB_SECURE_UNSECURE << GLOB_SECURE_SECURE_OFF,
		       GLOB_SECURE_SECURE_MASK << GLOB_SECURE_SECURE_OFF);
}

static u32 mv_xor_v2_memcmp_status_get(u32 xor_id, u32 desc_id)
{
	struct mv_xor_v2_hw_desc *desc = &qmem[xor_id][desc_id];

	return (desc->flags >> DESC_BYTE_CMP_STATUS_OFFS) & DESC_BYTE_CMP_STATUS_MASK;
}

/* mv_ddr dma api */
int mv_ddr_dma_memset(uint64_t start_addr, uint64_t size, uint64_t data)
{
	uint64_t start = start_addr;
	uint64_t end = start_addr + size;
	uint64_t buffer_size;
	u32 desc_id = 0;

	/* initialize dma descriptors queue memory region */
	memset((void *)qmem, 0, sizeof(qmem));

	/* initialize dma engine */
	mv_xor_v2_init(DMA_ENG_0);

	while (start < end) {
		if (desc_id >= MV_XOR_V2_MAX_DESC_NUM) {
			/* increase dma max desc number if required */
			printf("mv_ddr: error: dma desq size limit (%d) reached\n",
			       MV_XOR_V2_MAX_DESC_NUM);
			return 1; /* fail */
		}
		buffer_size = end - start;
		if (buffer_size > MV_XOR_MAX_TRANSFER_SIZE)
			buffer_size = MV_XOR_MAX_TRANSFER_SIZE;
		mv_xor_v2_desc_prep(DMA_ENG_0, desc_id, DESC_OP_MODE_MEMSET,
				    0, start, buffer_size, data);
		mv_xor_v2_enqueue(DMA_ENG_0, 1);
		desc_id++;
		start += buffer_size;
	}

	/* wait for transfer completion */
	while (mv_xor_v2_done(DMA_ENG_0) != desc_id)
		;

	/* disable dma engine */
	mv_xor_v2_finish(DMA_ENG_0);

	return 0; /* pass */
}

int mv_ddr_dma_memcpy(uint64_t *src, uint64_t *dst, uint64_t size, u32 dma_num, u32 desc_num)
{
	uint64_t desc_dst;
	u32 dma_id, desc_id;

	if (dma_num > MV_XOR_ENGINE_NUM) {
		printf("mv_ddr: error: dma engines number limit (%d) reached\n",
		       MV_XOR_ENGINE_NUM);
		return 1; /* fail */
	}

	if (desc_num > MV_XOR_V2_MAX_DESC_NUM) {
		/* increase dma max desc number if required */
		printf("mv_ddr: error: dma desq size limit (%d) reached\n",
		       MV_XOR_V2_MAX_DESC_NUM);
		return 1; /* fail */
	}

	/* initialize dma descriptors queue memory region */
	memset((void *)qmem, 0, sizeof(qmem));

	/* initialize dma engines */
	for (dma_id = 0; dma_id < dma_num; dma_id++)
		mv_xor_v2_init(dma_id);

	for (dma_id = 0; dma_id < dma_num; dma_id++) {
		for (desc_id = 0; desc_id < desc_num; desc_id++) {
			desc_dst = dst[dma_id] + size * desc_id;
			/* prepare dma hw descriptor */
			mv_xor_v2_desc_prep(dma_id, desc_id, DESC_OP_MODE_MEMCPY,
					    src[dma_id], desc_dst, size, 0);
		}
	}
	/* enqueue dma descriptors to start processing */
	for (dma_id = 0; dma_id < dma_num; dma_id++)
		mv_xor_v2_enqueue(dma_id, desc_num);

	/* wait for transfer completion */
	for (dma_id = 0; dma_id < dma_num; dma_id++)
		while (mv_xor_v2_done(dma_id) != desc_num)
			;

	/* disable dma engines */
	for (dma_id = 0; dma_id < dma_num; dma_id++)
		mv_xor_v2_finish(dma_id);

	return 0; /* pass */
}

int mv_ddr_dma_memcmp(uint64_t *src, uint64_t *dst, uint64_t size, u32 dma_num, u32 desc_num)
{
	uint64_t desc_dst;
	u32 dma_id, desc_id;
	int fail_cnt = 0;

	if (dma_num > MV_XOR_ENGINE_NUM) {
		printf("mv_ddr: error: dma engines number limit (%d) reached\n",
		       MV_XOR_ENGINE_NUM);
		return -1; /* fail */
	}

	if (desc_num > MV_XOR_V2_MAX_DESC_NUM) {
		/* increase dma max desc number if required */
		printf("mv_ddr: error: dma desq size limit (%d) reached\n",
		       MV_XOR_V2_MAX_DESC_NUM);
		return -1; /* fail */
	}

	/* initialize dma descriptors queue memory region */
	memset((void *)qmem, 0, sizeof(qmem));

	/* initialize dma engines */
	for (dma_id = 0; dma_id < dma_num; dma_id++)
		mv_xor_v2_init(dma_id);

	for (dma_id = 0; dma_id < dma_num; dma_id++) {
		for (desc_id = 0; desc_id < desc_num; desc_id++) {
			desc_dst = dst[dma_id] + size * desc_id;
			/* prepare dma hw descriptor */
			mv_xor_v2_desc_prep(dma_id, desc_id, DESC_OP_MODE_MEMCMP,
					    src[dma_id], desc_dst, size, 0);
		}
	}
	/* enqueue dma descriptors to start processing */
	for (dma_id = 0; dma_id < dma_num; dma_id++)
		mv_xor_v2_enqueue(dma_id, desc_num);

	/* wait for transfer completion */
	for (dma_id = 0; dma_id < dma_num; dma_id++)
		while (mv_xor_v2_done(dma_id) != desc_num)
			;

	/* disable dma engines */
	for (dma_id = 0; dma_id < dma_num; dma_id++)
		mv_xor_v2_finish(dma_id);

	for (dma_id = 0; dma_id < dma_num; dma_id++) {
		for (desc_id = 0; desc_id < desc_num; desc_id++) {
			if (mv_xor_v2_memcmp_status_get(dma_id, desc_id) == 0)
				fail_cnt++; /* count a number of failures */
		}
	}

	return fail_cnt;
}
