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

#include "snps.h"
#include "snps_regs.h"
#include "snps_mailbox_stream.h"
#include "ddr_topology_def.h"
#include "ddr3_training_ip_db.h"

struct mail_box_major_message mb_major_messages[] = {
/*	ID,					Message string	*/
	{MB_MAJOR_ID_END_INIT,			"End of initialization"},
	{MB_MAJOR_ID_FINE_WR_LVL,		"End of fine write leveling"},
	{MB_MAJOR_ID_READ_EN_TRANING,		"End of read enable training"},
	{MB_MAJOR_ID_READ_DELAY_CNTR_OPT,	"End of read delay center optimization"},
	{MB_MAJOR_ID_WRITE_DELAY_CNTR_OPT,	"End of write delay center optimization"},
	{MB_MAJOR_ID_2D_WRITE_DELAY_CNTR_OPT,	"End of 2D read delay/voltage center optimization"},
	{MB_MAJOR_ID_2D_READ_DELAY_CNTR_OPT,	"End of 2D write delay /voltage center optimization"},
	{MB_MAJOR_ID_TRAINING_SUCCESS,		"Training has run successfully (firmware complete)"},
	{MB_MAJOR_ID_STREAM_MSG,		"Steam message"},
	{MB_MAJOR_ID_MAX_READ_LATENCY,		"End of max read latency training"},
	{MB_MAJOR_ID_MAX_READ_DQ_DESKEW,	"End of read dq deskew training"},
	{MB_MAJOR_ID_RESERVED,			"End of LCDL offset calibration"},
	{MB_MAJOR_ID_LRDIMM_SPECIFIC_TRAINING,	"End of LRDIMM Specific training (DWL, MREP, MRD and MWD)"},
	{MB_MAJOR_ID_CA_TRAINING,		"End of CA training"},
	{MB_MAJOR_ID_MPR_READ_DELAY_CENTER_OPT, "End of MPR read delay center optimization"},
	{MB_MAJOR_ID_WRITE_LVL_COARSE_DELAY,	"End of Write leveling coarse delay"},
	{MB_MAJOR_ID_TRAINING_FAILED,		"Training has failed (firmware complete)"}
};

/* ==================================================================== */
/*				Message Block				*/
/* ==================================================================== */
/* Message Block fields:
 * Definition of all relevant fields from message block result output
 * NOTE: The array below must be aligned with message block enum definition
    (defined with enum snps_msg_block_result_id) */
struct snps_msg_block_result msg_blk_data[] = {
/* result_id			addr					mask		offset */
	{MSG_BLK_PMU_REV,	REG_54001_PMU_REV_ADDR,			NO_MASK,	0},
	{MSG_BLK_CS_TEST_FAIL,	REG_5400A_CS_TEST_FAIL_ADDR,		BYTE_MASK,	0},
	{MSG_BLK_1D_CDD_RW_1_0,	REG_54024_1D_CDD_RW_1_0_RW_0_3_ADDR,	BYTE_MASK,	0},
	{MSG_BLK_1D_CDD_RW_0_3,	REG_54024_1D_CDD_RW_1_0_RW_0_3_ADDR,	BYTE_MASK,	BYTE_OFFSET},
	{MSG_BLK_2D_R0_RX_CLK_DELAY_MARGIN, REG_54012_2D_R0_RX_CLK_DELAY_MARGIN_ADDR, BYTE_MASK, BYTE_OFFSET},
};

/*
 * This routine will read (and log) message block result output,
 * according to defined fields in msg_blk_data array.
 */
