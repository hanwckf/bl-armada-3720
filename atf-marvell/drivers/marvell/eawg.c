/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */
#include <delay_timer.h>
#include <debug.h>
#include <eawg.h>
#include <mmio.h>
#include <mvebu_def.h>
#include <stdio.h>

#define EAWG_BASE_REGS(ap)		MVEBU_AR_RFU_BASE(ap) + 0x6000
#define EAWG_WRITE_ADDR_REG(ap)		(EAWG_BASE_REGS(ap) + 0x0)
#define EAWG_WRITE_DATA_REG(ap)		(EAWG_BASE_REGS(ap) + 0x4)
#define EAWG_WRITE_DELAY_REG(ap)	(EAWG_BASE_REGS(ap) + 0x8)

#define EAWG_AXI_BASE_REG(ap)		(EAWG_BASE_REGS(ap) + 0x12)
#define EAWG_AXI_AUSER_OFFSET		12
#define EAWG_AXI_AUSER_MASK		(0xffff << EAWG_AXI_AUSER_OFFSET)
#define EAWG_AXI_LOCK_OFFSET		8
#define EAWG_AXI_LOCK_MASK		(0x3 << EAWG_AXI_LOCK_OFFSET)
#define EAWG_AXI_CACHE_OFFSET		4
#define EAWG_AXI_CACHE_MASK		(0xf << EAWG_AXI_CACHE_OFFSET)
#define EAWG_AXI_PROT_OFFSET		0
#define EAWG_AXI_PROT_MASK		(0x7 << EAWG_AXI_PROT_OFFSET)

#define EAWG_INTERRUPT_CONFIG_REG(ap)	(EAWG_BASE_REGS(ap) + 0x14)
#define EAWG_FAIL_STATUS_OFFSET		6
#define EAWG_FAIL_STATUS_MASK		(0x1 << EAWG_FAIL_STATUS_OFFSET)
#define EAWG_CLEAR_FAIL_OFFSET		5
#define EAWG_CLEAR_FAIL_MASK		(0x1 << EAWG_CLEAR_FAIL_OFFSET)
#define EAWG_MASK_FAIL_OFFSET		4
#define EAWG_MASK_FAIL_MASK		(0x1 << EAWG_MASK_FAIL_OFFSET)
#define EAWG_DONE_OFFSET		2
#define EAWG_DONE_MASK			(0x1 << EAWG_DONE_OFFSET)
#define EAWG_CLEAR_DONE_OFFSET		1
#define EAWG_CLEAR_DONE_MASK		(0x1 << EAWG_CLEAR_DONE_OFFSET)
#define EAWG_MASK_DONE_OFFSET		0
#define EAWG_MASK_DONE_MASK		(0x1 << EAWG_MASK_DONE_OFFSET)

#define EAWG_START_BASE_REG(ap)		(EAWG_BASE_REGS(ap) + 0x1c)
#define EAWG_START_OFFSET		0
#define EAWG_START_MASK			(0x1 << EAWG_START_OFFSET)
#define EAWG_PAUSE_OFFSET		1
#define EAWG_PAUSE_MASK			(0x1 << EAWG_PAUSE_OFFSET)

#define EAWG_DISABLE_REG(ap)		(EAWG_BASE_REGS(ap) + 0x20)
#define EAWG_DISABLE_OFFSET		0
#define EAWG_DISABLE_MASK		(0x1 << EAWG_DISABLE_OFFSET)

#define EAWG_STATUS_REG(ap)		(EAWG_BASE_REGS(ap) + 0x24)
#define EAWG_TRANS_ERROR_OFFSET		4
#define EAWG_TRANS_ERROR_MASK		(0x1 << EAWG_TRANS_ERROR_OFFSET)
#define EAWG_WORKING_OFFSET		3
#define EAWG_WORKING_MASK		(0x1 << EAWG_WORKING_OFFSET)

#define EAWG_FIFO_STATUS_REG(ap)	EAWG_STATUS_REG(ap)
#define EAWG_FIFO_CTRL_OFFSET		2
#define EAWG_FIFO_CTRL_MASK		(0x1 << EAWG_FIFO_CTRL_OFFSET)
#define EAWG_FIFO_DATA_OFFSET		1
#define EAWG_FIFO_DATA_MASK		(0x1 << EAWG_FIFO_DATA_OFFSET)
#define EAWG_FIFO_ADDR_OFFSET		0
#define EAWG_FIFO_ADDR_MASK		(0x1 << EAWG_FIFO_ADDR_OFFSET)

#define TIMEOUT				100

/* disable EAWG */
void disable_eawg(int ap_num)
{
	uint32_t disable_eawg_reg = mmio_read_32(EAWG_DISABLE_REG(ap_num));

	/*  Set bit 1 to 1 to disable eawg */
	disable_eawg_reg |= (0x1 << EAWG_DISABLE_OFFSET);
	mmio_write_32(EAWG_DISABLE_REG(ap_num), disable_eawg_reg);
}

