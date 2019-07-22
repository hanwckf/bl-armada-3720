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
/*-------------------------------------------------------------------------------
*  $Revision:
*
*  Description:	DDR initialization
*
*
*---------------------------------------------------------------------------*/

#include "../sys.h"
#include "ddr.h"
#include "ddr_support.h"

#define DLL_PHSEL_START		0x00
#define DLL_PHSEL_END		0x3F
#define DLL_PHSEL_STEP		0x1
#define BYTE_MASK(byte)		(0xff00ff << (byte))
#define BYTE_CONTROL(byte)	(PHY_DLL_CONTROL_BASE + (byte) * 4)
#define DLL_MASTER		16
#define DLL_NEG			24
#define DLL_TYPE_MASK(type)	(0x3F << (type))
#define BYTE_ERROR(a, b, mask)	(((a) & (mask)) != ((b) & (mask)))


struct dll_tuning_info {
	unsigned short left;
	unsigned short right;
	unsigned short medium;
};

static const unsigned int tune_patterns[] =
{
	0x5555aaaa,
	0xaaaa5555,
	0x0000ffff,
	0xf0f0f0f0,
	0x0f0f0f0f,
	0xf0f0f0f0,
	0x0f0f0f0f,
	0xabadbeef,
	0xbeefabad,
	0x0000000f,
	0x000000f0,
	0x00000f00,
	0x0000f000,
	0x000f0000,
	0x00f00000,
	0x0f000000,
	0xf0000000,
	0x0000000f,
	0x55555555,
	0xaaaaaaaa,
	0x00000000,
	0xffffffff,
	0x00000000,
	0xffffffff,
	0x00000000,
	0x807D5E19,
	0xA52D0F2D,
	0xB68A62AF,
	0x44F4BF0C,
	0x0
};

static int static_pattern(unsigned int wdata, unsigned int start,
			  unsigned int end, u32 byte_mask)
{
	volatile unsigned int *l_waddr;
	unsigned int l_rdata;

	for (l_waddr = (volatile unsigned int *)start;
		 l_waddr < (volatile unsigned int *)end;
		 l_waddr++) {
		*l_waddr = wdata;// write data in
		l_rdata = *l_waddr;// read data back

		if (BYTE_ERROR(l_rdata, wdata, byte_mask))
			return 1;
	}

	return 0;
}

static int walking1_pattern(unsigned int start, unsigned int end, u32 byte_mask)
{
	volatile unsigned int *waddr;// a pointer to a short( 16 bit)
	unsigned int wdata, rdata;
	int i;

	wdata = 0x8000;//original data 16 bits

	for (waddr = (volatile unsigned int *)start; waddr < (volatile unsigned int *)end; waddr++, wdata = 0x8000)
	{
		for (i = 0; i < 16; i++, wdata = wdata >> 1) {

			*waddr = wdata;// write data in
			rdata = *waddr;// read data back

			if (BYTE_ERROR(wdata, rdata, byte_mask))
				return 1;
		}
	}

	return 0;
}

static int ddr_wr_test(unsigned int start, unsigned int size, u32 byte_mask)
{
	unsigned int end;
	int i;

	end = start + size;

	for (i = 0; i < sizeof(tune_patterns) / sizeof(tune_patterns[0]); i++) {
		if (static_pattern(tune_patterns[i], start, end, byte_mask))
			return 1;
	}

	if (walking1_pattern(start, end, byte_mask))
		return 1;

	return 0; /* pass */
}

void reset_dll_phy(void)
{
	// reset PHY DLL. Dll_reset_timer ([31:24] of PHY Control Register 8, Offset 0x41C/0xC1C) is set to 0x10, reset DLL for 128*32=4096 MCLK cycles.
	// updates DLL master. Block read/MMR for 4096 MCLK cycles to guarantee DLL lock. Either wait 4096 MCLK (memPll/4) cycles, or check DLL lock status
	ll_write32(PHY_CONTROL_9, 0x20000000);	//DLL reset
	//TODO: optimize the delays for specific CPU frequency
	wait_ns(640);							//delay(512nCK);Assuming 800MHz CPU frequency
	// update Dll delay_l. When Dll_update_timer ([20:16] of PHY Control Register 8, Offset 0x41C/0xC1C) is 0x11, assert DLL_UPDATE_EN pin for 0x11*16 = 272 MCLK cycles.
	// copy DLL master to DLL slave. Slave controls the actual delay_l. Both DLL bypass and DLL needs 'update'.
	ll_write32(PHY_CONTROL_9, 0x40000000);	//Update  DLL
	//TODO: optimize the delays for specific CPU frequency
	wait_ns(640);                   		//delay(512nCK);Assuming 800MHz CPU frequency
	ll_write32(PHY_CONTROL_9, 0x0);			//Finish DLL update
	//TODO: optimize the delays for specific CPU frequency
	wait_ns(640);                   		//delay(512nCK);Assuming 800MHz CPU frequency
}

