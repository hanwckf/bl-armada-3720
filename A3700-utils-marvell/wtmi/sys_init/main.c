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

#include "sys.h"
#include "apctl.h"
#include "clock.h"
#include "avs.h"
#include "ddr/ddrcore.h"
#include <string.h>

#if DEBUG
#define ddr_debug printf
#else
#define ddr_debug(...)
#endif

/* DDR topology file defines the memory size by MiB. */
/* This macro is used to covert the size into Bytes. */
#define _MB(sz)	((sz) << 20)

/* Armada37xx always works with a DDR bus of 16-bits. */
#define MC_BUS_WIDTH				16
/*
 * DRAM size per each chip select is calculated
 * according to the device capacity and device
 * bus-width.
 * Working with the device of 8-bits, DRAM size
 * on a single chip select should be doubled.
 */
#define DDR_CS_CAP(dev_sz, dev_bw)	(dev_sz) * (MC_BUS_WIDTH / (dev_bw))

/*
 * CM3 has two windows for DRAM address decoding.
 * It supports up to 1.5GB memory address translation.
 */
#define CM3_DRAM_WIN0_ID		0
#define CM3_DRAM_WIN0_BASE		0x60000000
#define CM3_DRAM_WIN0_SZ_MAX	0x40000000 /* 1024MB */
#define CM3_DRAM_WIN1_ID		1
#define CM3_DRAM_WIN1_BASE		0xA0000000
#define CM3_DRAM_WIN1_SZ_MAX	0x20000000 /* 512MB */

/*
 * This function is used for setting the remap address
 * for a specific decode window.
 * The window is suppose to translate the destination
 * address into the offset to the target unit against
 * the window's base address. With address remapping,
 * the window is able to redirected the access to the
 * target unit with the additional offset of the remap
 * address.
 *
 * It's useful when CM3 need to access a high memory
 * address with the limited window size.
 *
 */
static void set_cm3_win_remap(u32 win, u32 remap_addr)
{
	u32 reg;

	/* Disable the window before configuring it. */
	reg = readl(CM3_WIN_CONROL(win));
	reg &= ~BIT0;
	writel(reg, CM3_WIN_CONROL(win));

	writel((remap_addr & 0xFFFF0000), CM3_WIN_REMAP_LOW(win));

	/* Re-enable the window. */
	reg |= BIT0;
	writel(reg, CM3_WIN_CONROL(win));
}

static u32 do_checksum32(u32 *start, u32 len)
{
	u32 sum = 0;
	u32 *startp = start;

	do {
		sum += *startp;
		startp++;
		len -= 4;
	} while (len > 0);

	return sum;
}

static int sys_check_warm_boot(void)
{
	/* warm boot bit is stored in BIT0 of 0xC001404C */
	if (readl(0xC001404C) & BIT0)
		return 1;

	return 0;
}

