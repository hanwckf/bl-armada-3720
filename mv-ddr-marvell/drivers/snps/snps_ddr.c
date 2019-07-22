/*******************************************************************************
Copyright (C) 2017 Marvell International Ltd.

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

#if (__GNUC__ == 5  && __GNUC_MINOR__ == 3 && __GNUC_PATCHLEVEL__ == 1)
#include "snps_static-gcc-5.h"
#elif (__GNUC__ == 7  && __GNUC_MINOR__ == 3 && __GNUC_PATCHLEVEL__ == 1)
#include "snps_static-gcc-7.h"
#else
#error "Unsupported GCC version! Please rebuild your SNPS FW image."
#endif
#include "snps.h"
#include "snps_update.h"
#include "snps_regs.h"

/* TODO: consider where to relocate the below structs: snps_section_names & snps_sections */
static struct snps_section_name snps_section_names[] = {
	{ SECTION_PHY_CONFIG , "PHY Configuration" },
	{ SECTION_1D_IMEM , "1D IMEM" },
	{ SECTION_1D_DMEM , "1D DMEM" },
	{ SECTION_2D_IMEM , "2D IMEM" },
	{ SECTION_2D_DMEM , "2D DMEM" },
	{ SECTION_PHY_INIT_ENGINE , "PHY Init Engine (PIE)" }
};

static struct snps_section_content snps_sections[] = {
	/* section ID		, load_static , load_static_update , load_dynamic_update */
	{.section_id = SECTION_PHY_CONFIG,
	 .load_type = LOAD_RANDOM,
	 .load_static.random = init_phy_static,
	 .load_static_update = init_phy_static_update,
	 .load_dynamic_update = init_phy_dynamic_update
	},
	{.section_id = SECTION_1D_IMEM,
	 .load_type = LOAD_SEQUENTIAL,
	 .load_static.sequential = &one_d_imem_static,
	 .load_static_update = NULL,
	 .load_dynamic_update = NULL
	},
	{.section_id = SECTION_1D_DMEM,
	 .load_type = LOAD_SEQUENTIAL,
	 .load_static.sequential = &one_d_dmem_static,
	 .load_static_update = one_d_dmem_static_update,
	 .load_dynamic_update = one_d_dmem_dynamic_update
	},
	{.section_id = SECTION_2D_IMEM,
	 .load_type = LOAD_SEQUENTIAL,
	 .load_static.sequential = &two_d_imem_static,
	 .load_static_update = NULL,
	 .load_dynamic_update = NULL
	},
	{.section_id = SECTION_2D_DMEM,
	 .load_type = LOAD_SEQUENTIAL,
	 .load_static.sequential = &two_d_dmem_static,
	 .load_static_update = two_d_dmem_static_update,
	 .load_dynamic_update = two_d_dmem_dynamic_update
	},
	{.section_id = SECTION_PHY_INIT_ENGINE,
	 .load_type = LOAD_RANDOM,
	 .load_static.random = pie_static,
	 .load_static_update = NULL,
	 .load_dynamic_update = pie_dynamic_update
	},
};

/* ======================================================================= */
/*				Mail Box Code				*/
/* ======================================================================= */
/* This routine will poll on mail box for completion messages:
 * 1. Initialize mail box log data structure
 * 2. Read mail box messages, while polling specific success/failure messages
 * 3. All messages will be logged, so they could be printed when training step
 *    is completed (according to each message log level settings)
 * 4. Print all received messages, in case they are set with required log level
 * returns MB_MAJOR_ID_TRAINING_SUCCESS OR MB_MAJOR_ID_TRAINING_FAILED;
 */
static int snps_mail_box_poll_completion(void)
{
	int ret = 0;
	debug_enter();

	/* Initialize & reset mail box log */
	snps_mail_box_log_init();

	/* Read mail box messages, and poll for completion messages (success/failure) */
	do {
		ret = snps_mail_box_read();
	} while (ret != MB_MAJOR_ID_TRAINING_SUCCESS &&	ret != MB_MAJOR_ID_TRAINING_FAILED
			&& ret != MAIL_BOX_ERROR);

	/* Once step completed, dump mail box messages according to log level */
	snps_mail_box_log_dump();

	debug_exit();
	return ret;
}

