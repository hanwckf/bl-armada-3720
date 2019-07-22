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
#include "ddr_support.h"
#include "../sys.h"
#include "ddr.h"

//This function will not check the sanity of data
void replace_val(unsigned int addr, unsigned int data, unsigned int offset, unsigned int mask)
{
        unsigned int w32RegVal;
        data <<= offset;                                //shift data into position
        data &= mask;                                   //mask the data
        w32RegVal = ll_read32(addr);    //Read the register value
        w32RegVal &= ~mask;                             //Mask the register data, modify only bits that are needed
        w32RegVal |= data;                              //Create the final value to write to register
        ll_write32(addr, w32RegVal);    //write data into register
}

void logs_training_regs(enum training type)
{
	switch(type)
	{
		case PHYINIT_SYNC2:
	        	LogMsg(LOG_LEVEL_INFO, FLAG_REGS_PHYINIT_SYNC2, "\n\tCH0_PHY_Control_6[0x%08X]: 0x%08X", CH0_PHY_Control_6, ll_read32(CH0_PHY_Control_6));
        	        LogMsg(LOG_LEVEL_INFO, FLAG_REGS_PHYINIT_SYNC2, "\n\tPHY_Control_15[0x%08X]: 0x%08X", PHY_Control_15, ll_read32(PHY_Control_15));
	                LogMsg(LOG_LEVEL_INFO, FLAG_REGS_PHYINIT_SYNC2, "\n\tPHY_Control_16[0x%08X]: 0x%08X", PHY_Control_16, ll_read32(PHY_Control_16));
			break;

		case INIT_TIMING:
			LogMsg(LOG_LEVEL_INFO, FLAG_REGS_INIT_TIMING, "\n\tCH0_DRAM_Config_1[0x%08X]: 0x%08X", CH0_Dram_Config_1, ll_read32(CH0_Dram_Config_1));
			break;

		case TERM:
			LogMsg(LOG_LEVEL_INFO, FLAG_REGS_TERM, "\n\tCH0_PHY_Control_2[0x%08X]: 0x%08X", CH0_PHY_Control_2, ll_read32(CH0_PHY_Control_2));
			break;

		case QS_GATE:
			LogMsg(LOG_LEVEL_INFO, FLAG_REGS_QS_GATE, "\n\tCH0_PHY_RL_Control_CS0_B0[0x%08X]: 0x%08X", CH0_PHY_RL_Control_CS0_B0, ll_read32(CH0_PHY_RL_Control_CS0_B0));
        	        LogMsg(LOG_LEVEL_INFO, FLAG_REGS_QS_GATE, "\n\tCH0_PHY_RL_Control_CS0_B1[0x%08X]: 0x%08X", CH0_PHY_RL_Control_CS0_B1, ll_read32(CH0_PHY_RL_Control_CS0_B1));
                	LogMsg(LOG_LEVEL_INFO, FLAG_REGS_QS_GATE, "\n\tCH0_PHY_RL_Control_CS1_B0[0x%08X]: 0x%08X", CH0_PHY_RL_Control_CS1_B0, ll_read32(CH0_PHY_RL_Control_CS1_B0));
			LogMsg(LOG_LEVEL_INFO, FLAG_REGS_QS_GATE, "\n\tCH0_PHY_RL_Control_CS1_B1[0x%08X]: 0x%08X\n", CH0_PHY_RL_Control_CS1_B1, ll_read32(CH0_PHY_RL_Control_CS1_B1));
			break;

		case VREF_READ:
			LogMsg(LOG_LEVEL_INFO, FLAG_REGS_VREF_READ, "\n\tPHY_Control_15[0x%08X]: 0x%08X\n", PHY_Control_15, ll_read32(PHY_Control_15));
			break;

		case VREF_WRITE:
			LogMsg(LOG_LEVEL_INFO, FLAG_REGS_VREF_WRITE, "\n\tCH0_DRAM_Config_4[0x%08X]: 0x%08X\n", CH0_DRAM_Config_4, ll_read32(CH0_DRAM_Config_4));
			break;

		case DLL_TUNE:
			LogMsg(LOG_LEVEL_INFO, FLAG_REGS_DLL_TUNE, "\n\tCH0_PHY_DLL_control_B0[0x%08X]: 0x%08X", CH0_PHY_DLL_control_B0, ll_read32(CH0_PHY_DLL_control_B0));
        	        LogMsg(LOG_LEVEL_INFO, FLAG_REGS_DLL_TUNE, "\n\tCH0_PHY_DLL_control_B1[0x%08X]: 0x%08X", CH0_PHY_DLL_control_B1, ll_read32(CH0_PHY_DLL_control_B1));
                	LogMsg(LOG_LEVEL_INFO, FLAG_REGS_DLL_TUNE, "\n\tCH0_PHY_DLL_control_ADCM[0x%08X]: 0x%08X\n", CH0_PHY_DLL_control_ADCM, ll_read32(CH0_PHY_DLL_control_ADCM));
			break;

		default:
			printf("\n\tNO SUCH TRAINING");
	}
}