int sys_init_main(void)
{
	struct ddr_topology map;
	struct ddr_init_para ddr_para;
	struct ddr_init_result *result_in_dram, result_in_sram;
	u32 chksum_in_dram = 0;

	printf("WTMI: system early-init\n");

	result_in_dram = (struct ddr_init_result *)(DDR_TUNE_RESULT_MEM_BASE);

	ddr_para.warm_boot = sys_check_warm_boot();
	if (ddr_para.warm_boot) {
		chksum_in_dram = *((u32 *)(DDR_TUNE_RESULT_MEM_BASE + sizeof(struct ddr_init_result)));
		if (chksum_in_dram != do_checksum32((u32 *)result_in_dram, sizeof(struct ddr_init_result)))
			printf("DDR tuning result checksum ERROR!\n");
	}

	map.bus_width       = CONFIG_BUS_WIDTH;
	map.cs_num          = CONFIG_CS_NUM;
	map.cs[0].group_num = 0;
	map.cs[0].bank_num  = 8;
	map.cs[0].capacity  = DDR_CS_CAP(CONFIG_DEV_CAP, CONFIG_BUS_WIDTH);
	if (map.cs_num > 1) {
		/* Assume a symetric topology applied on both CS */
		map.cs[1].group_num = 0;
		map.cs[1].bank_num  = 8;
		map.cs[1].capacity  = DDR_CS_CAP(CONFIG_DEV_CAP, CONFIG_BUS_WIDTH);
	}

	ddr_debug("\nDDR topology parameters:\n");
	ddr_debug("========================\n");
	ddr_debug("ddr type               DDR%d\n", CONFIG_DDR_TYPE+3);
	ddr_debug("ddr speedbin           %d\n", CONFIG_SPEED_BIN);
	ddr_debug("bus width              %d-bits\n", map.bus_width);
	ddr_debug("cs num                 %d\n", map.cs_num);
	ddr_debug("  cs[0] - group num    %d\n", map.cs[0].group_num);
	ddr_debug("  cs[0] - bank num     %d\n", map.cs[0].bank_num);
	ddr_debug("  cs[0] - capacity     %dMiB\n", map.cs[0].capacity);
	if (map.cs_num > 1) {
		ddr_debug("  cs[1] - group num    %d\n", map.cs[1].group_num);
		ddr_debug("  cs[1] - bank num     %d\n", map.cs[1].bank_num);
		ddr_debug("  cs[1] - capacity     %dMiB\n", map.cs[1].capacity);
	}

	/* WTMI_CLOCK was set in the compile parametr */
	set_clock_preset(WTMI_CLOCK);
	init_avs(get_cpu_clock());

	set_ddr_type(CONFIG_DDR_TYPE);
	set_ddr_topology_parameters(map);

	ddr_para.log_level  = LOG_LEVEL_ERROR;
	ddr_para.flags      = FLAG_REGS_DUMP_ALL;

	ddr_para.clock_init = setup_clock_tree;
	ddr_para.speed      = get_ddr_clock();

	/*
	 * Both CM3's DRAM address decoding windows are
	 * enabled by default. These two windows has a
	 * linear address mapping to the DDR chips. The
	 * memory address translation to the different
	 * DDR chip is transparent to CM3 processor. So
	 * that there is no need to use the dedicated
	 * window to the specific DDR chip.
	 *
	 * Single CS:
	 * Use only DRAM_WIN0 for address translation.
	 * Keep the default settings for DRAM_WIN0.
	 * Up to CM3_DRAM_WIN0_SZ_MAX (1GB) can be used
	 * by DDR training for memory test.
	 *
	 * Dual CS:
	 * - DRAM_WIN0 will translate the address for
	 *   both DDR chips if the chip's capacity is
	 *   less than CM3_DRAM_WIN0_SZ_MAX (1GB).
	 *   DRAM_WIN1 will translate the rest address
	 *   continueously.
	 * - DRAM_WIN1 has to be remapped to the start
	 *   address of the second chip if the chip's
	 *   capacity is more than CM3_DRAM_WIN0_SZ_MAX
	 *   (1GB). In this case, only the first 1GB on
	 *   CS1 is available for the memory test while
	 *   only first 512MB on CS2 is available for
	 *   the memory test.
	 *
	 */
	ddr_para.cs_wins[0].base = CM3_DRAM_WIN0_BASE;
	ddr_para.cs_wins[0].size =\
		(_MB(map.cs[0].capacity) > CM3_DRAM_WIN0_SZ_MAX) ?\
		CM3_DRAM_WIN0_SZ_MAX : _MB(map.cs[0].capacity);

	if (map.cs_num > 1) {
		if (_MB(map.cs[0].capacity) < CM3_DRAM_WIN0_SZ_MAX) {
			ddr_para.cs_wins[1].base = CM3_DRAM_WIN0_BASE +\
				_MB(map.cs[0].capacity);
			/*
			 * Though both DRAM_WIN0 and DRAM_WIN1 are used for
			 * CS1's address translation, the memory space on
			 * CS1 may not be fully provisioned if the total
			 * memory size is larger than the combined maximum
			 * size of both windows.
			 */
			ddr_para.cs_wins[1].size = MIN(_MB(map.cs[1].capacity),\
				(CM3_DRAM_WIN0_SZ_MAX + CM3_DRAM_WIN1_SZ_MAX -\
				_MB(map.cs[0].capacity)));

			/*
			 * Remap DRAM_WIN1 to the maximum address of DRAM_WIN0
			 * so that it can continue the translation of the on-
			 * going addresses which is beyond DRAM_WIN0.
			 */
			set_cm3_win_remap(CM3_DRAM_WIN1_ID, CM3_DRAM_WIN0_SZ_MAX);
		} else {
			ddr_para.cs_wins[1].base = CM3_DRAM_WIN1_BASE;
			/*
			 * In case of enlarging the size of DRAM_WIN1, CS1's
			 * accessible memory size cannot exceed its maximum
			 * size.
			 */
			ddr_para.cs_wins[1].size = MIN(_MB(map.cs[1].capacity),\
				CM3_DRAM_WIN1_SZ_MAX);

			/*
			 * DRAM_WIN0 is fully occupied by CS0. Even it cannot
			 * cover the entire address space on CS0, DRAM_WIN1
			 * has to be remapped to the start address of CS1 in
			 * order to guarantee at least 512MB accessible memory
			 * on CS1.
			 */
			set_cm3_win_remap(CM3_DRAM_WIN1_ID, _MB(map.cs[0].capacity));
		}
	}

	ddr_debug("\nDRAM windows:\n");
	ddr_debug("=============\n");
	ddr_debug("WIN[0] - base addr     0x%08x\n", CM3_DRAM_WIN0_BASE);
	ddr_debug("WIN[0] - size          0x%08x\n", CM3_DRAM_WIN0_SZ_MAX);
	if (map.cs_num > 1) {
		ddr_debug("WIN[1] - base addr     0x%08x\n", CM3_DRAM_WIN1_BASE);
		ddr_debug("WIN[1] - size          0x%08x\n", CM3_DRAM_WIN1_SZ_MAX);
		if (_MB(map.cs[0].capacity) > CM3_DRAM_WIN0_SZ_MAX)
			ddr_debug("WIN[1] - remap addr    0x%08x\n",
				_MB(map.cs[0].capacity));
	}

	ddr_debug("\nmemory test region:\n");
	ddr_debug("===================\n");
	ddr_debug("CS[0]                  0x%08x - 0x%08x\n",
		ddr_para.cs_wins[0].base,
		ddr_para.cs_wins[0].base + ddr_para.cs_wins[0].size - 1);
	if (map.cs_num > 1)
		ddr_debug("CS[1]                  0x%08x - 0x%08x\n",
			ddr_para.cs_wins[1].base,
			ddr_para.cs_wins[1].base + ddr_para.cs_wins[1].size - 1);

	/*
	* Use reserved settings if warm boot is found, otherwise, because ddr init process
	* may access DRAM memory, store the result in sram first, and copy to reserved dram
	* after init_ddr function
	*/
	if (ddr_para.warm_boot)
		init_ddr(ddr_para, result_in_dram);
	else
		init_ddr(ddr_para, &result_in_sram);

	/* Copy tuning result to reserved memory */
	if (!ddr_para.warm_boot) {
		memcpy(result_in_dram, &result_in_sram, sizeof(struct ddr_init_result));
		*((u32 *)(DDR_TUNE_RESULT_MEM_BASE + sizeof(struct ddr_init_result))) =
			do_checksum32((u32 *)&result_in_sram, sizeof(struct ddr_init_result));
	}

	kick_ap();

	return 0;
}

/***************************************************************************************************
  * exception_handler
  *
  * return: None
 ***************************************************************************************************/
static void exception_handler(int exception)
{
	return;
}

int main(int exception, char **dummy)
{
	u32     status;

	if (exception != 0) {
		exception_handler(exception);
		return NO_ERROR;
	}

	/* Initialization stuff */
	status = clock_init();
	if (status)
		return status;

	status = uart_init(115200);
	if (status)
		return status;
	init_printf(NULL, uart_putc);
	printf("WTMI%s\n", VERSION);

	/* Call ddrgen library */
	sys_init_main();

	/*
	* Now WTMI running image(fuse.bin or dummy.bin or freeRTOS image) and
	* system initialization image(sys_init.bin) are merged into one image -
	* "wtmi.bin", WTMI running image is aligned up to 16 bytes to be the
	* first part while sys_init.bin is the second part; wtmi.bin is loaded
	* to SRAM start address 0x1fff0000. CM3 executing start address is
	* sys_init.bin start address; after sys_init finishes the
	* initialization work, PC address will jump back to WTMI runing image
	* start address 0x1fff0000.
	*/
	__asm__ volatile("bl 0x1fff0000\n");

	return NO_ERROR;
}