int snps_message_block_read(int sequence_flags)
{
	int i, ret = 0;
	u16 val;
	debug_enter();

	/* TODO: once reached maximum run count, is there a need for a cyclic array? */
	if (gd.current_run_num >= SNPS_MAX_RUN_COUNT) {
		printf("Error: reached maximum number of runs - can't log anymore results");
		ret = -1;
		goto fail;
	}

	/* On first run, verify correctness of message block definitions:
	 * - In 'enum snps_msg_block_result_id' training results are defined with enum per results.
	 *   this enum is needed to read results from results array after training.
	 * - In 'msg_blk_data' we describe each result's information (address, mask, offset)
	 * So, the enum IDs and the 'msg_blk_data' struct must be aligned with each other */
	if (gd.current_run_num == 0)
		for (i = 0; i < MSG_BLK_MAX_ENTRY; i++)
			if (msg_blk_data[i].result_id != i) {
				printf("Error: %s: invalid message block field %d (expected %d)\n"
						, __func__, i, msg_blk_data[i].result_id);
				printf("Result's description struct isn't aligned with it's result ID enum\n");
				ret = -1;
				goto fail;
			}

	/* Log current run number & sequence_ctrl flags of current run */
	gd.results[gd.current_run_num].sequence_flags = sequence_flags;
	gd.results[gd.current_run_num].run_number = gd.current_run_num;

	/* Enable CSRs access (disable firmware access) */
	snps_csr_access_set(MICRO_CONT_MUX_SEL_ENABLE);

	/* Read all defined registers from message block (defined in msg_blk_data array) */
	for (i = 0; i < MSG_BLK_MAX_ENTRY; i++) {
		val = snps_read(msg_blk_data[i].addr);
		val = (val >> msg_blk_data[i].offset) & msg_blk_data[i].mask;
		gd.results[gd.current_run_num].msg_blk[i] = val;
		pr_debug("result %d, read(0x%x), mask(0x%x), = 0x%x\n", msg_blk_data[i].result_id,
				msg_blk_data[i].addr, msg_blk_data[i].mask, val);
	}
	/* Isolate CSRs - allow firmware unrestricted access */
	snps_csr_access_set(MICRO_CONT_MUX_SEL_ISOLATE);

	/* Increment next run number */
	gd.current_run_num++;

fail:
	debug_exit();
	return ret;
}

/* This routine returns a result value, from message block results log.
 * - enum snps_msg_block_result_id result_id: requested result (register or field)
 * - int run_number: requested run_number
 * - int sequence_ctrl: sequence training step:
 *			- if requested result is relevant only to a certain sequence training
 *			  step, than this field specifies the requested step id.
 *			- else SNPS_SEQEUNCE_CTRL_ALL should be used.
 *			NOTE: if specific sequence step requested, and this step wasn't
 *			performed in the requested run number, than value won't
 *			be returned (-1 will be returned).
 */
int snps_get_result(enum snps_msg_block_result_id result_id, int run_number, int sequence_ctrl)
{
	int ret;
	debug_enter();

	/* Verify requested run_number is valid */
	if (run_number >= gd.current_run_num) {
		printf("Error: requested invalid result run number (%d, max = %d)\n"
				, run_number, gd.current_run_num - 1);
		ret = -1;
		goto fail;
	}

	/* Verify requested result is valid */
	if (result_id >= MSG_BLK_MAX_ENTRY) {
		printf("Error: requested invalid  message block result id (%d, max = %d)\n"
				, result_id, MSG_BLK_MAX_ENTRY - 1);
		ret = -1;
		goto fail;
	}

	/* If requested result from a specific sequence training step,
	 * verify that the requested training step was performed in the requested run_number */
	if (sequence_ctrl != SEQEUNCE_CTRL_ALL &&
		!(gd.results[run_number].sequence_flags & sequence_ctrl)) {
		printf("Error: requested sequence_ctrl (0x%x) wasn't performed on run number %d\n"
				, sequence_ctrl, run_number);
		ret = -1;
		goto fail;
	}

	ret = gd.results[run_number].msg_blk[result_id];
fail:
	debug_exit();
	return ret;
}

/* ==================================================================== */
/*				Mail Box				*/
/* ==================================================================== */
/* Message log is represented by 2D array, with an array for each message:
 * - Message id is saved at cell#0 of array: messages[message_index][0]
 * - Message arguments (if exists) are saved at the following cells: messages[message_index][1..]*/

/* Initialize & reset mail box log
 * NOTE: this API must be called before using snps_mail_box_read_message API,
 * to log messages */
int snps_mail_box_log_init(void)
{
	int i, j, ret = 0;
	debug_enter();

	/* go over all message array entries and clear them from previous data */
	for (i = 0; i < MAILBOX_MSG_MAX_COUNT; i++)
		for (j = 0; j < MAILBOX_MSG_ARGUMENT_MAX_COUNT; j++)
			gd.messages[i][j] = -1;

	/* clear logged message counter */
	gd.message_log_index = 0;

	debug_exit();
	return ret;
}