/* This routine evaluates results of the performed training step/s.
 * It will read message block results (and possibly mail box messages),
 * and decide (return) one of the following:
 * SNPS_TRAINING_SEQUENCE_PASS : Training step passed.
 * SNPS_TRAINING_SEQUENCE_RETRY: Training step failed, but this can be recovered.
 *				 This means that a certain training step (or steps) should be retried.
 * SNPS_TRAINING_SEQUENCE_FAIL : Training step failed - unrecoverable.
 */
static int snps_evaluate_training_results(int sequence_ctrl)
{
	int ret;
	debug_enter();

	ret = snps_get_result(MSG_BLK_CS_TEST_FAIL, SNPS_LAST_RUN, SEQEUNCE_CTRL_ALL);
	if (ret == TRAINING_SEQUENCE_PASS) {
		pr_debug("Training step/s (%x) passed (CS_TEST_FAIL = 0)\n", sequence_ctrl);
		goto finish;
	}

	/* Else CS_TEST_FAIL equals non-zero, means that one or more ranks failed training */
	pr_debug("Error: %d ranks failed training\n" , ret);
	ret = TRAINING_SEQUENCE_FAIL;

	/* TODO: implement decision logic below, (SNPS state machine):
	 * read message block (and mail box if needed), and decide whether training is:
	 * 1. Recoverable, so we should retry training step/s, with new settings
	 * 2. Failed, since failure can't be recovered.
	 */

	/* return SNPS_TRAINING_SEQUENCE_RETRY; */
	/* return SNPS_TRAINING_SEQUENCE_FAIL; */


	/* some examples on how to use snps_get_results with various argument's combination
	result =  snps_get_result(SNPS_MSG_BLK_CDD_RW_1_0, SNPS_LAST_RUN, -1);
	result = snps_get_result(SNPS_MSG_BLK_CDD_RW_1_0, 1, SNPS_SEQUENCE_CTRL_RX_EN);
	result = snps_get_result(SNPS_MSG_BLK_CDD_RW_1_0, 2, SNPS_SEQUENCE_CTRL_RX_EN);
	result = snps_get_result(SNPS_MSG_BLK_CDD_RW_1_0, 2, -1);
	result = snps_get_result(SNPS_MSG_BLK_CDD_RW_1_0, 3, SNPS_SEQUENCE_CTRL_RX_EN);
	result = snps_get_result(SNPS_MSG_BLK_CDD_RW_1_0 + 10, 2, SNPS_SEQUENCE_CTRL_RX_EN);
	result = snps_get_result(SNPS_MSG_BLK_CDD_RW_1_0, 1, SNPS_SEQUENCE_CTRL_RFU14);
	*/

	/* example for checking specific result for 1D training, from step 'read gate training':
	 * if (snps_get_state() == TRAINING_1D) {
	 *	if (snps_sequence_ctrl_get() & SEQUENCE_CTRL_RX_EN) {
	 *		result = snps_get_result(SNPS_MSG_BLK_CDD_RW_1_0, 1, SNPS_SEQUENCE_CTRL_RX_EN);
	 *		if (result > some_threshold)
	 *			ret = SNPS_TRAINING_SEQUENCE_FAIL
	 *	}
	 * }
	 */


finish:
	debug_exit();
	return ret;
}

/* This routine will load the initial static section of requested section_id,
 * and if requested also static_update stage (with 'static_update' argument),
 * than it will also load the static update section for that section_id.
 *
 * Generally, each section may have 3 stages:
 * 1. Static settings - as received from Synopsys release
 * 2. Static update: values that needs to be updated with static values
 *			(static values means the value doesn't change along the training flow)
 * 3. Dynamic update: values that can be derived only dynamically at runtime
 *			(i.e topology dependant variables)
 *			- dynamic update section is handled in snps_load_dynamic() routine
 *
 * Arguments:
 *	- enum snps_section_id section_id:
 *			requested static section to be loaded
 *	- enum snps_static_update static_update:
 *			will be set to 'STATIC_UPDATE_YES' in case that section has also
 *			a static update part
 *			Else if no static update section, than it will be 'STATIC_UPDATE_NO'
 */
