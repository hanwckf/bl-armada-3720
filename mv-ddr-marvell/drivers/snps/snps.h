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

#ifndef _SNPS_H_
#define _SNPS_H_

#include "mv_ddr_atf_wrapper.h"
/*#define SNPS_DEBUG*/
#if defined(SNPS_DEBUG)
#define pr_debug(args...)	printf(args)
#define debug_enter()	printf("----> Enter %s\n", __func__);
#define debug_exit()	printf("<---- Exit  %s\n", __func__);
#else
#define debug_enter()
#define debug_exit()
#define pr_debug(args...)
#endif

/* Dynamic update struct:
 * Dynamic initialization sequence is done with these adress & get_value API pairs:
 * CPU runs over these entries and sequentially calls get_update_value to derive
 * the variables required value for the current state,
 * and eventually writes address = get_update_value() */
struct snps_address_dynamic_update {
	u32 addr;
	u16 (*get_update_value)(void);
};

/* initialization flow is bisected to 3 stages:
 * 1. Static initialization: with Synopsys delivery input
 * 2. Static update: with fixed values for certain variables
 * 3. Dynamic update: with dynamic values for certain variable
 *    (topology Or run-time dependant)
 */
enum snps_section_load_state {
	LOAD_STATIC = 0,
	LOAD_STATIC_UPDATE,
	LOAD_DYNAMIC_UPDATE,
};

/* Section IDs (training procedure is represented by it's functional sections) */
enum snps_section_id {
	SECTION_PHY_CONFIG = 0,
	SECTION_1D_IMEM,
	SECTION_1D_DMEM,
	SECTION_2D_IMEM,
	SECTION_2D_DMEM,
	SECTION_PHY_INIT_ENGINE
};

enum snps_training_state {
	PHY_CONFIG = 0,
	TRAINING_1D,
	TRAINING_2D,
	PHY_INIT_ENGINE,
};

/* snps_static_update: set as yes if static section include static update state */
enum snps_static_update {
	STATIC_UPDATE_YES = 0,
	STATIC_UPDATE_NO
};

/* snps_training_method: select if training procedure will be done sequentially
 * or step by step (Step by step allows more thorough debug & more precise
 * result evaluation capabilities
 */
enum snps_training_method {
	SEQUENCE_STEP_BY_STEP = 0,
	SEQUENCE_FULL,
};

enum snps_training_result {
	TRAINING_SEQUENCE_PASS = 0,
	TRAINING_SEQUENCE_RETRY,
	TRAINING_SEQUENCE_FAIL,
};

enum snps_load_source_type {
	LOAD_SEQUENTIAL,
	LOAD_RANDOM,
};

/* struct to associate section ID and it's name
 * (used for more informative prints to user
 */
struct snps_section_name {
	enum snps_section_id section_id;
	char *section_name;
};

/* snps_section_content will specify each section's content and it's location.
 * each struct must include static initialization struct,
 * but static_update and dynamic_update parts are optional only
 */
struct snps_section_content {
	enum snps_section_id		section_id;
	enum snps_load_source_type  load_type;
	union {
		const struct snps_address_data	*random;
		const struct snps_seq_data *sequential;
	} load_static;
	const struct snps_address_data	*load_static_update;
	const struct snps_address_dynamic_update	*load_dynamic_update;
};


/* ************ Message Block definitions ************ */
#define SNPS_MAX_RUN_COUNT 10
/* NOTE:
 * This enum below must be aligned with message block field's definition
 * (defined in snps_msg_block_result msg_blk_data)
 */
enum snps_msg_block_result_id {
	MSG_BLK_PMU_REV = 0,
	MSG_BLK_CS_TEST_FAIL,
	MSG_BLK_1D_CDD_RW_1_0,
	MSG_BLK_1D_CDD_RW_0_3,
	MSG_BLK_2D_R0_RX_CLK_DELAY_MARGIN,
	MSG_BLK_MAX_ENTRY
};

/* snps_msg_block_result describes each message block and training results fields
 * with this info CPU will read all required results and log them accordingly */
struct snps_msg_block_result {
	enum snps_msg_block_result_id result_id;
	int addr;
	u16 mask;
	u16 offset;
};

/* snps_msg_block_result_log logs all required training results for each
 * performed training step
 */
struct snps_msg_block_result_log {
	int run_number;
	/* TODO: add current state: PHY INIT/ 1D/ 2D/ PIE */
	int sequence_flags;
	u16 msg_blk[MSG_BLK_MAX_ENTRY];
};

/* ************ Mail Box definitions ************ */
/* TODO: update maximum supported mailbox according to actual message count */
#define MAILBOX_MSG_MAX_COUNT	100
/* Maximum supported arguments is 32, so we'll take some buffer */
#define MAILBOX_MSG_ARGUMENT_MAX_COUNT 36

