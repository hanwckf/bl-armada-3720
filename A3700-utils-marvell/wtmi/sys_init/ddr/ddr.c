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
#include "ddr.h"
#include "ddr_support.h"

#define SELF_REFESH_STS(cs_num)        (BIT2 << (cs_num * 4))
#define USER_CMD_0_CS_BIT(cs_num)      (BIT24 << (cs_num))
#define USER_CMD_0_SELF_REFRESH_ENTERY (0x10000040)
#define USER_CMD_0_SELF_REFRESH_EXIT   (0x10000080)

void mc6_init_timing_selfrefresh(enum ddr_type type, unsigned int speed)
{
        unsigned int wrval = 0, rdval = 0;
        LogMsg(LOG_LEVEL_INFO, FLAG_REGS_INIT_TIMING, "\nUpdate CAS Read and Write Latency for %dMHz", speed);
        rdval = ll_read32(CH0_Dram_Config_1);
        if(type==DDR3)
        {
                if(speed == 800)
                        wrval = (rdval & ~(0x00003F3F)) | (0x80B);      //for 800MHz
                else if(speed == 600)
                        wrval = (rdval & ~(0x00003F3F)) | (0x80B);      //for 600MHz
                else if(speed == 750)
                        wrval = (rdval & ~(0x00003F3F)) | (0x80B);      //for 750MHz
        }
        else if ((type==DDR4) && (speed == 800))
                wrval = (rdval & ~(0x00003F3F)) | (0xB0C);              //for 800MHz CL[0:5], cwl = 11[0xB], cl = 12[0xC]

        ll_write32(CH0_Dram_Config_1, wrval);
}

void send_mr_commands(enum ddr_type type){
        if(type == DDR3)
        {
                ll_write32(USER_COMMAND_2, 0x13000400); //send MRS command to MR2
                ll_write32(USER_COMMAND_2, 0x13000800); //send MRS command to MR3
                ll_write32(USER_COMMAND_2, 0x13000200); //send MRS command to MR1
                ll_write32(USER_COMMAND_2, 0x13000100); //send MRS command to MR0
        }
        else if(type == DDR4)
        {
                ll_write32(USER_COMMAND_2, 0x13000800); //send MRS command to MR3
                ll_write32(USER_COMMAND_2, 0x13004000); //send MRS command to MR6
                ll_write32(USER_COMMAND_2, 0x13002000); //send MRS command to MR5
                ll_write32(USER_COMMAND_2, 0x13001000); //send MRS command to MR4
                ll_write32(USER_COMMAND_2, 0x13000400); //send MRS command to MR2
                ll_write32(USER_COMMAND_2, 0x13000200); //send MRS command to MR1
                ll_write32(USER_COMMAND_2, 0x13000100); //send MRS command to MR0
        }
        wait_ns(10000);
}

void set_clear_trm(int set, unsigned int orig_val)
{
        unsigned int wrval = 0, rdval = 0;
        rdval = ll_read32(CH0_PHY_Control_2);
        if(set)
        {
                LogMsg(LOG_LEVEL_INFO, FLAG_REGS_TERM, "\nRestore termination values to original values");
                ll_write32(CH0_PHY_Control_2, rdval | orig_val);
        }
        else
        {
                LogMsg(LOG_LEVEL_INFO, FLAG_REGS_TERM, "\nSet termination values to 0");
                wrval = (rdval & ~(0x0FF00000));                //set trm to 0
                ll_write32(CH0_PHY_Control_2, wrval);
        }
}

void self_refresh_entry(u32 cs_num, enum ddr_type type)
{
	u32 sts_mask = SELF_REFESH_STS(cs_num);

	ll_write32(USER_COMMAND_0, (USER_CMD_0_SELF_REFRESH_ENTERY |
		   USER_CMD_0_CS_BIT(cs_num)));
	if (type == DDR4)
		while (!(ll_read32(DRAM_STATUS) & sts_mask))
			;
	else
		wait_ns(1000);
	LogMsg(LOG_LEVEL_INFO, FLAG_REGS_DUMP_SELFTEST,
		"\n\nCS %d Now in Self-refresh Mode", cs_num);
}

void self_refresh_exit(u32 cs_num)
{
	u32 sts_mask = SELF_REFESH_STS(cs_num);

	ll_write32(USER_COMMAND_0, (USER_CMD_0_SELF_REFRESH_EXIT |
		   USER_CMD_0_CS_BIT(cs_num)));
	while ((ll_read32(DRAM_STATUS) & sts_mask))
		;
	LogMsg(LOG_LEVEL_INFO, FLAG_REGS_DUMP_SELFTEST,
		"\nCS %d Exited self-refresh ...\n", cs_num);
}

void self_refresh_test(int verify, unsigned int base_addr, unsigned int size)
{
        unsigned int end, temp;
        unsigned int *waddr, refresh_error = 0;

        end = base_addr + size;

        if(!verify)
        {
                // Write pattern
                LogMsg(LOG_LEVEL_INFO, FLAG_REGS_DUMP_SELFTEST, "\nFill memory before entering Self-refresh...");
                for (waddr = (unsigned int *)base_addr; waddr  < (unsigned int *)end ; waddr++)
                {
                        *waddr = (unsigned int)waddr;
                }
                LogMsg(LOG_LEVEL_INFO, FLAG_REGS_DUMP_SELFTEST, "done");
        }
        else
        {
                // Check data after exit self refresh
                for (waddr = (unsigned int *)base_addr; waddr  < (unsigned int *)end ; waddr++)
                {
                        temp = *waddr;
                        if (temp != (unsigned int)waddr)
                        {
                                LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_DUMP_SELFTEST, "\nAt 0x%08x, expect 0x%08x, read back 0x%08x", (unsigned int)waddr, (unsigned int)waddr, temp);
                                refresh_error++;
                        }
                }
                if (refresh_error)
                        LogMsg(LOG_LEVEL_ERROR, FLAG_REGS_DUMP_SELFTEST, "\nSELF-REFRESH TEST FAIL. Error count = 0x%x", refresh_error);
                else
                        LogMsg(LOG_LEVEL_ERROR, FLAG_REGS_DUMP_SELFTEST, "\nSELF-REFRESH TEST PASS");
        }
}

void phyinit_sequence_sync2(volatile unsigned short ld_phase,
		volatile  unsigned short wrst_sel, volatile  unsigned short wckg_dly,
		volatile  unsigned short int wck_en)
{
	// sync2 procedure
	replace_val(CH0_PHY_Control_6, 1, 19, 0x00080000);		//MC_SYNC2_EN
	replace_val(CH0_PHY_Control_6, ld_phase, 9, 0x00000200);//ld_phase update
	replace_val(PHY_Control_15, wrst_sel, 0, 0x00000003);
	replace_val(PHY_Control_16, wckg_dly, 4, 0x000000F0);		//set wckg_dly
	replace_val(PHY_Control_16, wck_en, 0, 0x00000001);		//wck_en

	ll_write32(PHY_CONTROL_9, 0x80000000);
	ll_write32(PHY_CONTROL_9, 0x20000000);
	ll_write32(PHY_CONTROL_9, 0x40000000);
	wait_ns(10000);
	ll_write32(PHY_CONTROL_9, 0x80000000);
	wait_ns(10000);
}