static int snps_load_static(enum snps_section_id section_id, enum snps_static_update static_update)
{
	int stage_count, ret = -1;
	int stage, idx;
	debug_enter();

	if (snps_sections[section_id].section_id != section_id) {
		printf("Error: %s: invalid section entry for section id %d:'%s'\n", __func__,
			section_id, snps_section_names[section_id].section_name);
		goto fail;
	}

	/* prepare static update section as 2nd stage, in case needed */
	if (static_update == STATIC_UPDATE_YES)
		stage_count = 2;
	else
		stage_count = 1;

	/* Enable CSRs access (disable firmware access) */
	snps_csr_access_set(MICRO_CONT_MUX_SEL_ENABLE);

	for (stage = 0; stage < stage_count; stage++) {
		pr_debug("Load static %s section for %s\n" , (stage == 1 ? "update " : ""),
				snps_section_names[section_id].section_name);

		if ((stage == 0) &&
		    (snps_sections[section_id].load_type == LOAD_SEQUENTIAL)) {
			/* Sequentional load - only possible at stage 0 (init) */
			const struct snps_seq_data *sequent_ptr =
						 snps_sections[section_id].load_static.sequential;
			int address;

			/* Verify section */
			if (sequent_ptr == NULL)
				goto static_isolate_csr;

			/* Firstly write all predefined entries */
			for (idx = 0, address = sequent_ptr->start_addr;
			     idx < sequent_ptr->data_count; address++, idx++)
				SNPS_STATIC_WRITE(address, sequent_ptr->data[idx]);

			/* Pad the rest of memory with zeros */
			for (; address <= sequent_ptr->end_addr; address++)
				SNPS_STATIC_WRITE(address, 0x0);

		} else {
			/* Random sequence load */
			const struct snps_address_data *section_ptr;

			if (stage == 0)
				section_ptr = snps_sections[section_id].load_static.random;
			else
				section_ptr = snps_sections[section_id].load_static_update;
			/* Verify section */
			if (section_ptr == NULL)
				goto static_isolate_csr;

			/* Go over section and write it's address-data pairs */
			for (idx = 0; section_ptr[idx].addr != -1; idx++) {
				if (stage == 0) { /* 1st static init shouldn't be printed - too long */
					SNPS_STATIC_WRITE(section_ptr[idx].addr, section_ptr[idx].data);
				} else {
					snps_fw_write(section_ptr[idx].addr, section_ptr[idx].data);
				}
			}
		} /* End of Random sequence load */

		pr_debug("\t Static %s for section:'%s': %d writes\n", (stage == 0 ? "Init" : "Update"),
				snps_section_names[section_id].section_name, idx);
	}
	ret = 0;

static_isolate_csr:
	if (ret < 0)
		printf("invalid static %s content for section '%s'\n" ,
			   (stage == 1 ? "update" : ""), snps_section_names[section_id].section_name);
	/* Isolate CSRs - allow firmware unrestricted access */
	snps_csr_access_set(MICRO_CONT_MUX_SEL_ISOLATE);

fail:
	debug_exit();
	return ret;
}

/* snps_load_ext_swizzle_cfg:
 * This routine will override the default static swizzle configuration
 * with the platform's one.
 */