int snps_poll_reg(u32 reg, u32 mask, u32 value, u32 retry_count, u32 delay)
{
	int ret = 0;
	debug_enter();

	while (retry_count && (snps_read(reg) & mask) != value) {
		retry_count--;
		mdelay(1); /* TODO: check the delay needed here */
	};

	/* if reached maximum retry count, return failure */
	if (retry_count == 0)
		ret = -1;

	debug_exit();
	return ret;
}

/* get_mail: Read message identifier  */
static int snps_mail_box_get_message(enum snps_mailbox_msg_type msg_type)
{
	int ret, message, stream_msg_id;
	debug_enter();

	/* Wait for pending message: poll the UctWriteProtShadow, looking for 0 */
	if (snps_poll_reg(UCT_SHADOW_REGS, UCT_SHADOW_REGS_WRITE_PROT_MASK, 0,
			MAILBOX_RETRY_COUNT, MAILBOX_POLLING_DELAY) != 0) {
		printf("SNPS Error: timeout when waiting for mail box message\n");
		ret = MAIL_BOX_ERROR;
		goto fail;
	}

	/* Now read the actual message content:
	 * Read the UctWriteOnlyShadow register to get the major message number */
	message = snps_read(UCT_WRITE_ONLY_SHADOW_REG) & UCT_WRITE_ONLY_SHADOW_MASK;

	/* If reading a streaming message, also read the UctDatWriteOnlyShadow register */
	if (msg_type == MB_MSG_TYPE_STREAM) {
		stream_msg_id = snps_read(UCT_DAT_WRITE_ONLY_SHADOW_REG) & UCT_DAT_WRITE_ONLY_SHADOW_MASK;
		message |= (stream_msg_id << MAILBOX_STREAM_MSG_ID_OFFSET);
	}

	/* Write the DctWriteProt to 0 to acknowledge the receipt of the message */
	snps_fw_write(DCT_WRITE_PROT_REG, 0);

	/* Poll the UctWriteProtShadow, looking for 1 */
	if (snps_poll_reg(UCT_SHADOW_REGS, UCT_SHADOW_REGS_WRITE_PROT_MASK, 1,
				MAILBOX_RETRY_COUNT, MAILBOX_POLLING_DELAY) != 0) {
		printf("SNPS Error: timeout when acknowledging receipt of mail box message\n");
		ret = MAIL_BOX_ERROR;
		goto fail;
	}

	/* When 1 is seen, write the DctWriteProt to 1 to complete the protocol */
	snps_fw_write(DCT_WRITE_PROT_REG, DCT_WRITE_PROT_MASK);

	ret = message;

fail:
	debug_exit();
	return ret;
}

/* Read mail box message and log it.
 * Received messages and their arguments were logged in 'messages' global array
 * - cell#0: message ID
 * - cell#1:
 *		- if equals to MB_MSG_TYPE_MAJOR, than message is 'major' message
 *		- else if equals to MB_MSG_TYPE_STREAM, message is 'stream' type with arguments
 * - cells[#2..end]: message arguments (in case message is 'stream')
 * Routine returns message type
 */