/* clear done interrupt bit in EAWG status register */
static void clear_done_interrupt(int ap_num)
{
	uint32_t done_interrupt_reg = mmio_read_32(EAWG_INTERRUPT_CONFIG_REG(ap_num));

	/* Set bit 1 to 1 to clear the done interrupt */
	done_interrupt_reg |= (0x1 << EAWG_CLEAR_DONE_OFFSET);
	mmio_write_32(EAWG_INTERRUPT_CONFIG_REG(ap_num), done_interrupt_reg);
	return;
}

/* clear fail interrupt bit in EAWG status register */
static void clear_fail_interrupt(int ap_num)
{
	uint32_t fail_interrupt_reg = mmio_read_32(EAWG_INTERRUPT_CONFIG_REG(ap_num));
	/* Set bit 5 to 1 to clear the fail interrupt */
	fail_interrupt_reg |= (0x1 << EAWG_CLEAR_FAIL_OFFSET);
	mmio_write_32(EAWG_INTERRUPT_CONFIG_REG(ap_num), fail_interrupt_reg);
	return;
}

/* waits for the EAWG to finish interacting with the AWG */
int eawg_check_is_done(uintptr_t scratch_pad_reg, int ap_num)
{
	uint32_t eawg_done;
	uint32_t fail_interrupt;
	uint32_t timeout = TIMEOUT;

	/* as long as the eawg is active and there's no failure interrupt keep waiting */
	do {
		eawg_done = mmio_read_32(scratch_pad_reg);
		fail_interrupt = ((mmio_read_32(EAWG_STATUS_REG(ap_num)) &
					EAWG_TRANS_ERROR_MASK) >> EAWG_TRANS_ERROR_OFFSET);
	}  while (!eawg_done && (!fail_interrupt) && (timeout--));

	if (!timeout) {
		ERROR("timeout error while sending EAWG's transaction in AP%d\n", ap_num);
		return -1;
	} else if (fail_interrupt) {
		ERROR("fail interrupt while sending EAWG's transaction in AP%d\n", ap_num);
		clear_fail_interrupt(ap_num);
		return -1;

	}
	clear_done_interrupt(ap_num);
	return 0;
}

static int eawg_addr_fifo_full(int ap_num)
{
	return (mmio_read_32(EAWG_FIFO_STATUS_REG(ap_num)) & EAWG_FIFO_ADDR_MASK) >> EAWG_FIFO_ADDR_OFFSET;
}

static int eawg_data_fifo_full(int ap_num)
{
	return (mmio_read_32(EAWG_FIFO_STATUS_REG(ap_num)) & EAWG_FIFO_DATA_MASK) >> EAWG_FIFO_DATA_OFFSET;
}

static int eawg_ctrl_fifo_full(int ap_num)
{
	return (mmio_read_32(EAWG_FIFO_STATUS_REG(ap_num)) & EAWG_FIFO_CTRL_MASK) >> EAWG_FIFO_CTRL_OFFSET;
}

/* write transactions to the entry of EAWG's FIFO */
static int eawg_add_transaction(struct eawg_transaction input, int ap_num)
{
	mmio_write_32(EAWG_WRITE_ADDR_REG(ap_num), input.address);
	mmio_write_32(EAWG_WRITE_DATA_REG(ap_num), input.data);
	mmio_write_32(EAWG_WRITE_DELAY_REG(ap_num), input.delay);

	return 0;
}


/* Loading transactions to the FIFO needs to be done till
 * all the transactions are written.
 * in case the transactions number is bigger than the FIFO's size
 * writing to the FIFO is done by steps
 * return values: number of transactions left to write.
 */
int eawg_load_transactions(struct eawg_transaction *conf_input, int transactions_num, int ap_num)
{
	int fifo_is_full;

	while (transactions_num) {
		fifo_is_full = eawg_addr_fifo_full(ap_num)
			       || eawg_data_fifo_full(ap_num) || eawg_ctrl_fifo_full(ap_num);

		/* Incase fifo is full exit writing transactions loop */
		if (fifo_is_full) {
			printf("EAWG's FIFO in AP%d is full\n", ap_num);
			break;
		} else {
			eawg_add_transaction(*conf_input, ap_num);
			conf_input++;
			transactions_num--;
		}
	}

	/* Loop ended or stopped means either all transactions were written
	 * or the FIFO is full.
	 */
	if (fifo_is_full) {
		if (!transactions_num)
			return 0;
		else
			return transactions_num;
	}
	return 0;
}

/* start the EAWG */
void eawg_start(int ap_num)
{
	/* setting  bit 0 to 1 in start register launches EAWG */
	mmio_write_32(EAWG_START_BASE_REG(ap_num), 0x1);

	/* after CPU0 (primary cpu) in AP0 triggers its EAWG, the cpu enters WFE */
	if (!ap_num)
		__asm__("wfe");

	/* delay of 1 ms to ensure stability in the frequencies*/
	mdelay(1);
}