static int snps_load_ext_swizzle_cfg(void)
{
	u32 *swizzle_cfg = NULL;
	u32 i;

	debug_enter();

	pr_debug("%s Load external swizzle configuration\n", __func__);

	swizzle_cfg = snps_ext_swizzle_cfg_get();

	/* complete if nothing to do */
	if (!swizzle_cfg)
		goto finish;

	snps_csr_access_set(MICRO_CONT_MUX_SEL_ENABLE);
	/* configure the swizzle registers */
	for (i = 0; i < HWT_SWIZZLE_HWT_REGS_NUM; i++)
		snps_fw_write(
			PHY_REG_ADDR_MAP(P_STATE_0,
					 BLK_TYPE_MASTER,
					 INST_NUM_0,
					 (REG_120_HWT_SWIZZLE_HWT_ADDR0_BASE + i)),
			*(swizzle_cfg + i));
	snps_csr_access_set(MICRO_CONT_MUX_SEL_ISOLATE);

finish:
	debug_exit();
	return 0;
}

/* snps_load_dynamic:
 * This routine will overide the pre-loaded static sections, with topology
 * settings updates, and run-time updates (depends on the training step).
 * Each updated field has it's own function, to return the field's value for
 * any step of the training process.
 * The function pointers for getting updated field's values are defined
 * at the struct pointed by load_dynamic_update entry in the snps_sections array.
 */
static int snps_load_dynamic(enum snps_section_id section_id)
{
	int ret = 0, i = 0;
	const struct snps_address_dynamic_update *section_ptr;

	debug_enter();

	pr_debug("%s Dynamic update for section:'%s'\n", __func__, snps_section_names[section_id].section_name);

	/* Enable CSRs access (disable firmware access) */
	snps_csr_access_set(MICRO_CONT_MUX_SEL_ENABLE);

	section_ptr = snps_sections[section_id].load_dynamic_update;
	if (!section_ptr) {
		printf("invalid dynamic content for section '%s'\n" ,
			snps_section_names[section_id].section_name);
			ret = -1;
			goto isolate_csr;
	}
	/* Go over section and write it's address-data pairs */
	while (section_ptr[i].addr != -1) {
		/*snps_fw_write(section_ptr[i].addr, section_ptr[i].get_update_value());*/
		snps_fw_write(section_ptr[i].addr, section_ptr[i].get_update_value());
		i++;
	}

isolate_csr:
	/* Isolate CSRs - allow firmware unrestricted access */
	snps_csr_access_set(MICRO_CONT_MUX_SEL_ISOLATE);

	debug_exit();
	return ret;
}

/*
 * This routine will prepare all static settings of section:
 * 1. Load static IMEM & DMEM settings, including static topology updates for DMEM
 * - enum snps_section_id imem_section: SNPS_1D_IMEM / SNPS_2D_IMEM
 */
static int snps_ddr_section_init(enum snps_section_id imem_section)
{
	int ret = 0, dmem_section = imem_section + 1;
	debug_enter();

	/* --- Step 1(Cont.) :load static 1D/2D IMEM section, with no static update --- */
	ret = snps_load_static(imem_section, STATIC_UPDATE_NO);
	if (ret != 0)
		goto fail;

	/* --- Step 2: load static 1D/2D DMEM section, including static topology update --- */
	ret = snps_load_static(dmem_section, STATIC_UPDATE_YES);
	if (ret != 0)
		goto fail;

fail:
	if (ret)
		printf("SNPS DDR: %s initialization failed!\n",
			imem_section == SECTION_1D_IMEM ? "1D" : "2D");
	debug_exit();
	return ret;
}

/*
 * This routine will perform the actual training:
 * 1. Preparing relevant sequence_ctrl training steps
 * 2. Perform dynamic topology update per each training step/s
 * 3. Trigger training execution
 * 4. Poll mail box for completion (and log mails)
 * 5. Read message block training output results
 * 6. Evaluate success/failure of performed training step
 * 7. Decide how to retry, in case retry is required
 *
 * - enum snps_section_id dmem_section: SNPS_1D_DMEM / SNPS_2D_DMEM
 * - enum snps_training_method training_method:
 *   - SNPS_SEQUENCE_STEP_BY_STEP: Training procedure will performed step by step
 *   - SNPS_SEQUENCE_FULL: Training procedures (1D/2D) will performed as a full
 *			   sequence of all steps together
 */