int snps_mail_box_read(void)
{
	int i, argument_count, message_id, message_arg, stream_message_id, ret = 0;
	debug_enter();

	if (gd.message_log_index >= MAILBOX_MSG_MAX_COUNT) {
		printf("Error: Mail Box overflow (too many messages, max is %d)\n", MAILBOX_MSG_MAX_COUNT);
		ret = MAIL_BOX_ERROR;
		goto fail;
	}

	/* first read 'MAJOR' type of message */
	message_id = snps_mail_box_get_message(MB_MSG_TYPE_MAJOR);
	if (message_id == -1) {
		ret = MAIL_BOX_ERROR;
		goto fail;
	}

	pr_debug("%s: read message #%d = 0x%x\n", __func__, gd.message_log_index, message_id);

	/* if message type is 'stream', read message content and arguments (if exists) */
	if (message_id == MB_MAJOR_ID_STREAM_MSG) {
		stream_message_id = snps_mail_box_get_message(MB_MSG_TYPE_STREAM);
		if (stream_message_id == -1) {
			ret = MAIL_BOX_ERROR;
			goto fail;
		}
		/* Mail Box 'stream' message format:
		 * - First integer is the message ID
		 * - Lower byte of message ID is the argument count of the message
		 */
		argument_count =  stream_message_id & MAILBOX_STREAM_MSG_ARG_COUNT_MASK;
			pr_debug("message is 'stream' type: 'stream' ID=0x%x, arg_count=%d\n"
					, stream_message_id, argument_count);
		if (argument_count >= MAILBOX_MSG_ARGUMENT_MAX_COUNT) {
			printf("Error: Mail Box message with too many arguments (%d, supported %d)\n"
					, argument_count, MAILBOX_MSG_ARGUMENT_MAX_COUNT);
			ret = MAIL_BOX_ERROR;
			goto fail;
		}

		/* If arguments exists, save them in message log, starting cell#2 */
		for (i = 0; i < argument_count; i++) {
			message_arg = snps_mail_box_get_message(MB_MSG_TYPE_STREAM);
			/* pr_debug("arg#%d = 0x%x\n" , i, message_arg); */
			if (message_arg == -1) {
				ret = MAIL_BOX_ERROR;
				goto fail;
			}
			gd.messages[gd.message_log_index][i + 2] = message_arg;
		}
		 /* Mark 'stream' type in cell#1 of messages log */
		gd.messages[gd.message_log_index][1] = MB_MSG_TYPE_STREAM;
		message_id = (stream_message_id >> MAILBOX_STREAM_MSG_ID_OFFSET);
	} else {
		pr_debug("message is 'major' type (MSG ID = 0x%x)\n", message_id);
		/* else message type is 'major', with no arguments,
		 * so mark 'major' type in cell#1 of messages log
		 * NOTE: this mark will be used in snps_mail_box_log_dump(),
		 *	 to print message string from 'major' string dictionary) */
		gd.messages[gd.message_log_index][1] = MB_MSG_TYPE_MAJOR;
		ret = message_id;
	}

	/* Save message_id in cell#0 of current message log array,
	 * while next cells will hold the message arguments (if exists) */
	gd.messages[gd.message_log_index++][0] = message_id;

fail:
	debug_exit();
	return ret;
}

/* print 'major' type of meesages:
 * go over major messages dictionary struct 'mb_major_messages', find required message
 * according to msg_id (since dictionary ID's are not continuous),
 * and print the required message string, according to it's message id. */
static void snps_mail_box_print_major_msg(int msg_id)
{
	int major_msg_count = sizeof(mb_major_messages) / sizeof(mb_major_messages[0]);

	for (int i = 0; i <= major_msg_count; i++)
		if (mb_major_messages[i].message_id == msg_id) {
			pr_debug("%s\n", mb_major_messages[i].string);
			return;
		}

	/* in case msg_id wasn't found in major messages database */
	printf("SNPS ERROR: invalid 'major' message id from Mail Box (0x%x)\n", msg_id);
}

/* print 'stream' type of meesages:
 * go over stream messages dictionary struct 'mb_major_messages', find required message
 * according to msg_id (since dictionary ID's are not continuous),
 * and print the required message string, according to it's message id.
 * arguments:
 * - msg_id: Message ID, to be matched with message's string dictionary
 * - msg_log_index: Message index in log array 'messages'
 * - dmem_section: 1D / 2D - to differentiate the correct message string dictionary
 */
