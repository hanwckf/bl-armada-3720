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
#include <stdbool.h>

int vdac_set(unsigned int vref_range, unsigned int vref_ctrl)   //read vref training
{
        replace_val(PHY_Control_15, vref_ctrl, 24, 0x3F000000);
        replace_val(PHY_Control_15, vref_range, 30, 0xC0000000);
        wait_ns(1000);
        return 0;
}

int vref_read_training(int num_of_cs, struct ddr_init_para init_para)
{
	int dll_range = 0, best_range = 0, best_vref_cnt = -1;
	unsigned int vref_cnt;

	for (vref_cnt = 0x0; vref_cnt <= 0x3F; vref_cnt++) {
		LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_VREF_READ,
		       "\nSet VREF: 0x%02X", vref_cnt);
		vdac_set(1, vref_cnt);
		dll_range = dll_tuning(2, num_of_cs, &init_para, false, false);
		if (dll_range > best_range) {
			best_range = dll_range;
			best_vref_cnt = vref_cnt;
		}

	}
	return best_vref_cnt;
}

int vref_set(unsigned int range, unsigned int VREF_training_value_DQ)
{
	// 1. set range
	replace_val(CH0_DRAM_Config_4, range, 23, 0x00800000);

	// 2. set training value
	replace_val(CH0_DRAM_Config_4, VREF_training_value_DQ, 16, 0x003F0000);

	// 3. Send MR6 command to set range and vref_training value
	ll_write32(USER_COMMAND_2, 0x13004000);
	wait_ns(1000);

	return 0;
}

void en_dis_write_vref(unsigned int enable)
{
	if(enable)
		replace_val(CH0_DRAM_Config_3, 1, 4, 0x00000010);
	else
		replace_val(CH0_DRAM_Config_3, 0, 4, 0x00000010);

	 //Send MR6 command to enable or diable
        ll_write32(USER_COMMAND_2, 0x13004000);
        wait_ns(1000);

	return;
}

unsigned int vref_write_training(int num_of_cs, struct ddr_init_para init_para)
{
        unsigned int result_dll=0;
        unsigned int vref_cnt=0x0;
        unsigned int min=0, prev_min=0, max=0, prev_max=0, window_size = 0, prev_window_size=0, window_on=0;

	//enable vref training
	en_dis_write_vref(1);

        LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_VREF_WRITE, "\nIncrement vref and perform dll tuning for each vref to find min/max vref setting");
        for(vref_cnt=0x0; vref_cnt <= 0x3F; vref_cnt++)
        {
                LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_VREF_WRITE, "\nSet VREF: 0x%02X", vref_cnt);
                vref_set(1, vref_cnt);
		result_dll = dll_tuning(2, num_of_cs, &init_para, false, false);

		if (result_dll > 0) /* if DLL pass */
                {
                        if((min == 0) && (window_on ==0))                                       //set the left edge first time
                        {
                                min = vref_cnt;
                                max = vref_cnt;
                                window_size++;
                                window_on=1;
                        }
                        else                                                                    //update right edge for every continuous pass
                        {
                                if(window_on == 0)
                                {
                                        min = vref_cnt;
                                        window_on = 1;
                                }
                                max = vref_cnt;
                                window_size++;
				if(max == 0x3F)
                                        goto LAST_WINDOW;
                        }
                }
                else                                                                            //if DLL failed
                {
                        LAST_WINDOW:
                        LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_VREF_WRITE, "\nDLL tuning failed on atleast 1CS for this vref setting");
		        if((window_size >= prev_window_size) || (prev_window_size == 0))
                        {
                                prev_max = max; prev_min = min; prev_window_size = window_size; //save old window values to be compared later
                                max = 0; min = 0; window_size = 0;      //reset all windows
                                window_on = 0;
                        }
                }
		LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_VREF_WRITE, "\nmin  = 0x%X max = 0x%X window_size = %d\n", min, max, window_size);
        }
        LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_VREF_WRITE, "\nFinal min  = 0x%X Final max = 0x%X", prev_min, prev_max);

	//disable vref training
        en_dis_write_vref(0);

        if(prev_min!=prev_max)                                                  //prev values are the correct ones
                return (((prev_max-prev_min)/2) + prev_min);			//PASS
        else
                return 0;							//FAIL
}