static int snps_ddr_section_run(enum snps_section_id dmem_section, enum snps_training_method training_method)
{
	int ret = 0, step;
	/* Mark all sequence_flags cells as unused (-1) */
	int sequence_flags[SEQEUNCE_CTRL_1D_COUNT + 1] = { [0 ... (SEQEUNCE_CTRL_1D_COUNT)] = -1 };
	debug_enter();

	/* Set sequence_ctrl flags according to requested section */
	switch (dmem_section) {
	case SECTION_1D_DMEM:
		if (training_method == SEQUENCE_STEP_BY_STEP) {
			sequence_flags[0] = SEQUENCE_CTRL_DEV_INIT;
			sequence_flags[1] = SEQUENCE_CTRL_1D_RX_EN;
			sequence_flags[2] = SEQUENCE_CTRL_1D_WR_LVL;
			sequence_flags[3] = SEQUENCE_CTRL_1D_RD_DQS_1D;
			sequence_flags[4] = SEQUENCE_CTRL_1D_WR_DQ_1D;
			sequence_flags[5] = SEQUENCE_CTRL_1D_RD_DESKEW;
			sequence_flags[6] = SEQUENCE_CTRL_1D_MX_RD_LAT;
		} else
			sequence_flags[0] =  SEQUENCE_CTRL_1D_WR_LVL | SEQUENCE_CTRL_1D_RX_EN |
						SEQUENCE_CTRL_1D_RD_DQS_1D | SEQUENCE_CTRL_1D_WR_DQ_1D |
						SEQUENCE_CTRL_1D_RD_DESKEW | SEQUENCE_CTRL_1D_MX_RD_LAT;
	break;
	case SECTION_2D_DMEM:
		if (training_method == SEQUENCE_STEP_BY_STEP) {
			sequence_flags[0] = SEQUENCE_CTRL_DEV_INIT;
			sequence_flags[1] = SEQUENCE_CTRL_2D_READ_DQS;
			sequence_flags[2] = SEQUENCE_CTRL_2D_WRITE_DQ;
		} else
			sequence_flags[0] = SEQUENCE_CTRL_2D_READ_DQS | SEQUENCE_CTRL_2D_WRITE_DQ;
	break;
	default:
		printf("Error: %s: invalid DMEM section (%d)\n", __func__, dmem_section);
		ret = -1;
		goto fail;
	}

	/* Load dynamic updates for each section & step, trigger step, and evaluate results */
	for (step = 0; sequence_flags[step] != -1; ) {
		/* --- Step 3: Set required training step --- */
		snps_sequence_ctrl_set(sequence_flags[step]);

		/* load 1D/2D DMEM dynamic topology update */
		ret = snps_load_dynamic(dmem_section);
		if (ret != 0)
			goto fail;

		/* --- Step 4: Trigger PMU (PHY micro-controller unit) to execute training process --- */
		snps_trigger_pmu_training();

		/* --- Step 5: SNPS Firmware is running - read Mailbox for status & completion --- */
		ret = snps_mail_box_poll_completion();
		/* TODO: should we check success/failure using mailbox or message block?? */
		if (ret == MAIL_BOX_ERROR)
			goto fail;

		/* --- Step 6: read & log message block of recent step/s */
		ret = snps_message_block_read(sequence_flags[step]);
		if (ret != 0)
			goto fail;

		/* --- Step 7: Evaluate training results */
		ret = snps_evaluate_training_results(sequence_flags[step]);
		switch (ret) {
		case TRAINING_SEQUENCE_PASS:
			pr_debug("Training step/s sequence %x passed\n", sequence_flags[step]);
			step++;	/* set next step */
		break;
		case TRAINING_SEQUENCE_RETRY:
			/* TODO: consider relocate following retry settings to a specific routine */
			pr_debug("Training step/s sequence %x will be retried\n", sequence_flags[step]);
			/* TODO: set what step we need to retry:
			 * 1. Set new sequence_flags to be re-run from current section
			 * OR
			 * 2. In case 2D failed, and 1D step is required to be run again
			 *    then snps_ddr_section_trigger routine can be called recursively:
			 *    example:
			 *    -> call: snps_ddr_section_trigger(2D), which failed and now need 1D again
			 *    -> call snps_ddr_training(1D)  from here (recursively)
			 *    -> return here from 1D recursive call, and continue from same step of 2D training
			 */
		break;
		case TRAINING_SEQUENCE_FAIL:
			printf("Training step/s sequence %x failed\n", sequence_flags[step]);
			goto fail;
		}
	}

fail:
	debug_exit();
	return ret;
}

