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
#elif defined(CONFIG_A3700)
#include "mv_ddr_a3700_wrapper.h"
#endif

#include "mv_ddr_mc6.h"
#include "mv_ddr_topology.h"
#include "mv_ddr_common.h"
#include "mv_ddr_training_db.h"
#include "ddr_topology_def.h"

/* bank address switch boundary */
#define MV_DDR_BANK_MAP_OFFS	24
#define MV_DDR_BANK_MAP_MASK	0x1f
enum mv_ddr_bank_map {
	MV_DDR_BANK_MAP_512B,
	MV_DDR_BANK_MAP_1KB,
	MV_DDR_BANK_MAP_2KB,
	MV_DDR_BANK_MAP_4KB,
	MV_DDR_BANK_MAP_8KB,
	MV_DDR_BANK_MAP_16KB,
	MV_DDR_BANK_MAP_32KB,
	MV_DDR_BANK_MAP_64KB,
	MV_DDR_BANK_MAP_128KB,
	MV_DDR_BANK_MAP_256KB,
	MV_DDR_BANK_MAP_512KB,
	MV_DDR_BANK_MAP_1MB,
	MV_DDR_BANK_MAP_2MB,
	MV_DDR_BANK_MAP_4MB,
	MV_DDR_BANK_MAP_8MB,
	MV_DDR_BANK_MAP_16MB,
	MV_DDR_BANK_MAP_32MB,
	MV_DDR_BANK_MAP_64MB,
	MV_DDR_BANK_MAP_128MB,
	MV_DDR_BANK_MAP_256MB,
	MV_DDR_BANK_MAP_512MB,
	MV_DDR_BANK_MAP_1GB,
	MV_DDR_BANK_MAP_2GB,
	MV_DDR_BANK_MAP_4GB,
	MV_DDR_BANK_MAP_8GB,
	MV_DDR_BANK_MAP_16GB,
	MV_DDR_BANK_MAP_32GB,
	MV_DDR_BANK_MAP_64GB,
	MV_DDR_BANK_MAP_LAST
};

static void mv_ddr_mc6_timing_regs_cfg(unsigned int mc6_base, unsigned int freq_mhz)
{
	struct mv_ddr_mc6_timing mc6_timing;
	unsigned int page_size;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);

	/* get the spped bin index */
	enum mv_ddr_speed_bin speed_bin_index = iface_params->speed_bin_index;

	/* calculate memory size */
	enum mv_ddr_die_capacity memory_size = iface_params->memory_size;

	/* calculate page size */
	page_size = mv_ddr_page_size_get(iface_params->bus_width, memory_size);
	/* printf("page_size = %d\n", page_size); */

	/* calculate t_clck */
	mc6_timing.t_ckclk = MEGA / freq_mhz;
	/* printf("t_ckclk = %d\n", mc6_timing.t_ckclk); */

	/* calculate t_refi  */
	mc6_timing.t_refi = (iface_params->interface_temp == MV_DDR_TEMP_HIGH) ? TREFI_HIGH : TREFI_LOW;

	/* the t_refi is in nsec */
	mc6_timing.t_refi = mc6_timing.t_refi / (MEGA / FCLK_KHZ);
	/* printf("t_refi = %d\n", mc6_timing.t_refi); */
	mc6_timing.t_wr = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TWR);
	/* printf("t_wr = %d\n", mc6_timing.t_wr); */
	mc6_timing.t_wr = time_to_nclk(mc6_timing.t_wr, mc6_timing.t_ckclk);
	/* printf("t_wr = %d\n", mc6_timing.t_wr); */

	/* calculate t_rrd */
	mc6_timing.t_rrd = (page_size == 1) ? mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TRRD1K) :
		mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TRRD2K);
	/* printf("t_rrd = %d\n", mc6_timing.t_rrd); */
	mc6_timing.t_rrd = GET_MAX_VALUE(mc6_timing.t_ckclk * 4, mc6_timing.t_rrd);
	/* printf("t_rrd = %d\n", mc6_timing.t_rrd); */
	mc6_timing.t_rrd = time_to_nclk(mc6_timing.t_rrd, mc6_timing.t_ckclk);
	/* printf("t_rrd = %d\n", mc6_timing.t_rrd); */

	/* calculate t_faw */
	if (page_size == 1) {
		mc6_timing.t_faw = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TFAW1K);
		/* printf("t_faw = %d\n", mc6_timing.t_faw); */
		mc6_timing.t_faw = GET_MAX_VALUE(mc6_timing.t_ckclk * 20, mc6_timing.t_faw);
		mc6_timing.t_faw = time_to_nclk(mc6_timing.t_faw, mc6_timing.t_ckclk);
		/* printf("t_faw = %d\n", mc6_timing.t_faw); */
	} else {	/* page size =2, we do not support page size 0.5k */
		mc6_timing.t_faw = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TFAW2K);
		/* printf("t_faw = %d\n", mc6_timing.t_faw); */
		mc6_timing.t_faw = GET_MAX_VALUE(mc6_timing.t_ckclk * 28, mc6_timing.t_faw);
		mc6_timing.t_faw = time_to_nclk(mc6_timing.t_faw, mc6_timing.t_ckclk);
		/* printf("t_faw = %d\n", mc6_timing. t_faw); */
	}

	/* calculate t_rtp */
	mc6_timing.t_rtp = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TRTP);
	/* printf("t_rtp = %d\n", mc6_timing.t_rtp); */
	mc6_timing.t_rtp = GET_MAX_VALUE(mc6_timing.t_ckclk * 4, mc6_timing.t_rtp);
	/* printf("t_rtp = %d\n", mc6_timing.t_rtp); */
	mc6_timing.t_rtp = time_to_nclk(mc6_timing.t_rtp, mc6_timing.t_ckclk);
	/* printf("t_rtp = %d\n", mc6_timing.t_rtp); */

	/* calculate t_mode */
	mc6_timing.t_mod = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TMOD);
	/* printf("t_mod = %d\n", mc6_timing.t_mod); */
#ifdef CONFIG_DDR4
	mc6_timing.t_mod = GET_MAX_VALUE(mc6_timing.t_ckclk * 24, mc6_timing.t_mod);
#else /* CONFIG_DDR3 */
	mc6_timing.t_mod = GET_MAX_VALUE(mc6_timing.t_ckclk * 12, mc6_timing.t_mod);
#endif
	/* printf("t_mod = %d\n", mc6_timing.t_mod); */
	mc6_timing.t_mod = time_to_nclk(mc6_timing.t_mod, mc6_timing.t_ckclk);
	/* printf("t_mod = %d\n",mc6_timing. t_mod); */

	/* calculate t_wtr */
	mc6_timing.t_wtr = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TWTR);
	/* printf("t_wtr = %d\n", mc6_timing.t_wtr); */
	mc6_timing.t_wtr = GET_MAX_VALUE(mc6_timing.t_ckclk * 2, mc6_timing.t_wtr);
	/* printf("t_wtr = %d\n", mc6_timing.t_wtr); */
	mc6_timing.t_wtr = time_to_nclk(mc6_timing.t_wtr, mc6_timing.t_ckclk);
	/* printf("t_wtr = %d\n", mc6_timing.t_wtr); */

#ifdef CONFIG_DDR4
	/* calculate t_wtr_l */
	mc6_timing.t_wtr_l = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TWTRL);
	/* printf("t_wtr_l = %d\n", mc6_timing.t_wtr_l); */
	mc6_timing.t_wtr_l = GET_MAX_VALUE(mc6_timing.t_ckclk * 4, mc6_timing.t_wtr_l);
	/* printf("t_wtr_l = %d\n", mc6_timing.t_wtr_l); */
	mc6_timing.t_wtr_l = time_to_nclk(mc6_timing.t_wtr_l, mc6_timing.t_ckclk);
	/* printf("t_wtr_l = %d\n", mc6_timing.t_wtr_l); */
#endif

	/* calculate t_xp */
	mc6_timing.t_xp = TIMING_T_XP;
	/* printf("t_xp = %d\n", mc6_timing.t_xp); */
	mc6_timing.t_xp = GET_MAX_VALUE(mc6_timing.t_ckclk * 4, mc6_timing.t_xp);
	/* printf("t_xp = %d\n", mc6_timing.t_xp); */
	mc6_timing.t_xp = time_to_nclk(mc6_timing.t_xp, mc6_timing.t_ckclk);
	/* printf("t_xp = %d\n", mc6_timing.t_xp); */

#ifndef CONFIG_DDR4 /* CONFIG_DDR3 */
	/* calculate t_xpdll */
	mc6_timing.t_xpdll = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TXPDLL);
	mc6_timing.t_xpdll = GET_MAX_VALUE(mc6_timing.t_ckclk * 10, mc6_timing.t_xpdll);
	mc6_timing.t_xpdll = time_to_nclk(mc6_timing.t_xpdll, mc6_timing.t_ckclk);
#endif

	/* calculate t_cke */
	mc6_timing.t_cke = TIMING_T_CKE;
	/* printf("t_cke = %d\n", mc6_timing.t_cke); */
	mc6_timing.t_cke = GET_MAX_VALUE(mc6_timing.t_ckclk * 3, mc6_timing.t_cke);
	/* printf("t_cke = %d\n", mc6_timing.t_cke); */
	mc6_timing.t_cke = time_to_nclk(mc6_timing.t_cke, mc6_timing.t_ckclk);
	/* printf("t_cke = %d\n", mc6_timing.t_cke); */

	/* calculate t_ckesr */
	mc6_timing.t_ckesr = mc6_timing.t_cke + 1;
	/* printf("t_ckesr = %d\n", mc6_timing.t_ckesr); */

	/* calculate t_cpded */
#ifdef CONFIG_DDR4
	mc6_timing.t_cpded = 4;
#else /* CONFIG_DDR3 */
	mc6_timing.t_cpded = 1;