static bool mpr_read_test(unsigned int start, unsigned int ddr_size,
				  u32 byte_mask)
{
	volatile unsigned int *l_waddr;
	unsigned int l_rdata, l_pattern_ddr4;

	l_pattern_ddr4 = 0xFFFF0000;

	for (l_waddr = (volatile unsigned int *)start;
	     l_waddr < (volatile unsigned int *)(start + ddr_size);
	     l_waddr++) {
		l_rdata = *l_waddr;

		if (BYTE_ERROR(l_rdata, l_pattern_ddr4, byte_mask))
			return 1; /* 1 => fail */
	}
	return 0;
}
/* Check correctness of ddr read/write for all dll values.
 * Returns the working dll range.
 */
bool short_dll_tune(unsigned int ratio,
			    unsigned int mpr_en,
			    const struct ddr_init_para *params,
			    unsigned int num_of_cs,
			    struct dll_tuning_info *ret, u32 dll_type,
			    u32 mask, u32 ctrl_addrs)
{
	unsigned short left, right, i;
	unsigned short medium;
	unsigned int regval, res;
	u32 beckup = ll_read32(ctrl_addrs);

	ll_write32(PHY_CONTROL_9, 0x0);

	/* Automatically update PHY DLL with interval time
	 * set in Dll_auto_update_interval ([15:8] of
	 * PHY Control Register 13, Offset 0x248)
	 */
	regval = ll_read32(PHY_CONTROL_8);

	/* turn off Dll_auto_manual_update & Dll_auto_update_en
	 * DLL_auto_update_en has a known bug. Don't use.
	 */
	regval &= ~0xC;
	/* change Dll_reset_timer to 128*32 cycles */
	regval |= 0x80000000;
	ll_write32(PHY_CONTROL_8, regval);  /* Write R41C */

	LogMsg(LOG_LEVEL_DEBUG,
	       FLAG_REGS_DLL_TUNE,
	       "Increment dll_phsel by 1 and find the passing window");
	/* enable mpr mode */
	if(mpr_en)
	{
		ll_write32(CH0_DRAM_Config_3, (ll_read32(CH0_DRAM_Config_3) | 0x00000040));
		ll_write32(USER_COMMAND_2, 0x13000800);
	}
	left = DLL_PHSEL_END;
	right = DLL_PHSEL_START;

	for (i = DLL_PHSEL_START; i <= DLL_PHSEL_END; ++i) {
		int cs;

		replace_val(ctrl_addrs, i,
			     dll_type, DLL_TYPE_MASK(dll_type));
		reset_dll_phy();
		wait_ns(100);

		res = 0;
		for (cs = 0; cs < num_of_cs; ++cs) {
			if (mpr_en)
				res |= mpr_read_test(params->cs_wins[cs].base,
						     100*2, mask);
			else
				res |= ddr_wr_test(params->cs_wins[cs].base,
						   32, mask);
		}
		if (!res) { /* pass */
			if( i<left) left = i;
			if( i>right) right = i;
			LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_DLL_TUNE, "\n\t\tdll_phsel_0 = dll_phsel_1 = 0x%02X left = 0x%02X, right = 0x%02X", i, left, right);
		}
	}
	ll_write32(ctrl_addrs, beckup);
	reset_dll_phy();
	if (left > right) {
		LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_DLL_TUNE,
		       "\n\t\tNo passing window");
		return 0;
	}
	medium = left + ((right-left)/ratio);
	LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_DLL_TUNE,
	       "\n\t\tPassing window: 0x%02X-0x%02X \t\tMedium = 0x%02X",
	       left, right, medium);
	ret->left = left;
	ret->right = right;
	ret->medium = medium;
	/* disable mpr mode */
	if (mpr_en)
	{
		ll_write32(CH0_DRAM_Config_3, (ll_read32(CH0_DRAM_Config_3) & (~0x00000040)));
		ll_write32(USER_COMMAND_2, 0x13000800);
	}
	return 1;
}

/* function will return the dll range for that verf.
 * range <=0 means range is zero length
 */
int dll_tuning(unsigned int ratio, unsigned int num_of_cs,
	       const struct ddr_init_para *init_para, bool mpr_mode,
	       bool save_res)
{
	unsigned int i;
	/* size start at max dll range */
	int size = DLL_PHSEL_END - DLL_PHSEL_START;
	unsigned short byte[] = {0, 0, 1, 1};
	u32 dll_type[] = {DLL_MASTER, DLL_NEG, DLL_MASTER, DLL_NEG};
	const int loop_size = (sizeof(dll_type) / sizeof(dll_type[0]));
	u32 med[loop_size];
	struct dll_tuning_info dll_info;

	LogMsg(LOG_LEVEL_DEBUG, FLAG_REGS_DLL_TUNE,
	       "\nPerform coarse DLL tuning:");

	for (i = 0; i < loop_size; ++i) {
		int current_size;

		if (!short_dll_tune(ratio, mpr_mode, init_para, num_of_cs,
				    &dll_info, dll_type[i], BYTE_MASK(byte[i]),
				     BYTE_CONTROL(byte[i])))
			return 0;
		current_size = dll_info.right - dll_info.left;
		/* select minimum size of each byte0/1
		 * and dll master/neg variation
		 */
		med[i] = dll_info.medium;
		if (current_size < size)
			size = current_size;
	}
	if (save_res) {
		for (i = 0; i < loop_size; ++i)
			replace_val(BYTE_CONTROL(byte[i]), med[i],
			dll_type[i], DLL_TYPE_MASK(dll_type[i]));
		reset_dll_phy();
		wait_ns(100);
	}
	return size;
}
