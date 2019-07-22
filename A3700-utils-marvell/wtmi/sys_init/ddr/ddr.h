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
#ifndef _DDR_H
#define _DDR_H

#include "ddrcore.h"

#define DRAM_STATUS				0xC0000008
#define CH0_Dram_Config_1			0xC0000300
#define CH0_Dram_Config_2			0xC0000304
//For read leveling
#define CH0_DRAM_Config_3			0xC0000308
#define CH0_DRAM_Config_4           		0xC000030C
#define USER_COMMAND_0                  	0xC0000020
#define USER_COMMAND_2				0xC0000028
#define CH0_PHY_RL_Control_CS0_B0		0xC0001180
#define CH0_PHY_RL_Control_CS0_B1		0xC0001184
#define CH0_PHY_RL_Control_CS1_B0		0xC00011A4
#define CH0_PHY_RL_Control_CS1_B1		0xC00011A8
#define PHY_RL_CONTROL_CYCLE_DELAY_SHIFT 	8
#define PHY_RL_CONTROL_CYCLE_DELAY_MASK		0x00000F00
#define PHY_RL_CONTROL_TAP_DELAY_MASK		0x0000007F
#define PHY_RL_CONTROL_TAP_DELAY_SHIFT		0

#define CH0_PHY_WL_RL_Control			0xC00010C0
#define CH0_PHY_WL_RL_CONTROL_RL_ENABLE_SHIFT	0
#define CH0_PHY_WL_RL_CONTROL_RL_ENABLE_MASK	0x00000003

#define CH0_PHY_DQS_Gate_Outp_result		0xC0001170
#define CH0_PHY_DQS_Gate_Outn_result		0xC0001178

//For setup hold offset
#define NUM_OF_SUBPHY 				5
#define NUM_OF_CS 				1

#define CH0_PHY_PAD_WDLY0			0xC00010D0
#define CH0_PHY_PAD_WDLY1			0xC00010D4
#define CH0_PHY_PAD_WDLY2			0xC00010D8
#define CH0_PHY_PAD_WDLY3			0xC00010DC
#define CH0_PHY_PAD_DLY_Control			0xC00010E0

#define CH0_PHY_Control_1			0xC0001000
#define CH0_PHY_Control_2			0xC0001004
#define CH0_PHY_Control_3			0xC0001008
#define CH0_PHY_Control_4			0xC000100C
#define CH0_PHY_Control_5			0xC0001010
#define CH0_PHY_Control_6			0xC0001014

#define PHY_Control_15				0xC0001038
#define PHY_Control_16				0xC000103C
#define CH0_PHY_DLL_control_B0 	 		0xC0001050
#define CH0_PHY_DLL_control_B1  		0xC0001054
#define CH0_PHY_DLL_control_ADCM		0xC0001074

//For DLL phsel phsel1 shmoo
#define PHY_DLL_BYTE_SELECT 			0xC0001078
#define PHY_DLL_CONTROL_BASE			0xC0001050
#define PHY_CONTROL_8				0xC000101C
#define PHY_CONTROL_9				0xC0001020

int dll_tuning(unsigned int ratio, unsigned int num_of_cs,
	       const struct ddr_init_para *init_para, bool mpr_mode,
	       bool save_res);
void mc6_init_timing_selfrefresh(enum ddr_type type, unsigned int speed);
void set_clear_trm(int set, unsigned int val);
void self_refresh_entry(u32 cs_num, enum ddr_type type);
void self_refresh_exit(u32 tc_cs_num);
void self_refresh_test(int verify, unsigned int base_addr, unsigned int size);
void send_mr_commands(enum ddr_type type);
int qs_gating(unsigned int base_addr, unsigned int cs, struct ddr_init_result *result);
int vref_read_training(int num_of_cs, struct ddr_init_para init_para);
unsigned int vref_write_training(int num_of_cs, struct ddr_init_para init_para);
int vdac_set(unsigned int vref_range, unsigned int vref_ctrl);
int vref_set(unsigned int range, unsigned int VREF_training_value_DQ);
void en_dis_write_vref(unsigned int enable);

void phyinit_sequence_sync2(volatile unsigned short ld_phase,
		volatile  unsigned short wrst_sel, volatile  unsigned short wckg_dly,
		volatile  unsigned short int wck_en);

#endif