/* Prepare static & dynamic settings for requested section & trigger relevant training steps.
 * Generally, each section may have 3 stages:
 * 1. Static settings - as received from Synopsys release
 * 2. Static update: values that needs to be updated with static values
 *			(static values means the value doesn't change along the training flow)
 * 3. Dynamic update: values that can be derived only dynamically at runtime
 *			(i.e topology dependant variables)
 *			- dynamic update section is handled in snps_load_dynamic() routine
 * Arguments
 * - enum snps_section_id imem_section: SNPS_1D_IMEM / SNPS_2D_IMEM
 */
static int snps_ddr_training(enum snps_section_id imem_section)
{
	int ret, dmem_section = imem_section + 1;
	debug_enter();

	/* Verify requested imem section */
	if (imem_section != SECTION_1D_IMEM && imem_section != SECTION_2D_IMEM) {
		ret = -1;
		printf("Error: %s: invalid IMEM section (%d)\n", __func__, imem_section);
		goto fail;
	}

	pr_debug("SNPS DDR: %s training...\n" , dmem_section == SECTION_1D_DMEM ? "1D" : "2D");

	/* Prepare PHY init & 1D/2D training configuration (1D IMEM & static 1D DMEM) */
	ret = snps_ddr_section_init(imem_section);
	if (ret != 0)
		goto fail;

	/* run 1D/2D training execution */
	ret = snps_ddr_section_run(dmem_section, SEQUENCE_FULL);

fail:
	printf("SNPS DDR: %s training %s\n" , dmem_section == SECTION_1D_DMEM ? "1D" : "2D",
						ret == 0 ? "passed" : "failed");
	debug_exit();
	return ret;
}

/* initialize global settings */
static void snps_global_settings_init(unsigned long base_address)
{
	gd.current_run_num = 0;
	gd.base_address =  base_address;
}


int snps_init(unsigned int base_address)
{
	int ret;
	debug_enter();

	printf("Synopsys DDR43 PHY Firmware version: %s\n" , SNPS_DDR_PHY_FW_VERSION);

	/* initialize global settings */
	snps_global_settings_init(base_address);

	/* --- Step 1: load static PHY initialization --- */
	/* load PHY config static section, with no static update */
	snps_set_state(PHY_CONFIG);
	ret = snps_load_static(SECTION_PHY_CONFIG, STATIC_UPDATE_YES);
	if (ret != 0)
		goto fail;

	ret = snps_load_ext_swizzle_cfg();
	if (ret != 0)
		goto fail;

	/* load PHY initialization dynamic topology update */
	ret = snps_load_dynamic(SECTION_PHY_CONFIG);
	if (ret != 0)
		goto fail;

	ret = snps_poll_calibration_completion();
	if (ret != 0)
		goto fail;

	/* 1D training */
	snps_set_state(TRAINING_1D);
	ret = snps_ddr_training(SECTION_1D_IMEM);
	if (ret != 0)
		goto fail;

	/* fix 1d receiver centering training result */
	snps_crx_1d_fix();

	/* 2D training */
	snps_set_state(TRAINING_2D);

	ret = snps_ddr_training(SECTION_2D_IMEM);
	if (ret != 0)
		goto fail;

	/* load PHY Init Engine (PIE) static section, with no static update */
	snps_set_state(PHY_INIT_ENGINE);
	ret = snps_load_static(SECTION_PHY_INIT_ENGINE, STATIC_UPDATE_NO);
	if (ret != 0)
		goto fail;

fail:
	printf("SNPS DDR: training %s\n", ret == 0 ? "completed" : "failed");
	debug_exit();
	return ret;
}