/* snps_mailbox_msg_type uses unique values to distinct between 'Major' and
 * 'Stream' type of messages.
 * enum 'MB_MSG_TYPE_MAJOR' below is used to mark 'Major' type of messages in
 * argument array of messages log.
 * This way, when printing the messages after we finished logging them in
 * messages array, we know how to distinct between 'major' and 'stream' types.
 */
enum snps_mailbox_msg_type {
	MB_MSG_TYPE_STREAM = 0xfffe,
	MB_MSG_TYPE_MAJOR
};

/* TODO: update retry count & polling delay according to actual requirements */
#define MAIL_BOX_ERROR		-1
#define MAILBOX_RETRY_COUNT	20000
#define MAILBOX_POLLING_DELAY	500	/* 300ms between reads */

/* mail_box_major_message: describe 'Major' type of messages with it's ID and string content */
struct mail_box_major_message {
	u16 message_id;
	char *string;
};

/* mail_box_stream_message: describe 'Stream' type of messages with it's ID and string & argument array content */
struct mail_box_stream_message {
	u16 message_id;
	u16 argument_count;
	u16 log_level;
	/* TODO: explain why multiplying arg_count by 2 and adding 1 */
	const char *string[(MAILBOX_MSG_ARGUMENT_MAX_COUNT * 2) + 1];
};

enum snps_major_msg_id {
	MB_MAJOR_ID_END_INIT = 0,
	MB_MAJOR_ID_FINE_WR_LVL,
	MB_MAJOR_ID_READ_EN_TRANING,
	MB_MAJOR_ID_READ_DELAY_CNTR_OPT,
	MB_MAJOR_ID_WRITE_DELAY_CNTR_OPT,
	MB_MAJOR_ID_2D_WRITE_DELAY_CNTR_OPT,	/* read delay/voltage center */
	MB_MAJOR_ID_2D_READ_DELAY_CNTR_OPT,	/* write delay /voltage center */
	MB_MAJOR_ID_TRAINING_SUCCESS,
	MB_MAJOR_ID_STREAM_MSG,
	MB_MAJOR_ID_MAX_READ_LATENCY,
	MB_MAJOR_ID_MAX_READ_DQ_DESKEW,
	MB_MAJOR_ID_RESERVED,
	MB_MAJOR_ID_LRDIMM_SPECIFIC_TRAINING,
	MB_MAJOR_ID_CA_TRAINING,
	MB_MAJOR_ID_MPR_READ_DELAY_CENTER_OPT = 0xfd,
	MB_MAJOR_ID_WRITE_LVL_COARSE_DELAY = 0xfd,
	MB_MAJOR_ID_TRAINING_FAILED = 0xff
};

enum snps_log_level {
	LOG_LEVEL_10 = 0,
	LOG_LEVEL_20,
	LOG_LEVEL_30,
	LOG_LEVEL_40,
	LOG_LEVEL_50
};
/*************************************************/

/* global data, settings, mail box, message block, etc..) */
struct snps_global_data {
	unsigned long  base_address;	/* SNPS unit Base address */
	enum snps_training_state training_state;

	/* Mail Box */
	int messages[MAILBOX_MSG_MAX_COUNT][MAILBOX_MSG_ARGUMENT_MAX_COUNT];
	int message_log_index;

	/* Message Block & training results data */
	struct snps_msg_block_result_log results[SNPS_MAX_RUN_COUNT];
	int current_run_num;
};

struct snps_global_data gd;


extern int static_section_completed;
#define SNPS_LAST_RUN	(gd.current_run_num - 1)
/* this macro writes 'data' to specified 'address' offset,
 * without support for debug prints, since static sections are too large to be printed */
#define SNPS_STATIC_WRITE(offset, data)	mmio_write_16((uint64_t)(gd.base_address + (2 * offset)), (uint16_t)data)

int snps_message_block_read(int sequence_flags);
int snps_get_result(enum snps_msg_block_result_id result_id, int run_number, int sequence_ctrl);
void snps_fw_write(u32 addr, u16 data);
u16 snps_read(u32 addr);
void snps_csr_access_set(u16 enable);
void snps_sequence_ctrl_set(int sequence_flags);
u16 snps_sequence_ctrl_get(void);
void snps_trigger_pmu_training(void);
int snps_mail_box_log_init(void);
int snps_mail_box_read(void);
void snps_mail_box_log_dump(void);
int snps_poll_calibration_completion(void);
enum snps_training_state snps_get_state(void);
void snps_set_state(enum snps_training_state training_state);
void snps_crx_1d_fix(void);
#endif	/* _SNPS_H_ */
