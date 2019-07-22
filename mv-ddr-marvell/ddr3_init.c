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

#include "ddr3_init.h"
#include "mv_ddr_common.h"

#if defined(CONFIG_DDR4)
static char *ddr_type = "DDR4";
#else /* CONFIG_DDR4 */
static char *ddr_type = "DDR3";
#endif /* CONFIG_DDR4 */

/*
 * generic_init_controller controls D-unit configuration:
 * '1' - dynamic D-unit configuration,
 */
u8 generic_init_controller = 1;

static int mv_ddr_training_params_set(u8 dev_num);

/*
 * Name:     ddr3_init - Main DDR3 Init function
 * Desc:     This routine initialize the DDR3 MC and runs HW training.
 * Args:     None.
 * Notes:
 * Returns:  None.
 */
int ddr3_init(void)
{
	int status;
	int is_manual_cal_done;

	/* Print mv_ddr version */
	mv_ddr_ver_print();

	mv_ddr_pre_training_fixup();

	/* SoC/Board special initializations */
	mv_ddr_pre_training_soc_config(ddr_type);

	/* Set log level for training library */
	mv_ddr_user_log_level_set(DEBUG_BLOCK_ALL);

	mv_ddr_early_init();

	if (mv_ddr_topology_map_update()) {
		printf("mv_ddr: failed to update topology\n");
		return MV_FAIL;
	}

	if (mv_ddr_early_init2() != MV_OK)
		return MV_FAIL;

	/* Set training algorithm's parameters */
	status = mv_ddr_training_params_set(0);
	if (MV_OK != status)
		return status;

#if defined(CONFIG_MC_STATIC)
	mv_ddr_mc_static_config();
#else
	mv_ddr_mc_config();
#endif

	is_manual_cal_done = mv_ddr_manual_cal_do();

	mv_ddr_mc_init();

	if (!is_manual_cal_done) {
#if !defined(CONFIG_MC_STATIC)
#if defined(CONFIG_DDR4)
		status = mv_ddr4_calibration_adjust(0, 1, 0);
		if (status != MV_OK) {
			printf("%s: failed (0x%x)\n", __func__, status);
			return status;
		}
#endif
#endif
	}

#if defined(CONFIG_MC_STATIC_PRINT)
	mv_ddr_mc_static_print();
#endif

	status = ddr3_silicon_post_init();
	if (MV_OK != status) {
		printf("DDR3 Post Init - FAILED 0x%x\n", status);
		return status;
	}

#if defined(CONFIG_PHY_STATIC)
	mv_ddr_phy_static_config();
#else
	/* PHY initialization (Training) */
	status = hws_ddr3_tip_run_alg(0, ALGO_TYPE_DYNAMIC);
	if (MV_OK != status) {
		printf("%s Training Sequence - FAILED\n", ddr_type);
		return status;
	}
#endif

#if defined(CONFIG_PHY_STATIC_PRINT)
	mv_ddr_phy_static_print();
#endif

	/* Post MC/PHY initializations */
	mv_ddr_post_training_soc_config(ddr_type);

	mv_ddr_post_training_fixup();

	if (mv_ddr_is_ecc_ena())
		mv_ddr_mem_scrubbing();

	printf("mv_ddr: completed successfully\n");

	return MV_OK;
}

/*
 * Name:	mv_ddr_training_params_set
 * Desc:
 * Args:
 * Notes:	sets internal training params
 * Returns:
 */
static int mv_ddr_training_params_set(u8 dev_num)
{
	struct tune_train_params params;
	int status;
	u32 cs_num;

	cs_num = mv_ddr_cs_num_get();

	/* NOTE: do not remove any field initilization */
	params.ck_delay = TUNE_TRAINING_PARAMS_CK_DELAY;
	params.phy_reg3_val = TUNE_TRAINING_PARAMS_PHYREG3VAL;
	params.g_zpri_data = TUNE_TRAINING_PARAMS_PRI_DATA;
	params.g_znri_data = TUNE_TRAINING_PARAMS_NRI_DATA;
	params.g_zpri_ctrl = TUNE_TRAINING_PARAMS_PRI_CTRL;
	params.g_znri_ctrl = TUNE_TRAINING_PARAMS_NRI_CTRL;
	params.g_znodt_data = TUNE_TRAINING_PARAMS_N_ODT_DATA;
	params.g_zpodt_ctrl = TUNE_TRAINING_PARAMS_P_ODT_CTRL;
	params.g_znodt_ctrl = TUNE_TRAINING_PARAMS_N_ODT_CTRL;

#if defined(CONFIG_DDR4)
	params.g_zpodt_data = TUNE_TRAINING_PARAMS_P_ODT_DATA_DDR4;
	params.g_odt_config = TUNE_TRAINING_PARAMS_ODT_CONFIG_DDR4;
	params.g_rtt_nom = TUNE_TRAINING_PARAMS_RTT_NOM_DDR4;
	params.g_dic = TUNE_TRAINING_PARAMS_DIC_DDR4;
	if (cs_num == 1) {
		params.g_rtt_wr =  TUNE_TRAINING_PARAMS_RTT_WR_1CS;
		params.g_rtt_park = TUNE_TRAINING_PARAMS_RTT_PARK_1CS;
	} else {
		params.g_rtt_wr =  TUNE_TRAINING_PARAMS_RTT_WR_2CS;
		params.g_rtt_park = TUNE_TRAINING_PARAMS_RTT_PARK_2CS;
	}
#else /* CONFIG_DDR4 */
	params.g_zpodt_data = TUNE_TRAINING_PARAMS_P_ODT_DATA;
	params.g_dic = TUNE_TRAINING_PARAMS_DIC;
	params.g_rtt_nom = TUNE_TRAINING_PARAMS_RTT_NOM;
	if (cs_num == 1) {
		params.g_rtt_wr = TUNE_TRAINING_PARAMS_RTT_WR_1CS;
		params.g_odt_config = TUNE_TRAINING_PARAMS_ODT_CONFIG_1CS;
	} else {
		params.g_rtt_wr = TUNE_TRAINING_PARAMS_RTT_WR_2CS;
		params.g_odt_config = TUNE_TRAINING_PARAMS_ODT_CONFIG_2CS;
	}
#endif /* CONFIG_DDR4 */

	status = ddr3_tip_tune_training_params(dev_num, &params);
	if (MV_OK != status) {
		printf("%s Training Sequence - FAILED\n", ddr_type);
		return status;
	}

	return MV_OK;
}