#endif
	/* printf("t_cpded = %d\n", mc6_timing.t_cpded); */

	/* calculate t_cksrx */
	mc6_timing.t_cksrx = TIMING_T_CKSRX;
	/* printf("t_cksrx = %d\n", mc6_timing.t_cksrx); */
	mc6_timing.t_cksrx = GET_MAX_VALUE(mc6_timing.t_ckclk * 5, mc6_timing.t_cksrx);
	/* printf("t_cksrx = %d\n", mc6_timing.t_cksrx); */
	mc6_timing.t_cksrx = time_to_nclk(mc6_timing.t_cksrx, mc6_timing.t_ckclk);
	/* printf("t_cksrx = %d\n", mc6_timing.t_cksrx); */

	/* calculate t_cksre */
	mc6_timing.t_cksre = mc6_timing.t_cksrx;
	/* printf("t_cksre = %d\n", mc6_timing.t_cksre); */

	/* calculate t_ras */
	mc6_timing.t_ras = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TRAS);
	/* printf("t_ras = %d\n", mc6_timing.t_ras); */
	mc6_timing.t_ras = time_to_nclk(mc6_timing.t_ras, mc6_timing.t_ckclk);
	/* printf("t_ras = %d\n", mc6_timing.t_ras); */

	/* calculate t_rcd */
	mc6_timing.t_rcd = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TRCD);
	/* printf("t_rcd = %d\n", mc6_timing.t_rcd); */
	mc6_timing.t_rcd = time_to_nclk(mc6_timing.t_rcd, mc6_timing.t_ckclk);
	/* printf("t_rcd = %d\n", mc6_timing.t_rcd); */

	/* calculate t_rp */
	mc6_timing.t_rp = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TRP);
	/* printf("t_rp = %d\n", mc6_timing.t_rp); */
	mc6_timing.t_rp = time_to_nclk(mc6_timing.t_rp, mc6_timing.t_ckclk);
	/* printf("t_rp = %d\n", mc6_timing.t_rp); */

	/*calculate t_rfc */
	mc6_timing.t_rfc = time_to_nclk(mv_ddr_rfc_get(memory_size) * 1000, mc6_timing.t_ckclk);
	/* printf("t_rfc = %d\n", mc6_timing.t_rfc); */

	/* calculate t_xs */
	mc6_timing.t_xs = mc6_timing.t_rfc + time_to_nclk(TIMING_T_XS_OVER_TRFC, mc6_timing.t_ckclk);
	/* printf("t_xs = %d\n", mc6_timing.t_xs); */

#ifdef CONFIG_DDR4
	/* calculate t_rrd_l */
	mc6_timing.t_rrd_l = (page_size == 1) ? mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TRRDL1K) :
						mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TRRDL2K);
	/* printf("t_rrd_l = %d\n", mc6_timing.t_rrd_l); */
	mc6_timing.t_rrd_l = GET_MAX_VALUE(mc6_timing.t_ckclk * 4, mc6_timing.t_rrd_l);
	/* printf("t_rrd_l = %d\n", mc6_timing. t_rrd_l); */
	mc6_timing.t_rrd_l = time_to_nclk(mc6_timing.t_rrd_l, mc6_timing.t_ckclk);
	/* printf("t_rrd_l = %d\n", mc6_timing.t_rrd_l); */

	/* calculate t_ccd_l */
	mc6_timing.t_ccd_l = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TCCDL);
	mc6_timing.t_ccd_l = GET_MAX_VALUE(mc6_timing.t_ckclk * 5, mc6_timing.t_ccd_l);
	mc6_timing.t_ccd_l = time_to_nclk(mc6_timing.t_ccd_l, mc6_timing.t_ckclk);
	/* printf("t_ccd_l = %d\n", mc6_timing.t_ccd_l); */
#endif

	/* calculate t_rc */
	mc6_timing.t_rc = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TRC);
	/* printf("t_rc = %d\n", mc6_timing.t_rc); */
	mc6_timing.t_rc = time_to_nclk(mc6_timing.t_rc, mc6_timing.t_ckclk);
	/* printf("t_rc = %d\n", mc6_timing.t_rc); */

	/* constant timing parameters */
	mc6_timing.read_gap_extend = TIMING_READ_GAP_EXTEND;
	/* printf("read_gap_extend = %d\n", mc6_timing.read_gap_extend); */

	mc6_timing.t_res = TIMING_T_RES;
	/* printf("t_res = %d\n", mc6_timing.t_res); */
	mc6_timing.t_res = time_to_nclk(mc6_timing.t_res, mc6_timing.t_ckclk);
	/* printf("t_res = %d\n", mc6_timing.t_res); */

	mc6_timing.t_resinit = TIMING_T_RESINIT;
	/* printf("t_resinit = %d\n", mc6_timing.t_resinit); */
	mc6_timing.t_resinit = time_to_nclk(mc6_timing.t_resinit, mc6_timing.t_ckclk);
	/* printf("t_resinit = %d\n", mc6_timing.t_resinit); */

	mc6_timing.t_restcke = TIMING_T_RESTCKE;
	/* printf("t_restcke = %d\n", mc6_timing.t_restcke); */
	mc6_timing.t_restcke = time_to_nclk(mc6_timing.t_restcke, mc6_timing.t_ckclk);
	/* printf("t_restcke = %d\n", mc6_timing.t_restcke); */

	mc6_timing.t_actpden = TIMING_T_ACTPDEN;
	/* printf("t_actpden = %d\n", mc6_timing.t_actpden); */

#ifdef CONFIG_DDR4
	mc6_timing.t_zqinit = TIMING_T_ZQINIT;
	mc6_timing.t_zqoper = TIMING_T_ZQOPER;
	mc6_timing.t_zqcs = TIMING_T_ZQCS;
#else /* CONFIG_DDR3 */
	mc6_timing.t_zqinit = TIMING_T_ZQINIT;
	mc6_timing.t_zqinit = GET_MAX_VALUE(mc6_timing.t_ckclk * 512, mc6_timing.t_zqinit);
	mc6_timing.t_zqinit = time_to_nclk(mc6_timing.t_zqinit, mc6_timing.t_ckclk);

	/* tzqoper is 1/2 of tzqinit per jedec spec */
	mc6_timing.t_zqoper = mc6_timing.t_zqinit / 2;
	/* tzqcs is 1/8 of tzqinit per jedec spec */
	mc6_timing.t_zqcs = mc6_timing.t_zqinit / 8;