static void snps_mail_box_print_stream_msg(int msg_id, int msg_log_index)
{
	const struct mail_box_stream_message *mb_stream_database, *stream_msg;
	/* cells[#2..end]: message arguments, for 'stream' messages */
	int j, stream_msg_count;
	__attribute__((unused)) int  arg_index = 2;
	debug_enter();

	/* 1D and 2D have different mail box dictionary database */
	mb_stream_database = (snps_get_state() == TRAINING_2D ? two_d_messages : one_d_messages);
	stream_msg_count = sizeof(mb_stream_database) / sizeof(mb_stream_database[0]);

	/* Most of the dictionary msg_id's are continuous, so first check database if
	 * msg_id cell holds this msg_id */
	if (mb_stream_database[msg_id].message_id == msg_id)
		stream_msg = &mb_stream_database[msg_id];
	else {
		/* go over dictionary and find msg_id cell */
		for (int i = 0; i <= stream_msg_count; i++) {
			if (mb_stream_database[i].message_id == msg_id)
				stream_msg = &mb_stream_database[msg_id];
		}

		/* else, in case msg_id wasn't found in 'stream' messages database */
		printf("SNPS ERROR: invalid %s 'stream' message id from Mail Box (0x%x)\n",
			snps_get_state() == TRAINING_2D ? "2D" : "1D", msg_id);
		return;
	}

	/* print message (stream_msg) in 'stream' format, which includes arguments.
	 * message arguments are held in 'messages' array, starting from cell#1 */
	for (j = 0; j < (stream_msg->argument_count * 2) + 1; j++) {
		/* 'stream' message format may have decimal and hex values, for example:
		 * 0x00aa0002 "\nPMU3:InvClk ATxDly setting:%x margin:%d\n"
		 * This type of complex string is represented in 'struct mail_box_stream_message'
		 * by an array of strings, and each argument is described by "HEX" or "INT":
		 *	{"\nPMU3:InvClk ATxDly setting", "HEX", "margin:", "INT", "\n"}
		 * So for each 'stream' message we need to go over it's array of strings,
		 * and print the arguments with their required format.
		 */
		if (strcmp(stream_msg->string[j], "INT") == 0)
			pr_debug("%d", gd.messages[msg_log_index][arg_index++]);
		else if (strcmp(stream_msg->string[j], "HEX") == 0)
			pr_debug("0x%x", gd.messages[msg_log_index][arg_index++]);
		else
			pr_debug("%s", stream_msg->string[j]);
		}
}

/* Go over all logged messages from previous training step and print them.
 * Received messages and their arguments were logged in 'messages' global array
 * - cell#0: message ID
 * - cell#1:
 *		- if equals to MB_MSG_TYPE_MAJOR, than message is 'major' message
 *		- else message is 'stream' type, with arguments, and cells[#1..end]
 *		  of each message hold message arguments
 *
 * For each message we translate from it's message ID to it's actual text
 * string, with the help of the 1D & 2D message dictionaries
 * (one_d_messages, two_d_messages)
 */
void snps_mail_box_log_dump(void)
{
	int i, msg_id;
	debug_enter();

	pr_debug("Reading %d messages from Mail Box\n", gd.message_log_index);
	for (i = 0; i < gd.message_log_index; i++) {
		/* Message ID is in cell#0 of 'messages' array */
		msg_id = gd.messages[i][0];

		/* messages of 'major' type are marked in cell#1 of message */
		if (gd.messages[i][1] == MB_MSG_TYPE_MAJOR)
			snps_mail_box_print_major_msg(msg_id);
		else
			/* Else, print message in 'stream' format, which includes arguments */
			snps_mail_box_print_stream_msg(msg_id, i);
	}

	debug_exit();
}

/* this routine writes 'data' to specified 'address' offset, with optional debug print support */
void snps_fw_write(u32 offset, u16 data)
{
	/* TODO: add log-level support (if log_level >= write_level, print write) */
	pr_debug("Write: 0x%x = 0x%x\n", offset, data);
	mmio_write_16((uintptr_t)(gd.base_address + (2 * offset)), (uint16_t)data);
#if defined(SNPS_DEBUG)
	snps_read(offset);
#endif
}

u16 snps_read(u32 offset)
{
	/* TODO: add log-level support (if log_level >= write_level, print write) */
	pr_debug("Read: 0x%x.. ", offset);
	u16 val = mmio_read_16(gd.base_address + (2 * offset));
	pr_debug(" = 0x%x\n", val);
	return val;
}

/* snps_csr_access_set:
 * - Enable access to the internal CSRs: set the MicroContMuxSel CSR to 0.
 *   This allows the memory controller unrestricted access to the configuration CSRs.
 * - Isolate the APB access from the internal CSRs: set the MicroContMuxSel CSR to 1.
 *   This allows the firmware unrestricted access to the configuration CSRs.
 */
void snps_csr_access_set(u16 enable)
{
	debug_enter();

	snps_fw_write(MICRO_CONT_MUX_SEL_REG, enable);

	debug_exit();
}


