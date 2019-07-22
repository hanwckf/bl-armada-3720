/*
* ***************************************************************************
* Copyright (C) 2017 Marvell International Ltd.
* ***************************************************************************
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* Neither the name of Marvell nor the names of its contributors may be used
* to endorse or promote products derived from this software without specific
* prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
***************************************************************************
*/

#ifndef __DDRCORE_H_
#define __DDRCORE_H_

#define MAX_CS_NUM		2
#define MAX_BANK_GROUP_NUM	4
#define MAX_BANK_NUM	8

/* LOG LEVELS */
#define LOG_LEVEL_NONE  0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_DEBUG 3

/* REGS DUMP BITMAP */
#define FLAG_REGS_DUMP_NONE     NO_BIT
#define FLAG_REGS_DUMP_SELFTEST BIT0
#define FLAG_REGS_DUMP_DDR_CTRL BIT1
#define FLAG_REGS_DUMP_DDR_PHY  BIT2
#define FLAG_REGS_PHYINIT_SYNC2	BIT3
#define FLAG_REGS_INIT_TIMING	BIT4
#define FLAG_REGS_TERM		BIT5
#define FLAG_REGS_QS_GATE	BIT6
#define FLAG_REGS_VREF_READ	BIT7
#define FLAG_REGS_VREF_WRITE	BIT8
#define FLAG_REGS_DLL_TUNE	BIT9
#define FLAG_WARM_BOOT		BIT10
#define FLAG_REGS_DUMP_ALL      0xFFFFFFFF

enum ddr_type {
	DDR3  = 0,
	DDR4,
	DDR_TYPE_MAX,
};

struct ddr_cs_data {
	unsigned int group_num;
	unsigned int bank_num;
	unsigned int capacity;
};

enum ddr_speedbin{
	SPEED_BIN_DDR_1600,
};

struct ddr_topology {
	enum ddr_speedbin speed_bin;
	unsigned int bus_width;
	unsigned int cs_num;
	struct ddr_cs_data cs[MAX_CS_NUM];
};

typedef int (*clock_func_t)(void);

struct ddr_dll_tune {
	unsigned int dll_ctrl_b0; /* 0x1050 */
	unsigned int dll_ctrl_b1; /* 0x1054 */
	unsigned int dll_ctrl_adcm; /* 0x1074 */
};

struct ddr_init_result {
	struct ddr_dll_tune dll_tune;
	union {
		struct {
			unsigned int wl_rl_ctl; /* 0x10C0 */
			unsigned int cs0_b0; /* 0x1180 */
			unsigned int cs0_b1; /* 0x1184 */
			unsigned int cs1_b0; /* 0x11A4 */
			unsigned int cs1_b1; /* 0x11A8 */
		} ddr3; /* qs_gating */

		struct {
			unsigned int vref_read; /* only for DDR4 0x1038 */
			unsigned int vref_write; /* only for DDR4 0x30C */
		} ddr4;
	};
};

struct ddr_win {
	unsigned int base;
	unsigned int size;
};

struct ddr_init_para {
	bool warm_boot;
	unsigned int speed;
	unsigned int log_level;
	unsigned int flags;
	clock_func_t clock_init;
	struct ddr_win cs_wins[MAX_CS_NUM];
};

int set_ddr_type(enum ddr_type type);
int set_ddr_topology_parameters(struct ddr_topology top_map);
int init_ddr(struct ddr_init_para init_para,
	struct ddr_init_result *result);

#endif /* __DDRCORE_H_ */