#endif
	/* printf("t_zqinit = %d\n", mc6_timing.t_zqinit); */
	/* printf("t_zqoper = %d\n", mc6_timing.t_zqoper); */
	/* printf("t_zqcs = %d\n", mc6_timing.t_zqcs); */

	mc6_timing.t_ccd = TIMING_T_CCD;
	/* printf("t_ccd = %d\n", mc6_timing.t_ccd); */

	mc6_timing.t_mrd = TIMING_T_MRD;
	/* printf("t_mrd = %d\n", mc6_timing.t_mrd); */

	mc6_timing.t_mpx_lh = TIMING_T_MPX_LH;
	/* printf("t_mpx_lh = %d\n", mc6_timing.t_mpx_lh); */

	mc6_timing.t_mpx_s = TIMING_T_MPX_S;
	/* printf("t_mpx_s = %d\n", mc6_timing.t_mpx_s); */

	mc6_timing.t_xmp = mc6_timing.t_rfc + time_to_nclk(TIMING_T_XMP_OVER_TRFC, mc6_timing.t_ckclk);
	/* printf("t_xmp = %d\n", mc6_timing.t_xmp); */

	mc6_timing.t_mrd_pda = TIMING_T_MRD_PDA;
	/* printf("t_mrd_pda = %d\n", mc6_timing.t_mrd_pda); */
	mc6_timing.t_mrd_pda = GET_MAX_VALUE(mc6_timing.t_ckclk * 16, mc6_timing.t_mrd_pda);
	/* printf("t_mrd_pda = %d\n", mc6_timing. t_mrd_pda); */
	mc6_timing.t_mrd_pda = time_to_nclk(mc6_timing.t_mrd_pda, mc6_timing.t_ckclk);
	/* printf("t_mrd_pda = %d\n", mc6_timing. t_mrd_pda); */

	mc6_timing.t_xsdll = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TXSDLL);
	/* printf("t_xsdll = %d\n", mc6_timing.t_xsdll); */

	mc6_timing.t_rwd_ext_dly = TIMING_T_RWD_EXT_DLY;
	/* printf("t_rwd_ext_dly = %d\n", mc6_timing.t_rwd_ext_dly); */

	mc6_timing.t_wl_early = TIMING_T_WL_EARLY;
	/* printf("t_wl_early = %d\n", mc6_timing.t_wl_early); */

	mc6_timing.t_ccd_ccs_wr_ext_dly = TIMING_T_CCD_CCS_WR_EXT_DLY;
	/* printf("t_ccd_ccs_wr_ext_dly = %d\n", mc6_timing.t_ccd_ccs_wr_ext_dly); */

	mc6_timing.t_ccd_ccs_ext_dly = TIMING_T_CCD_CCS_EXT_DLY;
	/* printf("t_ccd_ccs_ext_dly = %d\n", mc6_timing.t_ccd_ccs_ext_dly); */

	mc6_timing.cl = iface_params->cas_l;
	mc6_timing.cwl = iface_params->cas_wl;

	/* configure the timing registers */
	reg_bit_clrset(mc6_base + MC6_CH0_DRAM_CFG1_REG,
		       mc6_timing.cwl << CWL_OFFS | mc6_timing.cl << CL_OFFS,
		       CWL_MASK << CWL_OFFS | CL_MASK << CL_OFFS);
	/* printf("MC6_CH0_DRAM_CFG1_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_DRAM_CFG1_REG,
	       reg_read(mc6_base + MC6_CH0_DRAM_CFG1_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_DDR_INIT_TIMING_CTRL0_REG,
		       mc6_timing.t_restcke << INIT_COUNT_NOP_OFFS,
		       INIT_COUNT_NOP_MASK << INIT_COUNT_NOP_OFFS);
	/* printf("MC6_CH0_DDR_INIT_TIMING_CTRL0_REG addr 0x%x, data 0x%x\n",
	** mc6_base + MC6_CH0_DDR_INIT_TIMING_CTRL0_REG,
	** reg_read(mc6_base + MC6_CH0_DDR_INIT_TIMING_CTRL0_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_DDR_INIT_TIMING_CTRL1_REG,
		       mc6_timing.t_resinit << INIT_COUNT_OFFS,
		       INIT_COUNT_MASK << INIT_COUNT_OFFS);
	/* printf("MC6_CH0_DDR_INIT_TIMING_CTRL1_REG addr 0x%x, data 0x%x\n",
	** mc6_base + MC6_CH0_DDR_INIT_TIMING_CTRL1_REG,
	** reg_read(mc6_base + MC6_CH0_DDR_INIT_TIMING_CTRL1_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_DDR_INIT_TIMING_CTRL2_REG,
		       mc6_timing.t_res << RESET_COUNT_OFFS,
		       RESET_COUNT_MASK << RESET_COUNT_OFFS);
	/* printf("MC6_CH0_DDR_INIT_TIMING_CTRL2_REG addr 0x%x, data 0x%x\n",
	** mc6_base + MC6_CH0_DDR_INIT_TIMING_CTRL2_REG,
	** reg_read(mc6_base + MC6_CH0_DDR_INIT_TIMING_CTRL2_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_ZQC_TIMING0_REG,
		       mc6_timing.t_zqinit << TZQINIT_OFFS,
		       TZQINIT_MASK << TZQINIT_OFFS);
	/* printf("MC6_CH0_ZQC_TIMING0_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_ZQC_TIMING0_REG,
	       reg_read(mc6_base + MC6_CH0_ZQC_TIMING0_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_ZQC_TIMING1_REG,
		       mc6_timing.t_zqoper << TZQCL_OFFS |
		       mc6_timing.t_zqcs << TZQCS_OFFS,
		       TZQCL_MASK << TZQCL_OFFS |
		       TZQCS_MASK << TZQCS_OFFS);
	/* printf("MC6_CH0_ZQC_TIMING1_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_ZQC_TIMING1_REG,
	       reg_read(mc6_base + MC6_CH0_ZQC_TIMING1_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_REFRESH_TIMING_REG,
		       mc6_timing.t_refi << TREFI_OFFS |
		       mc6_timing.t_rfc << TRFC_OFFS,
		       TREFI_MASK << TREFI_OFFS |
		       TRFC_MASK << TRFC_OFFS);
	/* printf("MC6_CH0_REFRESH_TIMING_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_REFRESH_TIMING_REG,
	       reg_read(mc6_base + MC6_CH0_REFRESH_TIMING_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_SELFREFRESH_TIMING0_REG,
		       mc6_timing.t_xsdll << TXSRD_OFFS |
		       mc6_timing.t_xs << TXSNR_OFFS,
		       TXSRD_MASK << TXSRD_OFFS |
		       TXSNR_MASK << TXSNR_OFFS);
	/* printf("MC6_CH0_SELFREFRESH_TIMING0_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_SELFREFRESH_TIMING0_REG,
	       reg_read(mc6_base + MC6_CH0_SELFREFRESH_TIMING0_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_SELFREFRESH_TIMING1_REG,
		       mc6_timing.t_cksrx << TCKSRX_OFFS |
		       mc6_timing.t_cksre << TCKSRE_OFFS,
		       TCKSRX_MASK << TCKSRX_OFFS |
		       TCKSRE_MASK << TCKSRE_OFFS);
	/* printf("MC6_CH0_SELFREFRESH_TIMING1_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_SELFREFRESH_TIMING1_REG,
	       reg_read(mc6_base + MC6_CH0_SELFREFRESH_TIMING1_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_PWRDOWN_TIMING0_REG,
#ifdef CONFIG_DDR4
		       TXARDS_VAL << TXARDS_OFFS |
#else /* CONFIG_DDR3 */
		       mc6_timing.t_xpdll << TXARDS_OFFS |
#endif
		       mc6_timing.t_xp << TXP_OFFS |
		       mc6_timing.t_ckesr << TCKESR_OFFS |
		       mc6_timing.t_cpded << TCPDED_OFFS,
		       TXARDS_MASK << TXARDS_OFFS |
		       TXP_MASK << TXP_OFFS |
		       TCKESR_MASK << TCKESR_OFFS |
		       TCPDED_MASK << TCPDED_OFFS);
	/* printf("MC6_CH0_PWRDOWN_TIMING0_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_PWRDOWN_TIMING0_REG,
	       reg_read(mc6_base + MC6_CH0_PWRDOWN_TIMING0_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_PWRDOWN_TIMING1_REG,
		       mc6_timing.t_actpden << TPDEN_OFFS,
		       TPDEN_MASK << TPDEN_OFFS);
	/* printf("MC6_CH0_PWRDOWN_TIMING1_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_PWRDOWN_TIMING1_REG,
	       reg_read(mc6_base + MC6_CH0_PWRDOWN_TIMING1_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_MRS_TIMING_REG,
		       mc6_timing.t_mrd << TMRD_OFFS |
		       mc6_timing.t_mod << TMOD_OFFS,
		       TMRD_MASK << TMRD_OFFS |
		       TMOD_MASK << TMOD_OFFS);
	/* printf("MC6_CH0_MRS_TIMING_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_MRS_TIMING_REG,
	       reg_read(mc6_base + MC6_CH0_MRS_TIMING_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_ACT_TIMING_REG,
		       mc6_timing.t_ras << TRAS_OFFS |
		       mc6_timing.t_rcd << TRCD_OFFS |
		       mc6_timing.t_rc << TRC_OFFS |
		       mc6_timing.t_faw << TFAW_OFFS,
		       TRAS_MASK << TRAS_OFFS |
		       TRCD_MASK << TRCD_OFFS |
		       TRC_MASK << TRC_OFFS |
		       TFAW_MASK << TFAW_OFFS);
	/* printf("MC6_CH0_ACT_TIMING_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_ACT_TIMING_REG,
	       reg_read(mc6_base + MC6_CH0_ACT_TIMING_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_PRECHARGE_TIMING_REG,
		       mc6_timing.t_rp << TRP_OFFS |
		       mc6_timing.t_rtp << TRTP_OFFS |
		       mc6_timing.t_wr << TWR_OFFS |
		       mc6_timing.t_rp << TRPA_OFFS,
		       TRP_MASK << TRP_OFFS |
		       TRTP_MASK << TRTP_OFFS |
		       TWR_MASK << TWR_OFFS |
		       TRPA_MASK << TRPA_OFFS);
	/* printf("MC6_CH0_PRECHARGE_TIMING_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_PRECHARGE_TIMING_REG,
	       reg_read(mc6_base + MC6_CH0_PRECHARGE_TIMING_REG)); */

#ifdef CONFIG_DDR4
	reg_bit_clrset(mc6_base + MC6_CH0_CAS_RAS_TIMING0_REG,
		       mc6_timing.t_wtr << TWTR_S_OFFS |
		       mc6_timing.t_wtr_l << TWTR_OFFS |
		       mc6_timing.t_ccd << TCCD_S_OFFS |
		       mc6_timing.t_ccd_l << TCCD_OFFS,
		       TWTR_S_MASK << TWTR_S_OFFS |
		       TWTR_MASK << TWTR_OFFS |
		       TCCD_S_MASK << TCCD_S_OFFS |
		       TCCD_MASK << TCCD_OFFS);
#else /* CONFIG_DDR3 */
	reg_bit_clrset(mc6_base + MC6_CH0_CAS_RAS_TIMING0_REG,
		       mc6_timing.t_wtr << TWTR_OFFS |
		       mc6_timing.t_ccd << TCCD_OFFS,
		       TWTR_MASK << TWTR_OFFS |
		       TCCD_MASK << TCCD_OFFS);
#endif
	/* printf("MC6_CH0_CAS_RAS_TIMING0_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_CAS_RAS_TIMING0_REG,
	       reg_read(mc6_base + MC6_CH0_CAS_RAS_TIMING0_REG)); */

#ifdef CONFIG_DDR4
	/* TODO: check why change default of 17:16 tDQS2DQ from '1' to '0' */
	reg_bit_clrset(mc6_base + MC6_CH0_CAS_RAS_TIMING1_REG,
		       mc6_timing.t_rrd << TRRD_S_OFFS |
		       mc6_timing.t_rrd_l << TRRD_OFFS |
		       TDQS2DQ_VAL << TDQS2DQ_OFFS,
		       TRRD_S_MASK << TRRD_S_OFFS |
		       TRRD_MASK << TRRD_OFFS |
		       TDQS2DQ_MASK << TDQS2DQ_OFFS);
#else /* CONFIG_DDR3 */
	reg_bit_clrset(mc6_base + MC6_CH0_CAS_RAS_TIMING1_REG,
		       mc6_timing.t_rrd << TRRD_OFFS |
		       TDQS2DQ_VAL << TDQS2DQ_OFFS,
		       TRRD_MASK << TRRD_OFFS |
		       TDQS2DQ_MASK << TDQS2DQ_OFFS);
#endif
	/* printf("MC6_CH0_CAS_RAS_TIMING1_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_CAS_RAS_TIMING1_REG,
	       reg_read(mc6_base + MC6_CH0_CAS_RAS_TIMING1_REG)); */