/* this routine will go over all sequence_ctrl bits, and print
 * the names of the training sequence steps */
static void snps_sequence_ctrl_print(void)
{
	int i = 1;
	int sequence_ctrl = snps_sequence_ctrl_get();

	if (sequence_ctrl & SEQUENCE_CTRL_DEV_INIT)
		pr_debug("%d. Device/PHY Init\n", i++);

	if (snps_get_state() == TRAINING_1D) {
		if (sequence_ctrl & SEQUENCE_CTRL_1D_WR_LVL)
			pr_debug("%d. 1D Write leveling\n", i++);
		if (sequence_ctrl & SEQUENCE_CTRL_1D_RX_EN)
			pr_debug("%d. 1D Read gate training\n", i++);
		if (sequence_ctrl & SEQUENCE_CTRL_1D_RD_DQS_1D)
			pr_debug("%d. 1D read dqs training\n", i++);
		if (sequence_ctrl & SEQUENCE_CTRL_1D_WR_DQ_1D)
			pr_debug("%d. 1D write dq training\n", i++);
		if (sequence_ctrl & SEQUENCE_CTRL_1D_RD_DESKEW)
			pr_debug("%d. 1D Per lane read dq deskew training\n", i++);
		if (sequence_ctrl & SEQUENCE_CTRL_1D_MX_RD_LAT)
			pr_debug("%d. 1D Max read latency training\n", i++);
	} else {	/* 2D Training sequence options */
		if (sequence_ctrl & SEQUENCE_CTRL_2D_READ_DQS)
			pr_debug("%d. 2D read dqs training\n", i++);
		if (sequence_ctrl & SEQUENCE_CTRL_2D_WRITE_DQ)
			pr_debug("%d. 2D write dq training\n", i++);
	}

	if (i == 1)
		pr_debug("ERROR: no valid sequence_ctrl flags were set!\n");
}

/* Set required sequence_ctrl training steps to be run by FW */
void snps_sequence_ctrl_set(int sequence_flags)
{
	debug_enter();

	/* Isolate CSRs - allow firmware unrestricted access */
	snps_csr_access_set(MICRO_CONT_MUX_SEL_ENABLE);

	/* Set Device/PHY initialization ctrl flag - Should always be set */
	sequence_flags |= SEQUENCE_CTRL_DEV_INIT;
	snps_fw_write(REG_5400B_SEQUENCE_CTRL, sequence_flags);

	/* Isolate CSRs - allow firmware unrestricted access */
	snps_csr_access_set(MICRO_CONT_MUX_SEL_ISOLATE);

	pr_debug("Setting the following %s training step/s: (0x%x)\n",
		snps_get_state() == TRAINING_2D ? "2D" : "1D", sequence_flags);
	snps_sequence_ctrl_print();

	debug_exit();
}

/* Set required sequence_ctrl training steps to be run by FW */
u16 snps_sequence_ctrl_get(void)
{
	u16 val;
	debug_enter();
	/* Isolate CSRs - allow firmware unrestricted access */
	snps_csr_access_set(MICRO_CONT_MUX_SEL_ENABLE);

	val = snps_read(REG_5400B_SEQUENCE_CTRL);

	/* Isolate CSRs - allow firmware unrestricted access */
	snps_csr_access_set(MICRO_CONT_MUX_SEL_ISOLATE);

	debug_exit();

	return val;
}


/* snps_trigger_pmu_training: trigger execution of SNPS PHY Training */
void snps_trigger_pmu_training(void)
{
	debug_enter();
	/* Reset the firmware micro-controller by writing the MicroReset CSR reg:
	 * 1. Set the StallToMicro and ResetToMicro fields to 1.
	 * 2. Rewrite the CSR so that only the StallToMicro remains set.
	 */

	snps_fw_write(MICRO_RESET_CSR_REG, MICRO_RESET_CSR_STALL_TO_MICRO |
			MICRO_RESET_CSR_RESET_TO_MICRO);

	snps_fw_write(MICRO_RESET_CSR_REG, MICRO_RESET_CSR_STALL_TO_MICRO);

	/* Begin execution of the training firmware by setting the MicroReset CSR to 0 */
	snps_fw_write(MICRO_RESET_CSR_REG, 0);

	debug_exit();
}

