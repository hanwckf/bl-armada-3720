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
#include "../sys.h"
#include "ddr_support.h"
#include "ddr.h"

#define MAX_BYTELANES		2
#define MAX_RL_CYCLE_DELAY 	0xF
#define MAX_RL_TAP_DELAY	0x7F

int qs_gating(unsigned int base_addr, unsigned int cs, struct ddr_init_result *result);

int qs_gating(unsigned int base_addr, unsigned int cs_num, struct ddr_init_result *result)
{
        unsigned int result_outp, result_outn;
        unsigned int rl_cycle_dly = 0, rl_tap_dly = 0;
        unsigned int cal_done_flag = 0;
	unsigned int enable_wl_rl_ctl_val = 0;

        //QS Gate calibration does not require write data to be properly calibrated. Dummy reads that generate a DQS burst is all that is required. The DQ value is not important.
	LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_QS_GATE, "\nCS%d Calibration start:", cs_num);
        //1.Set QSG_CLR=0, then set QSG_CLR=1. This is to clear the outp/outn registers
        //enable phy rl enable
        replace_val(CH0_PHY_WL_RL_Control, 0x3, CH0_PHY_WL_RL_CONTROL_RL_ENABLE_SHIFT, CH0_PHY_WL_RL_CONTROL_RL_ENABLE_MASK);
	enable_wl_rl_ctl_val = ll_read32(CH0_PHY_WL_RL_Control);
        LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_QS_GATE, "\n\tEnable Read Leveling and clear QSG_Outp/QSG_Outn registers: 0x%08X 0x%08X", CH0_PHY_WL_RL_Control, ll_read32(CH0_PHY_WL_RL_Control));

        ll_read32(base_addr + 0x4);
        wait_ns(100000);
        //Read register QSG_Dx_OUTP and QSG_Dx_OUTN
        result_outp = ll_read32(CH0_PHY_DQS_Gate_Outp_result);
        result_outn = ll_read32(CH0_PHY_DQS_Gate_Outn_result);
        LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_QS_GATE, "\n\tPost clearing outp outn:");
        LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_QS_GATE, "\n\t\tresult_outp[0x%08X] = 0x%08X result_outn[0x%08X] = 0x%08X rl_cycle_dly = 0x%02X rl_tap_dly = 0x%02X",\
        	CH0_PHY_DQS_Gate_Outp_result, result_outp, CH0_PHY_DQS_Gate_Outn_result, result_outn, rl_cycle_dly, rl_tap_dly);

        //2.Enter MPR mode.
	LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_QS_GATE, "\n\tEnter MPR mode");
        ll_write32(CH0_DRAM_Config_3, (ll_read32(CH0_DRAM_Config_3) | 0x00000040));
        ll_write32(USER_COMMAND_2, 0x13000800);

	LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_QS_GATE, "\n\tIncrement cycle and tap delay to detect edges");
        for(rl_cycle_dly = 0; rl_cycle_dly < 0xF; rl_cycle_dly++)
        {
                if(cal_done_flag)
                        break;

                //set cycle delay
                replace_val(CH0_PHY_RL_Control_CS0_B0 + cs_num*0x24, rl_cycle_dly, PHY_RL_CONTROL_CYCLE_DELAY_SHIFT, PHY_RL_CONTROL_CYCLE_DELAY_MASK);
                replace_val(CH0_PHY_RL_Control_CS0_B1 + cs_num*0x24, rl_cycle_dly, PHY_RL_CONTROL_CYCLE_DELAY_SHIFT, PHY_RL_CONTROL_CYCLE_DELAY_MASK);

                for(rl_tap_dly = 0; rl_tap_dly < 0x7F; rl_tap_dly++)
                {
                        //set tap delay
                        replace_val(CH0_PHY_RL_Control_CS0_B0 + cs_num*0x24, rl_tap_dly, PHY_RL_CONTROL_TAP_DELAY_SHIFT, PHY_RL_CONTROL_TAP_DELAY_MASK);
                        replace_val(CH0_PHY_RL_Control_CS0_B1 + cs_num*0x24, rl_tap_dly, PHY_RL_CONTROL_TAP_DELAY_SHIFT, PHY_RL_CONTROL_TAP_DELAY_MASK);

                        //3.Issue a single read to DRAM
                        ll_read32(base_addr + 0x4);

                        //4.Wait until data has returned to the MC.
                        wait_ns(100000);        //TODO: 100us - optimize it?

                        //5.Read register QSG_Dx_OUTP and QSG_Dx_OUTN.
                        result_outp = ll_read32(CH0_PHY_DQS_Gate_Outp_result);
                        result_outn = ll_read32(CH0_PHY_DQS_Gate_Outn_result);
			LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_QS_GATE, "\n\t\tRead Outp = 0x%08X Outn = 0x%08X -> cycle = 0x%02X tap = 0x%02X", result_outp, result_outn, rl_cycle_dly, rl_tap_dly);

                        //6.If QSG_Dx_OUTP != 0xF and QSG_Dx_OUTN != 0xE, then increase either QSG_ALL_CYC_DLY, QSG_Dx_PH_DLY, or QSG_Dx_TAP_DLY. Repeat from 3.
                        //7.If QSG_Dx_OUTP == 0xF and QSG_Dx_OUTN == 0xE, then calibration is done
                        if( (result_outp == 0xFF) && (result_outn == 0xEE) ) {
				LogMsg(LOG_LEVEL_INFO, FLAG_REGS_QS_GATE, "\n\tCS%d: Final Cycle = 0x%02X Tap = 0x%02X", cs_num, rl_cycle_dly, rl_tap_dly);
                                cal_done_flag = 1;
                                break;
                        }
                }
        }

        //8.Exit MPR mode
	LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_QS_GATE, "\n\tExit MPR mode");
        ll_write32(CH0_DRAM_Config_3, (ll_read32(CH0_DRAM_Config_3) & (~0x00000040)));
        ll_write32(USER_COMMAND_2, 0x13000800);

        //disable phy rl enable
	LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_QS_GATE, "\n\tDiable Read Leveling");
        replace_val(CH0_PHY_WL_RL_Control, 0x0, CH0_PHY_WL_RL_CONTROL_RL_ENABLE_SHIFT, CH0_PHY_WL_RL_CONTROL_RL_ENABLE_MASK);

	if(cal_done_flag)
	{
		result->ddr3.wl_rl_ctl = enable_wl_rl_ctl_val;
		//result->qs_gate.dis_wl_rl_ctl = ll_read32(CH0_PHY_WL_RL_Control);
		result->ddr3.cs0_b0 = ll_read32(CH0_PHY_RL_Control_CS0_B0);
	        result->ddr3.cs0_b1 = ll_read32(CH0_PHY_RL_Control_CS0_B1);
		result->ddr3.cs1_b0 = ll_read32(CH0_PHY_RL_Control_CS1_B0);
	        result->ddr3.cs1_b1 = ll_read32(CH0_PHY_RL_Control_CS1_B1);
		LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_QS_GATE, "\nCS%d Calibration done\n", cs_num);
	}
	else
		LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_QS_GATE, "\nCS%d Calibration failed: cycle and tap delay exhausted\n");
        return cal_done_flag;
}