#ifndef CONFIG_A3700
	reg_bit_clrset(mc6_base + MC6_CH0_OFF_SPEC_TIMING0_REG,
		       mc6_timing.t_ccd_ccs_ext_dly << TCCD_CCS_EXT_DLY_OFFS |
		       mc6_timing.t_ccd_ccs_wr_ext_dly << TCCD_CCS_WR_EXT_DLY_OFFS |
		       mc6_timing.t_rwd_ext_dly << TRWD_EXT_DLY_OFFS |
		       mc6_timing.t_wl_early << TWL_EARLY_OFFS,
		       TCCD_CCS_EXT_DLY_MASK << TCCD_CCS_EXT_DLY_OFFS |
		       TCCD_CCS_WR_EXT_DLY_MASK << TCCD_CCS_WR_EXT_DLY_OFFS |
		       TRWD_EXT_DLY_MASK << TRWD_EXT_DLY_OFFS |
		       TWL_EARLY_MASK << TWL_EARLY_OFFS);
	/* printf("MC6_CH0_OFF_SPEC_TIMING0_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_OFF_SPEC_TIMING0_REG,
	       reg_read(mc6_base + MC6_CH0_OFF_SPEC_TIMING0_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_OFF_SPEC_TIMING1_REG,
		       mc6_timing.read_gap_extend << READ_GAP_EXTEND_OFFS |
		       mc6_timing.t_ccd_ccs_ext_dly << TCCD_CCS_EXT_DLY_MIN_OFFS |
		       mc6_timing.t_ccd_ccs_wr_ext_dly << TCCD_CCS_WR_EXT_DLY_MIN_OFFS,
		       READ_GAP_EXTEND_MASK << READ_GAP_EXTEND_OFFS |
		       TCCD_CCS_EXT_DLY_MIN_MASK << TCCD_CCS_EXT_DLY_MIN_OFFS |
		       TCCD_CCS_WR_EXT_DLY_MIN_MASK << TCCD_CCS_WR_EXT_DLY_MIN_OFFS);
	/* printf("MC6_CH0_OFF_SPEC_TIMING1_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_OFF_SPEC_TIMING1_REG,
	       reg_read(mc6_base + MC6_CH0_OFF_SPEC_TIMING1_REG)); */

	/* TODO: check why change default of 3:0 tDQSCK from '3' to '0' */
	reg_bit_clrset(mc6_base + MC6_CH0_DRAM_READ_TIMING_REG,
		       TDQSCK_VAL << TDQSCK_OFFS,
		       TDQSCK_MASK << TDQSCK_OFFS);
	/* printf("MC6_CH0_DRAM_READ_TIMING_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_DRAM_READ_TIMING_REG,
	       reg_read(mc6_base + MC6_CH0_DRAM_READ_TIMING_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_DRAM_MPD_TIMING_REG,
		       mc6_timing.t_xmp << TXMP_OFFS |
		       mc6_timing.t_mpx_s << TMPX_S_OFFS |
		       mc6_timing.t_mpx_lh << TMPX_LH_OFFS,
		       TXMP_MASK << TXMP_OFFS |
		       TMPX_S_MASK << TMPX_S_OFFS |
		       TMPX_LH_MASK << TMPX_LH_OFFS);
	/* printf("MC6_CH0_DRAM_MPD_TIMING_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_DRAM_MPD_TIMING_REG,
	       reg_read(mc6_base + MC6_CH0_DRAM_MPD_TIMING_REG)); */

	reg_bit_clrset(mc6_base + MC6_CH0_DRAM_PDA_TIMING_REG,
		       mc6_timing.t_mrd_pda << TMRD_PDA_OFFS,
		       TMRD_PDA_MASK << TMRD_PDA_OFFS);
	/* printf("MC6_CH0_DRAM_PDA_TIMING_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_DRAM_PDA_TIMING_REG,
	       reg_read(mc6_base + MC6_CH0_DRAM_PDA_TIMING_REG)); */
#endif
}

void mv_ddr_mc6_and_dram_timing_set(unsigned int mc6_base)
{
	/* get the frequency */
	u32 freq_mhz;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);

	/* get the frequency form the topology */
	freq_mhz = mv_ddr_freq_get(iface_params->memory_freq);

	mv_ddr_mc6_timing_regs_cfg(mc6_base, freq_mhz);
}

#ifdef CONFIG_MC6P
/* TODO:  get relevant parameters from topology */
static int mv_ddr_mc6_cfg_set(unsigned int mc6_base)
{
	u32 data_width = 0, bus_width;

	bus_width = mv_ddr_if_bus_width_get();
	if (bus_width == 32)
		data_width = DATA_WIDTH_X32;
	else if (bus_width == 64)
		data_width = DATA_WIDTH_X64;
	else
		return -1;
	/*
	 * configure data width, mvn and burst length (burst length is default BL8)
	 * TODO: get data width from topology
	 */
	reg_bit_clrset(mc6_base + MC6_MC_CTRL0_REG,
#ifdef CONFIG_MC6P
		       MVN_ENABLE << MVN_EN_OFFS |
#endif
		       data_width << DATA_WIDTH_OFFS,
#ifdef CONFIG_MC6P
		       MVN_EN_MASK << MVN_EN_OFFS |
#endif
		       DATA_WIDTH_MASK << DATA_WIDTH_OFFS);
	/* printf("MC6_MC_CTRL0_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_MC_CTRL0_REG,
		  reg_read(mc6_base + MC6_MC_CTRL0_REG)); */

	/* configure starvation timer order filter and auto precharge */
	reg_bit_clrset(mc6_base + MC6_SPOOL_CTRL_REG,
#ifdef CONFIG_MC6P
		       SPOOL_ORDER_FILTER_EN << SPOOL_ORDER_FILTER_EN_OFFS |
		       SPOOL_SMART_AUTO_PRECHARGE_EN << SPOOL_SMART_AUTO_PRECHARGE_EN_OFFS |
#endif
		       STARV_TIMER_VAL << STARV_TIMER_INIT_OFFS,
#ifdef CONFIG_MC6P
		       SPOOL_ORDER_FILTER_EN_MASK << SPOOL_ORDER_FILTER_EN_OFFS |
		       SPOOL_SMART_AUTO_PRECHARGE_EN_MASK << SPOOL_SMART_AUTO_PRECHARGE_EN_OFFS |
#endif
		       STARV_TIMER_INIT_MASK << STARV_TIMER_INIT_OFFS);
	/* printf("MC6_SPOOL_CTRL_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_SPOOL_CTRL_REG,
		  reg_read(mc6_base + MC6_SPOOL_CTRL_REG)); */

	/* configure share enable and share read request */
	reg_bit_clrset(mc6_base + MC6_MC_WR_BUF_CTRL_REG,
		       TIME_SHARE_EN_CH0_VAL << TIME_SHARE_EN_CH0_OFFS |
		       TIME_SHARE_RD_REQ_CH0_VAL << TIME_SHARE_RD_REQ_CH0_OFFS,
		       TIME_SHARE_EN_CH0_MASK << TIME_SHARE_EN_CH0_OFFS |
		       TIME_SHARE_RD_REQ_CH0_MASK << TIME_SHARE_RD_REQ_CH0_OFFS);
	/* printf("MC6_MC_WR_BUF_CTRL_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_MC_WR_BUF_CTRL_REG,
		  reg_read(mc6_base + MC6_MC_WR_BUF_CTRL_REG)); */

	/* configure read data latency */
	reg_bit_clrset(mc6_base + MC6_RD_DPATH_CTRL_REG,
#ifndef CONFIG_MC6P
		       MB_RD_DATA_LATENCY_CH1_VAL << MB_RD_DATA_LATENCY_CH1_OFFS |
#endif
		       MB_RD_DATA_LATENCY_CH0_VAL << MB_RD_DATA_LATENCY_CH0_OFFS,
#ifndef CONFIG_MC6P
		       MB_RD_DATA_LATENCY_CH1_MASK << MB_RD_DATA_LATENCY_CH1_OFFS |
#endif
		       MB_RD_DATA_LATENCY_CH0_MASK << MB_RD_DATA_LATENCY_CH0_OFFS);
	/* printf("MC6_RD_DPATH_CTRL_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_RD_DPATH_CTRL_REG,
		  reg_read(mc6_base + MC6_RD_DPATH_CTRL_REG)); */

	/* configure phy mask, retry mode and acs exit delay */
	reg_bit_clrset(mc6_base + MC6_CH0_MC_CTRL1_REG,
#ifdef CONFIG_MC6P
		      /*
		       * workaround preventing mrs commands to be sent
		       * to memory during mc6p initialization; done by
		       * masking out clk enable and reset signals
		       */
		       PHY_MASK_ENA << PHY_MASK_OFFS |
#else
		       PHY_MASK_DIS << PHY_MASK_OFFS |
#endif
#ifdef CONFIG_MC6P
		       RETRY_MODE_VAL << RETRY_MODE_OFFS |
#endif
		       ACS_EXIT_DLY_VAL << ACS_EXIT_DLY_OFFS,
		       PHY_MASK_MASK << PHY_MASK_OFFS |
#ifdef CONFIG_MC6P
		       RETRY_MODE_MASK << RETRY_MODE_OFFS |
#endif
		       ACS_EXIT_DLY_MASK << ACS_EXIT_DLY_OFFS);
	/* printf("MC6_CH0_MC_CTRL1_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_MC_CTRL1_REG,
		  reg_read(mc6_base + MC6_CH0_MC_CTRL1_REG)); */

	/* configure phy output and input interface timing control */
	reg_bit_clrset(mc6_base + MC6_CH0_MC_CTRL3_REG,
		       PHY_OUT_FF_BYPASS_VAL << PHY_OUT_FF_BYPASS_OFFS |
		       PHY_IN_FF_BYPASS_VAL << PHY_IN_FF_BYPASS_OFFS,
		       PHY_OUT_FF_BYPASS_MASK << PHY_OUT_FF_BYPASS_OFFS |
		       PHY_IN_FF_BYPASS_MASK << PHY_IN_FF_BYPASS_OFFS);
	/* printf("MC6_CH0_MC_CTRL3_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_MC_CTRL3_REG,
		  reg_read(mc6_base + MC6_CH0_MC_CTRL3_REG)); */

	/*
	 * configure dm according to topology
	 * TODO: get dm configuration from topology
	 */
	reg_bit_clrset(mc6_base + MC6_CH0_DRAM_CFG2_REG,
#ifndef CONFIG_MC6P
		       DM_DIS << DRAM_CFG2_DM_OFFS,
#else
		       DM_EN << DRAM_CFG2_DM_OFFS,
#endif
		       DRAM_CFG2_DM_MASK << DRAM_CFG2_DM_OFFS);
	/* printf("MC6_CH0_DRAM_CFG2_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_DRAM_CFG2_REG,
		  reg_read(mc6_base + MC6_CH0_DRAM_CFG2_REG)); */

	/* configure dll reset when sending mr commands to memory */
	reg_bit_clrset(mc6_base + MC6_CH0_DRAM_CFG3_REG,
		       DLL_RESET_VAL << DLL_RESET_OFFS,
		       DLL_RESET_MASK << DLL_RESET_OFFS);
	/* printf("MC6_CH0_DRAM_CFG3_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_DRAM_CFG3_REG,
		  reg_read(mc6_base + MC6_CH0_DRAM_CFG3_REG)); */

	/*
	 * configure dram type, mirror enable, 2t mode  rdimm support
	 * TODO: get mirror configuration from topology; currently set to cs1 always mirrored
	 * TODO: add proper configuration while working with rdimm
	 */
	reg_bit_clrset(mc6_base + MC6_CH0_MC_CTRL2_REG,
		       MODE_2T_VAL << MODE_2T_OFFS |
		       CS1_MIRROR << ADDR_MIRROR_EN_OFFS |
		       DDR4_TYPE << SDRAM_TYPE_OFFS,
		       MODE_2T_MASK << SDRAM_TYPE_OFFS |
		       ADDR_MIRROR_EN_MASK << ADDR_MIRROR_EN_OFFS |
		       SDRAM_TYPE_MASK << SDRAM_TYPE_OFFS);
	/* printf("MC6_CH0_MC_CTRL2_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_MC_CTRL2_REG,
		  reg_read(mc6_base + MC6_CH0_MC_CTRL2_REG)); */

	/* configure rpp starvation parameters */
	reg_bit_clrset(mc6_base + MC6_RPP_STARVATION_CTRL_REG,
		       BW_ALLOC_MODE_SEL_VAL << BW_ALLOC_MODE_SEL_OFFS |
		       RPP_STARVATION_EN_VAL << RPP_STARVATION_EN_OFFS |
		       RPP_STARV_TIMER_INIT_VAL << RPP_STARV_TIMER_INIT_OFFS,
		       BW_ALLOC_MODE_SEL_MASK << BW_ALLOC_MODE_SEL_OFFS |
		       RPP_STARVATION_EN_MASK << RPP_STARVATION_EN_OFFS |
		       RPP_STARV_TIMER_INIT_MASK << RPP_STARV_TIMER_INIT_OFFS);
	/* printf("MC6_RPP_STARVATION_CTRL_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_RPP_STARVATION_CTRL_REG,
		  reg_read(mc6_base + MC6_RPP_STARVATION_CTRL_REG)); */

	/* configure delays to and from power saving mode */
	reg_bit_clrset(mc6_base + MC6_MC_PWR_CTRL_REG,
		       AC_ON_DLY_VAL << AC_ON_DLY_OFFS |
		       AC_OFF_DLY_VAL << AC_OFF_DLY_OFFS,
		       AC_ON_DLY_MASK << AC_ON_DLY_OFFS |
		       AC_OFF_DLY_MASK << AC_OFF_DLY_OFFS);
	/* printf("MC6_MC_PWR_CTRL_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_MC_PWR_CTRL_REG,
		  reg_read(mc6_base + MC6_MC_PWR_CTRL_REG)); */

	/*
	 * configure dq vref value
	 * TODO: set vref configuration to training value
	 * currently set to constant value in case mrs commands are sent to memory
	 * TODO: double-check this write with design
	 */
	reg_bit_clrset(mc6_base + MC6_CH0_DRAM_CFG4_REG,
		       VREF_TRAINING_VALUE_DQ_VAL << VREF_TRAINING_VALUE_DQ_OFFS,
		       VREF_TRAINING_VALUE_DQ_MASK << VREF_TRAINING_VALUE_DQ_OFFS);
	/* printf("MC6_CH0_DRAM_CFG4_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_CH0_DRAM_CFG4_REG,
		  reg_read(mc6_base + MC6_CH0_DRAM_CFG4_REG)); */

	return 0;
}
#endif /* CONFIG_MC6P */