int snps_poll_calibration_completion(void)
{
	int ret = 0;
	debug_enter();

	mdelay(20); /* TODO: need to check actual calibration completed indication instead of delay */

	debug_exit();
	return ret;
}

void snps_set_state(enum snps_training_state training_state)
{
	gd.training_state = training_state;
}

enum snps_training_state snps_get_state(void)
{
	return gd.training_state;
}

#define RX_PB_DLY_SHIFT		0x1d
#define RX_CLK_DLY_SHIFT	4	/* TODO: check if frequency dependent */
static void snps_rx_pb_dly_add(u8 rank, u8 sphy, u8 nibble)
{
	debug_enter();

	u8 lane, start_lane = 0, end_lane = 4; /* low nibble */
	u16 rx_dly, val;
	u32 addr;

	if (nibble == UP_NIBL) { /* upper nibble */
		start_lane = 4;
		end_lane = 9; /* dbi signal (lane 8) to be delayed too */
	}

	/* add rx per-bit delay to all dq signals (lanes) within a given nibble */
	for (lane = start_lane; lane < end_lane; lane++) {
		addr = PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, sphy, REG_68_RX_PB_DLY(rank, lane));
		val = snps_read(addr);
		rx_dly = (val >> RX_PB_DLY_OFFS) & RX_PB_DLY_MASK;
		rx_dly += RX_PB_DLY_SHIFT;
		val &= ~(RX_PB_DLY_MASK << RX_PB_DLY_OFFS);
		val |= (rx_dly & RX_PB_DLY_MASK) << RX_PB_DLY_OFFS;
		snps_fw_write(addr, val);
	}

	/* center corresponding dqs signal (rx clk) */
	addr = PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, sphy, REG_8C_RX_CLK_DLY(rank, nibble));
	val = snps_read(addr);
	val &= ~(RX_CLK_DLY_MASK << RX_CLK_DLY_OFFS);
	val |= (RX_CLK_DLY_SHIFT & RX_CLK_DLY_MASK) << RX_CLK_DLY_OFFS;
	snps_fw_write(addr, val);

	debug_exit();
}

/*
 * workaround preventing 2d receiver centering failure;
 * add delay to all dq and dqs signals, if rx clk delay 1d training
 * result is less than two (corresponds to left-aligned dq signals
 * with respect to dqs signal)
 */
#define RX_CLK_DLY_MIN	2
void snps_crx_1d_fix(void)
{
	debug_enter();

	u8 rank, sphy;
	u8 cs_num = mv_ddr_cs_num_get();
	u8 sphy_num = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	u16 val, low_nibble_clk, upper_nibble_clk;
	u32 addr;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/* enable access to csr registers */
	snps_csr_access_set(MICRO_CONT_MUX_SEL_ENABLE);

	for (rank = 0; rank < cs_num; rank++) {
		for (sphy = 0; sphy < sphy_num; sphy++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sphy);

			/* read low nibble rx clk dly */
			addr = PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, sphy,
						REG_8C_RX_CLK_DLY(rank, LOW_NIBL));
			val = snps_read(addr);
			low_nibble_clk = (val >> RX_CLK_DLY_OFFS) & RX_CLK_DLY_MASK;

			/* read upper nibble rx clk dly */
			addr = PHY_REG_ADDR_MAP(P_STATE_0, BLK_TYPE_DBYTE, sphy,
						REG_8C_RX_CLK_DLY(rank, UP_NIBL));
			val = snps_read(addr);
			upper_nibble_clk = (val >> RX_CLK_DLY_OFFS) & RX_CLK_DLY_MASK;

			/* check the criteria and fix low nibble */
			if (low_nibble_clk < RX_CLK_DLY_MIN)
				snps_rx_pb_dly_add(rank, sphy, LOW_NIBL);

			/* check the criteria and fix upper nibble */
			if (upper_nibble_clk < RX_CLK_DLY_MIN)
				snps_rx_pb_dly_add(rank, sphy, UP_NIBL);
		}
	}

	/* disable access to csr registers*/
	snps_csr_access_set(MICRO_CONT_MUX_SEL_ISOLATE);

	debug_exit();
}