#ifdef CONFIG_MC6P
/* configure dfi interface to handle the handshake b/w mc6 and phy */
static void mv_ddr_mc6_dfi_config(unsigned int mc6_base)
{
	u32 val, mask, i, cl, cwl;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);

	cl = iface_params->cas_l;
	cwl = iface_params->cas_wl;

	val = TRDDATA_EN(cl) << TRDDATA_EN_OFFS |
	      TPHY_RDLAT_VAL << TPHY_RDLAT_OFFS |
	      TPHY_WRLAT(cwl) << TPHY_WRLAT_OFFS |
	      TPHY_WRDATA_VAL << TPHY_WRDATA_OFFS;
	mask = TRDDATA_EN_MASK << TRDDATA_EN_OFFS |
	       TPHY_RDLAT_MASK << TPHY_RDLAT_OFFS |
	       TPHY_WRLAT_MASK << TPHY_WRLAT_OFFS |
	       TPHY_WRDATA_MASK << TPHY_WRDATA_OFFS;
	reg_bit_clrset(mc6_base + MC6_DFI_PHY_CTRL_1_REG, val, mask);
	/* printf("MC6_DFI_PHY_CTRL_1_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_DFI_PHY_CTRL_1_REG,
		  reg_read(mc6_base + MC6_DFI_PHY_CTRL_1_REG)); */

	val = TPHY_RDCSLAT(cl) << TPHY_RDCSLAT_OFFS |
	      TPHY_WRCSLAT(cwl) << TPHY_WRCSLAT_OFFS |
	      DFI_DRAM_CLK_DIS << DFI_DRAM_CLK_DIS_OFFS;
	mask = TPHY_RDCSLAT_MASK << TPHY_RDCSLAT_OFFS |
	       TPHY_WRCSLAT_MASK << TPHY_WRCSLAT_OFFS |
	       DFI_DRAM_CLK_DIS_MASK << DFI_DRAM_CLK_DIS_OFFS;
	reg_bit_clrset(mc6_base + MC6_DFI_PHY_CTRL_0_REG, val, mask);
	/* printf("MC6_DFI_PHY_CTRL_0_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_DFI_PHY_CTRL_0_REG,
		  reg_read(mc6_base + MC6_DFI_PHY_CTRL_0_REG)); */

	/* configure channel 0 and init dfi phy per cs */
	reg_bit_clrset(mc6_base + MC6_DFI_PHY_USER_CMD_0_REG,
		       DFI_USER_CMD_0_CH0_VAL << DFI_USER_CMD_0_CH_OFFS |
		       iface_params->as_bus_params[0].cs_bitmask << DFI_USER_CMD_0_CS_OFFS |
		       DFI_PHY_INIT_DDR_DONE_REQ_VAL << DFI_PHY_INIT_DDR_DONE_REQ_OFFS |
		       DFI_PHY_INIT_REQ_VAL << DFI_PHY_INIT_REQ_OFFS,
		       DFI_USER_CMD_0_CH_MASK << DFI_USER_CMD_0_CH_OFFS |
		       DFI_USER_CMD_0_CS_MASK << DFI_USER_CMD_0_CS_OFFS |
		       DFI_PHY_INIT_DDR_DONE_REQ_MASK << DFI_USER_CMD_0_CS_OFFS |
		       DFI_PHY_INIT_REQ_MASK << DFI_PHY_INIT_REQ_OFFS);

#define MV_DDR_DFI_POLL_COUNT	5
	/* poll on dfi init bit for done for max (MV_DDR_DFI_POLL_COUNT * 10ms) */
	for (i = 0; i < MV_DDR_DFI_POLL_COUNT; i++) {
		val = reg_read(mc6_base + DFI_PHY_LEVELING_STATUS_REG);
		if (((val >> DFI_PHY_INIT_DONE_OFFS) & DFI_PHY_INIT_DONE_MASK) == DFI_PHY_INIT_DONE_VAL)
			break;
		else
			mdelay(10);
	}

	if (i >= MV_DDR_DFI_POLL_COUNT)
		printf("error: %s: dfi initialization failed\n", __func__);

	val = DFI_DRAM_CLK_EN << DFI_DRAM_CLK_DIS_OFFS;
	mask = DFI_DRAM_CLK_DIS_MASK << DFI_DRAM_CLK_DIS_OFFS;
	reg_bit_clrset(mc6_base + MC6_DFI_PHY_CTRL_0_REG, val, mask);
	/* printf("MC6_DFI_PHY_CTRL_0_REG addr 0x%x, data 0x%x\n", mc6_base + MC6_DFI_PHY_CTRL_0_REG,
		  reg_read(mc6_base + MC6_DFI_PHY_CTRL_0_REG)); */
}
#endif

/*
 * initialize mc6 memory controller
 * mc6: init per appropriate cs
 * mc6p: configure dfi and then init
 */
void mv_ddr_mc6_init(unsigned int mc6_base)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);

#ifdef CONFIG_MC6P
	/* configure dfi */
	mv_ddr_mc6_dfi_config(mc6_base);
#endif
	/* init appropriate cs */
	reg_bit_clrset(mc6_base + MC6_USER_CMD0_REG,
		       USER_CMD0_CH0_VAL << USER_CMD0_CH0_OFFS |
		       (iface_params->as_bus_params[0].cs_bitmask & USER_CMD0_CS_MASK)
		       << USER_CMD0_CS_OFFS |
		       SDRAM_INIT_REQ_VAL << SDRAM_INIT_REQ_OFFS,
		       USER_CMD0_CH0_MASK << USER_CMD0_CH0_OFFS |
		       USER_CMD0_CS_MASK << USER_CMD0_CS_OFFS |
		       SDRAM_INIT_REQ_MASK << SDRAM_INIT_REQ_OFFS);
	mdelay(10);

#ifdef CONFIG_MC6P
	/*
	 * restore the configuration changed by the workaround preventing
	 * mrs commands from being sent to memory during mc6p initialization
	 */
	reg_bit_clrset(mc6_base + MC6_CH0_MC_CTRL1_REG,
		       PHY_MASK_DIS << PHY_MASK_OFFS,
		       PHY_MASK_MASK << PHY_MASK_OFFS);
	udelay(100);

	/* write buffer drain for appropriate cs */
	reg_bit_clrset(mc6_base + MC6_USER_CMD0_REG,
		       USER_CMD0_CH0_VAL << USER_CMD0_CH0_OFFS |
		       (iface_params->as_bus_params[0].cs_bitmask & USER_CMD0_CS_MASK)
			<< USER_CMD0_CS_OFFS |
		       WCP_DRAIN_REQ_VAL << WCP_DRAIN_REQ_OFFS,
		       USER_CMD0_CH0_MASK << USER_CMD0_CH0_OFFS |
		       USER_CMD0_CS_MASK << USER_CMD0_CS_OFFS |
		       WCP_DRAIN_REQ_MASK << WCP_DRAIN_REQ_OFFS);
#endif
}

struct mv_ddr_addr_table {
	unsigned int num_of_bank_groups;
	unsigned int num_of_bank_addr_in_bank_group;
	unsigned int row_addr;
	unsigned int column_addr;
	unsigned int page_size_k_byte;
};

#ifdef CONFIG_DDR4
#define MV_DDR_DIE_CAP_MIN_IDX	MV_DDR_DIE_CAP_2GBIT
#define MV_DDR_DIE_CAP_MAX_IDX	MV_DDR_DIE_CAP_16GBIT
#else /* CONFIG_DDR3 */
#define MV_DDR_DIE_CAP_MIN_IDX	MV_DDR_DIE_CAP_512MBIT
#define MV_DDR_DIE_CAP_MAX_IDX	MV_DDR_DIE_CAP_8GBIT
#endif
#define MV_DDR_DIE_CAP_SZ	(MV_DDR_DIE_CAP_MAX_IDX - MV_DDR_DIE_CAP_MIN_IDX + 1)
#define MV_DDR_DEV_WID_MIN_IDX	MV_DDR_DEV_WIDTH_8BIT
#define MV_DDR_DEV_WID_MAX_IDX	MV_DDR_DEV_WIDTH_16BIT
#define MV_DDR_DEV_WID_SZ	(MV_DDR_DEV_WID_MAX_IDX - MV_DDR_DEV_WID_MIN_IDX + 1)

static struct mv_ddr_addr_table addr_table_db[MV_DDR_DIE_CAP_SZ][MV_DDR_DEV_WID_SZ] = {
#ifdef CONFIG_DDR4
	{ /* MV_DDR_DIE_CAP_2GBIT */
		{4, 4, 14, 10, 1}, /* MV_DDR_DEV_WIDTH_8BIT*/
		{2, 4, 14, 10, 2}, /* MV_DDR_DEV_WIDTH_16BIT*/
	},
	{ /* MV_DDR_DIE_CAP_4GBIT */
		{4, 4, 15, 10, 1},
		{2, 4, 15, 10, 2}
	},
	{ /* MV_DDR_DIE_CAP_8GBIT */
		{4, 4, 16, 10, 1},
		{2, 4, 16, 10, 2},
	},
	{ /* MV_DDR_DIE_CAP_16GBIT */
		{4, 4, 17, 10, 1},
		{2, 4, 17, 10, 2},
	},
#else /* CONFIG_DDR3 */
	{ /* MV_DDR_DIE_CAP_512MBIT */
		{1, 8, 13, 10, 1}, /* MV_DDR_DEV_WIDTH_8BIT*/
		{1, 8, 12, 10, 2}, /* MV_DDR_DEV_WIDTH_16BIT*/
	},
	{ /* MV_DDR_DIE_CAP_1GBIT */
		{1, 8, 14, 10, 1},
		{1, 8, 13, 10, 2},
	},
	{ /* MV_DDR_DIE_CAP_2GBIT */
		{1, 8, 15, 10, 1},
		{1, 8, 14, 10, 2},
	},
	{ /* MV_DDR_DIE_CAP_4GBIT */
		{1, 8, 16, 10, 1},
		{1, 8, 15, 10, 2}
	},
	{ /* MV_DDR_DIE_CAP_8GBIT */
		{1, 8, 16, 11, 2},
		{1, 8, 16, 10, 2},
	},
#endif
};

static int mv_ddr_addr_table_set(struct mv_ddr_addr_table *addr_table,
				 enum mv_ddr_die_capacity mem_size,
				 enum mv_ddr_dev_width bus_width)
{
	if (mem_size < MV_DDR_DIE_CAP_MIN_IDX ||
	    mem_size > MV_DDR_DIE_CAP_MAX_IDX) {
		printf("%s: unsupported memory size found\n", __func__);
		return -1;
	}

	if (bus_width < MV_DDR_DEV_WID_MIN_IDX ||
	    bus_width > MV_DDR_DEV_WID_MAX_IDX) {
		printf("%s: unsupported bus width found\n", __func__);
		return -1;
	}

	memcpy((void *)addr_table,
	       (void *)&addr_table_db[mem_size - MV_DDR_DIE_CAP_MIN_IDX]
				   [bus_width - MV_DDR_DEV_WID_MIN_IDX],
	       sizeof(struct mv_ddr_addr_table));

	return 0;
}

static unsigned int mv_ddr_area_length_convert(unsigned int area_length)
{
	unsigned int area_length_map = 0xffffffff;

	switch (area_length) {
	case 384:
		area_length_map = 0x0;
		break;
	case 768:
		area_length_map = 0x1;
		break;
	case 1536:
		area_length_map = 0x2;
		break;
	case 3072:
		area_length_map = 0x3;
		break;
	case 8:
		area_length_map = 0x7;
		break;
	case 16:
		area_length_map = 0x8;
		break;
	case 32:
		area_length_map = 0x9;
		break;
	case 64:
		area_length_map = 0xA;
		break;
	case 128:
		area_length_map = 0xB;
		break;
	case 256:
		area_length_map = 0xC;
		break;
	case 512:
		area_length_map = 0xD;
		break;
	case 1024:
		area_length_map = 0xE;
		break;
	case 2048:
		area_length_map = 0xF;
		break;
	case 4096:
		area_length_map = 0x10;
		break;
	case 8192:
		area_length_map = 0x11;
		break;
	case 16384:
		area_length_map = 0x12;
		break;
	case 32768:
		area_length_map = 0x13;
		break;
	default:
		/* over than 32GB is not supported */
		printf("%s: unsupported area length %d\n", __func__, area_length);
	}

	return area_length_map;
}

static unsigned int mv_ddr_bank_addr_convert(unsigned int num_of_bank_addr_in_bank_group)
{
	unsigned int num_of_bank_addr_in_bank_group_map = 0xff;

	switch (num_of_bank_addr_in_bank_group) {
	case 2:
		num_of_bank_addr_in_bank_group_map = 0x0;
		break;
	case 4:
		num_of_bank_addr_in_bank_group_map = 0x1;
		break;
	case 8:
		num_of_bank_addr_in_bank_group_map = 0x2;
		break;
	default:
		printf("%s: number of bank address in bank group %d is not supported\n", __func__,
		       num_of_bank_addr_in_bank_group);
	}

	return num_of_bank_addr_in_bank_group_map;
}

static unsigned int mv_ddr_bank_groups_convert(unsigned int num_of_bank_groups)
{
	unsigned int num_of_bank_groups_map = 0xff;

	switch (num_of_bank_groups) {
	case 1:
		num_of_bank_groups_map = 0x0;
		break;
	case 2:
		num_of_bank_groups_map = 0x1;
		break;
	case 4:
		num_of_bank_groups_map = 0x2;
		break;
	default:
		printf("%s: number of bank group %d is not supported\n", __func__,
		       num_of_bank_groups);
	}

	return num_of_bank_groups_map;
}

static unsigned int mv_ddr_column_num_convert(unsigned int column_addr)
{
	unsigned int column_addr_map = 0xff;

	switch (column_addr) {
	case 8:
		column_addr_map = 0x1;
		break;
	case 9:
		column_addr_map = 0x2;
		break;
	case 10:
		column_addr_map = 0x3;
		break;
	case 11:
		column_addr_map = 0x4;
		break;
	case 12:
		column_addr_map = 0x5;
		break;
	default:
		printf("%s: number of columns %d is not supported\n", __func__,
		       column_addr);
	}

	return column_addr_map;
}

static unsigned int mv_ddr_row_num_convert(unsigned int row_addr)
{
	unsigned int row_addr_map = 0xff;

	switch (row_addr) {
	case 11:
		row_addr_map = 0x1;
		break;
	case 12:
		row_addr_map = 0x2;
		break;
	case 13:
		row_addr_map = 0x3;
		break;
	case 14:
		row_addr_map = 0x4;
		break;
	case 15:
		row_addr_map = 0x5;
		break;
	case 16:
		row_addr_map = 0x6;
		break;
	default:
		printf("%s: number of rows %d is not supported\n", __func__,
		       row_addr);
	}

	return row_addr_map;
}

static unsigned int mv_ddr_stack_addr_num_convert(unsigned int stack_addr)
{
	unsigned int stack_addr_map = 0xff;

	switch (stack_addr) {
	case 1:
		stack_addr_map = 0x0;
		break;
	case 2:
		stack_addr_map = 0x1;
		break;
	case 4:
		stack_addr_map = 0x2;
		break;
	case 8:
		stack_addr_map = 0x3;
		break;
	default:
		printf("%s: number of stacks %d is not supported\n", __func__,
		       stack_addr);
	}

	return stack_addr_map;
}

static unsigned int mv_ddr_device_type_convert(enum mv_ddr_dev_width bus_width)
{
	unsigned int device_type_map = 0xff;

	switch (bus_width) {
	case MV_DDR_DEV_WIDTH_8BIT:
		device_type_map = 0x1;
		break;
	case MV_DDR_DEV_WIDTH_16BIT:
		device_type_map = 0x2;
		break;
	default:
		printf("%s: device type is not supported\n", __func__);
	}

	return device_type_map;
}

static unsigned int mv_ddr_bank_map_cfg_get(enum mv_ddr_bank_map bank_map)
{
	unsigned int bank_map_cfg = 0xff;

	switch (bank_map) {
	case MV_DDR_BANK_MAP_512B:
		bank_map_cfg = 0x2;
		break;
	case MV_DDR_BANK_MAP_1KB:
		bank_map_cfg = 0x3;
		break;
	case MV_DDR_BANK_MAP_2KB:
		bank_map_cfg = 0x4;
		break;
	case MV_DDR_BANK_MAP_4KB:
		bank_map_cfg = 0x5;
		break;
	case MV_DDR_BANK_MAP_8KB:
		bank_map_cfg = 0x6;
		break;
	case MV_DDR_BANK_MAP_16KB:
		bank_map_cfg = 0x7;
		break;
	case MV_DDR_BANK_MAP_32KB:
		bank_map_cfg = 0x8;
		break;
	case MV_DDR_BANK_MAP_64KB:
		bank_map_cfg = 0x9;
		break;
	case MV_DDR_BANK_MAP_128KB:
		bank_map_cfg = 0xa;
		break;
	case MV_DDR_BANK_MAP_256KB:
		bank_map_cfg = 0xb;
		break;
	case MV_DDR_BANK_MAP_512KB:
		bank_map_cfg = 0xc;
		break;
	case MV_DDR_BANK_MAP_1MB:
		bank_map_cfg = 0xd;
		break;
	case MV_DDR_BANK_MAP_2MB:
		bank_map_cfg = 0xe;
		break;
	case MV_DDR_BANK_MAP_4MB:
		bank_map_cfg = 0xf;
		break;
	case MV_DDR_BANK_MAP_8MB:
		bank_map_cfg = 0x10;
		break;
	case MV_DDR_BANK_MAP_16MB:
		bank_map_cfg = 0x11;
		break;
	case MV_DDR_BANK_MAP_32MB:
		bank_map_cfg = 0x12;
		break;
	case MV_DDR_BANK_MAP_64MB:
		bank_map_cfg = 0x13;
		break;
	case MV_DDR_BANK_MAP_128MB:
		bank_map_cfg = 0x14;
		break;
	case MV_DDR_BANK_MAP_256MB:
		bank_map_cfg = 0x15;
		break;
	case MV_DDR_BANK_MAP_512MB:
		bank_map_cfg = 0x16;
		break;
	case MV_DDR_BANK_MAP_1GB:
		bank_map_cfg = 0x17;
		break;
	case MV_DDR_BANK_MAP_2GB:
		bank_map_cfg = 0x18;
		break;
	case MV_DDR_BANK_MAP_4GB:
		bank_map_cfg = 0x19;
		break;
	case MV_DDR_BANK_MAP_8GB:
		bank_map_cfg = 0x1a;
		break;
	case MV_DDR_BANK_MAP_16GB:
		bank_map_cfg = 0x1b;
		break;
	case MV_DDR_BANK_MAP_32GB:
		bank_map_cfg = 0x1c;
		break;
	case MV_DDR_BANK_MAP_64GB:
		bank_map_cfg = 0x1d;
		break;
	default:
		printf("%s: unsupported bank address switch boundary found\n", __func__);
	}

	return bank_map_cfg;
}

void mv_ddr_mc6_sizes_cfg(unsigned int mc6_base, unsigned long iface_base_addr)
{
	unsigned int cs_idx;
	unsigned int cs_num;
#if !defined(CONFIG_A3700) && !defined(CONFIG_MC6P)
	unsigned int reserved_mem_idx;
#endif
	unsigned long long area_length_bytes;
	unsigned int are_length_mega_bytes;
	unsigned long long start_addr_bytes;
	unsigned int start_addr_low, start_addr_high;
	enum mv_ddr_bank_map bm = MV_DDR_BANK_MAP_LAST;

	struct mv_ddr_addr_table addr_tbl = {0};
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);
	cs_num = mv_ddr_cs_num_get();

	area_length_bytes = mv_ddr_mem_sz_per_cs_get();
	are_length_mega_bytes = area_length_bytes / MV_DDR_MEGABYTE;

	mv_ddr_addr_table_set(&addr_tbl,
			      iface_params->memory_size,
			      iface_params->bus_width);

	/*
	 * calculate bank address assignment boundary
	 * if ddr3 page size is N, ddr4 page size is 2 * N
	 * if x16 mode page size is M, x32 mode page size is 2 * M
	 */
#ifdef CONFIG_DDR4
	bm = MV_DDR_BANK_MAP_4KB;
	if (tm->bus_act_mask == BUS_MASK_32BIT)
		bm = MV_DDR_BANK_MAP_8KB;
#else /* CONFIG_DDR3 */
	bm = MV_DDR_BANK_MAP_2KB;
	if (tm->bus_act_mask == BUS_MASK_32BIT)
		bm = MV_DDR_BANK_MAP_4KB;
#endif

	/* configure all length per cs here and activate the cs */
	for (cs_idx = 0; cs_idx < cs_num; cs_idx++) {
		start_addr_bytes = iface_base_addr + area_length_bytes * cs_idx;
		start_addr_low = start_addr_bytes & MV_DDR_32_BITS_MASK;
		start_addr_high = (start_addr_bytes >> START_ADDR_HTOL_OFFS) & MV_DDR_32_BITS_MASK;

		reg_bit_clrset(mc6_base + MC6_CH0_MMAP_LOW_REG(cs_idx),
			       CS_VALID_ENA << CS_VALID_OFFS |
			       INTERLEAVE_DIS << INTERLEAVE_OFFS |
			       mv_ddr_area_length_convert(are_length_mega_bytes) << AREA_LENGTH_OFFS |
			       start_addr_low,
			       CS_VALID_MASK << CS_VALID_OFFS |
			       INTERLEAVE_MASK << INTERLEAVE_OFFS |
			       AREA_LENGTH_MASK << AREA_LENGTH_OFFS |
			       START_ADDRESS_L_MASK << START_ADDRESS_L_OFFS);
		/* printf("MC6_CH0_MMAP_LOW_REG(cs_idx) addr 0x%x, data 0x%x\n",
		** mc6_base + MC6_CH0_MMAP_LOW_REG(cs_idx),
		** reg_read(mc6_base + MC6_CH0_MMAP_LOW_REG(cs_idx))); */

		reg_bit_clrset(mc6_base + MC6_CH0_MMAP_HIGH_REG(cs_idx),
			       start_addr_high << START_ADDRESS_H_OFFS,
			       START_ADDRESS_H_MASK << START_ADDRESS_H_OFFS);
		/* printf("MC6_CH0_MMAP_HIGH_REG(cs_idx) addr 0x%x, data 0x%x\n",
		** mc6_base + MC6_CH0_MMAP_HIGH_REG(cs_idx),
		** reg_read(mc6_base + MC6_CH0_MMAP_HIGH_REG(cs_idx))); */

		reg_bit_clrset(mc6_base + MC6_CH0_MC_CFG_REG(cs_idx),
			       mv_ddr_bank_addr_convert(addr_tbl.num_of_bank_addr_in_bank_group) <<
			       BA_NUM_OFFS |
			       mv_ddr_bank_groups_convert(addr_tbl.num_of_bank_groups) <<
			       BG_NUM_OFFS |
			       mv_ddr_column_num_convert(addr_tbl.column_addr) <<
			       CA_NUM_OFFS |
			       mv_ddr_row_num_convert(addr_tbl.row_addr) <<
			       RA_NUM_OFFS |
			       mv_ddr_stack_addr_num_convert(SINGLE_STACK) <<
			       SA_NUM_OFFS |
			       mv_ddr_device_type_convert(iface_params->bus_width) <<
			       DEVICE_TYPE_OFFS |
			       mv_ddr_bank_map_cfg_get(bm) <<
			       MV_DDR_BANK_MAP_OFFS,
			       BA_NUM_MASK << BA_NUM_OFFS |
			       BG_NUM_MASK << BG_NUM_OFFS |
			       CA_NUM_MASK << CA_NUM_OFFS |
			       RA_NUM_MASK << RA_NUM_OFFS |
			       SA_NUM_MASK << SA_NUM_OFFS |
			       DEVICE_TYPE_MASK << DEVICE_TYPE_OFFS |
			       MV_DDR_BANK_MAP_MASK << MV_DDR_BANK_MAP_OFFS);
	}

#if !defined(CONFIG_A3700) && !defined(CONFIG_MC6P)
	/* configure here the channel 1 reg_map_low and reg_map_high to unused memory area due to mc6 bug */
	for (cs_idx = 0, reserved_mem_idx = cs_num; cs_idx < cs_num; cs_idx++, reserved_mem_idx++) {
		start_addr_bytes = area_length_bytes * reserved_mem_idx;
		start_addr_low = start_addr_bytes & MV_DDR_32_BITS_MASK;
		start_addr_high = (start_addr_bytes >> START_ADDR_HTOL_OFFS) & MV_DDR_32_BITS_MASK;

		reg_bit_clrset(mc6_base + MC6_CH1_MMAP_LOW_REG(cs_idx),
			       CS_VALID_ENA << CS_VALID_OFFS |
			       INTERLEAVE_DIS << INTERLEAVE_OFFS |
			       mv_ddr_area_length_convert(are_length_mega_bytes) << AREA_LENGTH_OFFS |
			       start_addr_low,
			       CS_VALID_MASK << CS_VALID_OFFS |
			       INTERLEAVE_MASK << INTERLEAVE_OFFS |
			       AREA_LENGTH_MASK << AREA_LENGTH_OFFS |
			       START_ADDRESS_L_MASK << START_ADDRESS_L_OFFS);
		/* printf("MC6_CH1_MMAP_LOW_REG(cs_idx) addr 0x%x, data 0x%x\n",
		** mc6_base + MC6_CH1_MMAP_LOW_REG(cs_idx),
		** reg_read(mc6_base + MC6_CH1_MMAP_LOW_REG(cs_idx))); */

		reg_bit_clrset(mc6_base + MC6_CH1_MMAP_HIGH_REG(cs_idx),
			       start_addr_high << START_ADDRESS_H_OFFS,
			       START_ADDRESS_H_MASK << START_ADDRESS_H_OFFS);
		/* printf("MC6_CH1_MMAP_HIGH_REG(cs_idx) addr 0x%x, data 0x%x\n",
		** mc6_base + MC6_CH1_MMAP_HIGH_REG(cs_idx),
		** reg_read(mc6_base + MC6_CH1_MMAP_HIGH_REG(cs_idx))); */
	}
#endif
}

static void mv_ddr_mc6_ecc_enable(unsigned int mc6_base)
{
	reg_bit_clrset(mc6_base + MC6_RAS_CTRL_REG,
		       ECC_EN_ENA << ECC_EN_OFFS,
		       ECC_EN_MASK << ECC_EN_OFFS);
}

/* FIXME: revise hard-coded values and remove platform dependent flags */
int mv_ddr_mc6_config(unsigned int mc6_base, unsigned long iface_base_addr, int ecc_is_ena)
{
	unsigned int cs_num;
	unsigned int odt_cfg_wr, odt_cfg_rd;

	mv_ddr_mc6_and_dram_timing_set(mc6_base);

	mv_ddr_mc6_sizes_cfg(mc6_base, iface_base_addr);

	if (ecc_is_ena)
		mv_ddr_mc6_ecc_enable(mc6_base);

#ifdef CONFIG_MC6P
	/*
	 * TODO: check the static configuration below
	 * insert all into the driver
	 * remove all mc6 functions into mc6 driver
	 * call only init function to be the interface
	 * for mc6 driver and it will expand the other
	 * functions above
	 */
	if (mv_ddr_mc6_cfg_set(mc6_base))
		return -1;

	/* TODO: move the following three writes to electrical parameters settings */
	/* ODT_Control_1: ODT_write_en, ODT0_read_en */
	reg_write(mc6_base + MC6_CH0_ODT_CTRL1_REG, MC6_CH0_ODT_CTRL1_VAL);

	/* DRAM_Config_5 CS0: RTT_Park, RTT_WR */
	reg_write(mc6_base + MC6_CH0_DRAM_CFG5_REG(0/* cs 0 */), MC6_CH0_DRAM_CFG5_VAL);
	/* DRAM_Config_5 CS1: RTT_Park, RTT_WR  */
	reg_write(mc6_base + MC6_CH0_DRAM_CFG5_REG(1/* cs 1 */), MC6_CH0_DRAM_CFG5_VAL);
#else
	/* TODO: remove hard-coded values and platform related flags */
	reg_write(0x20064, 0x606);	/* MC "readReady"+ MC2PHYlatency */
	reg_write(0x21180, 0x500);	/* PHY_RL_Control for CS0:phy_rl_cycle_dly and phy_rl_tap_dly*/
	reg_write(0x21000, 0x60);	/* phy_rfifo_rptr_dly_val */
	reg_write(0x210c0, 0x81000001);	/* PHY_WL_RL_Control: bit 31 phy_rdq_sel_u_en for pod '1' phy_rl_enable = '1' */
	reg_write(0x202c8, 0xfefe);	/* MC_Control_3: phy_in_ff_bypass, phy_out_ff_bypass */

	reg_write(0x20310, 0x21000000);	/* DRAM_Config_5 CS0: RTT_Park, RTT_WR */
	reg_write(0x20318, 0x0);	/* DRAM_Config_5 CS2: RTT_Park, RTT_WR */
	reg_write(0x2031c, 0x0);	/* DRAM_Config_5 CS3: RTT_Park, RTT_WR */
	reg_write(0x20304, 0x400);	/* DRAM_Config_2 */
	reg_write(0x20308, 0x1);	/* DRAM_Config_3 DLL_reset */
#if defined(A80X0)
	reg_write(0x20314, 0x0);	/* DRAM_Config_5 CS1: RTT_Park, RTT_WR - Diff1 */
#if defined(CONFIG_64BIT)
	reg_write(0x20044, 0x30400);	/* MC_Control_0 - bust length, data width need to configure - diff4 - config */
#else
	reg_write(0x20044, 0x30300);	/* MC_Control_0 - bust length, data width need to configure - diff4 - config */
#endif	/* (CONFIG_64BIT) */
#endif	/* #if defined(A80X0) */
#if defined(A70X0)
	reg_write(0x20314, 0x21010000);
	reg_write(0x20044, 0x30300);
#endif	/* #if defined(A70X0) */
	reg_write(0x202c0, 0x6000);	/* MC_Control_1 - tw2r_dis? , acs_exit_dly timing???, config?? */
	reg_write(0x202c4, 0x120030);	/* MC_Control_2 - sdram typ, mode 2t, mirror en, rdimm mode - config */
	reg_write(0x20180, 0x30200);	/* RPP_Starvation_Control - default */
	reg_write(0x20050, 0xff);	/* Spool_Control default */
	reg_write(0x20054, 0x4c0);	/* MC_pwr_ctl - default */
	reg_write(0x2030c, 0x90000);	/* DRAM_Config_4: vref training value, odt? - config */
#endif	/* #ifdef CONFIG_MC6P */

	/* ODT_Control_2: force_odt */
	odt_cfg_wr = mv_ddr_mc6_odt_cfg_wr_get();
	odt_cfg_rd = mv_ddr_mc6_odt_cfg_rd_get();
	cs_num = mv_ddr_cs_num_get();
	if (cs_num == 1)
		reg_write(mc6_base + MC6_CH0_ODT_CTRL2_REG,
			  odt_cfg_wr << MC6_ODT_WRITE_CS0_OFFS |
			  odt_cfg_rd << MC6_ODT_READ_OFFS);
	else /* dual cs */
		reg_write(mc6_base + MC6_CH0_ODT_CTRL2_REG,
			  odt_cfg_wr << MC6_ODT_WRITE_CS0_OFFS |
			  odt_cfg_wr << MC6_ODT_WRITE_CS1_OFFS |
			  odt_cfg_rd << MC6_ODT_READ_OFFS);

	return 0;
}

uint64_t mv_ddr_mc6_mem_offs_get(u32 cs, unsigned int mc6_base)
{
	uint64_t offset;
	u32 start_address_l;
	u32 start_address_h;

	start_address_l = reg_read((mc6_base + MC6_CH0_MMAP_LOW_REG(cs)));
	start_address_h = reg_read((mc6_base + MC6_CH0_MMAP_HIGH_REG(cs)));

	offset = (start_address_h * _4G) +
		 (start_address_l & (START_ADDRESS_L_MASK << START_ADDRESS_L_OFFS));

	return offset;
}

uint64_t mv_ddr_mc6_mem_size_get(unsigned int mc6_base)
{
	u32 reg_val = reg_read((mc6_base + MC6_CH0_MMAP_LOW_BASE));

	reg_val = (reg_val >> AREA_LENGTH_OFFS) & AREA_LENGTH_MASK;
	switch (reg_val) {
	case 0xe:
		return _1G;
	case 0xf:
		return _2G;
	case 0x10:
		return _4G;
	case 0x11:
		return _8G;
	case 0x12:
		return _16G;
	case 0x13:
		return _32G;
	case 0x14:
		return _64G;
	case 0x15:
		return _128G;
	default:
		return 0;
	}
}

unsigned int mv_ddr_mc6_odt_cfg_pat_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int odt_cfg_pat = tm->edata.mac_edata.odt_cfg_pat;

	if (odt_cfg_pat >= MV_DDR_ODT_CFG_LAST) {
		printf("error: %s: unsupported odt_cfg_pat parameter found\n", __func__);
		odt_cfg_pat = PARAM_UNDEFINED;
	}

	return odt_cfg_pat;
}

unsigned int mv_ddr_mc6_odt_cfg_wr_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int odt_cfg_wr = tm->edata.mac_edata.odt_cfg_wr;

	if (odt_cfg_wr >= MV_DDR_ODT_CFG_LAST) {
		printf("error: %s: unsupported odt_cfg_wr parameter found\n", __func__);
		odt_cfg_wr = PARAM_UNDEFINED;
	}

	return odt_cfg_wr;
}

unsigned int mv_ddr_mc6_odt_cfg_rd_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int odt_cfg_rd = tm->edata.mac_edata.odt_cfg_rd;

	if (odt_cfg_rd >= MV_DDR_ODT_CFG_LAST) {
		printf("error: %s: unsupported odt_cfg_rd parameter found\n", __func__);
		odt_cfg_rd = PARAM_UNDEFINED;
	}

	return odt_cfg_rd;
}
