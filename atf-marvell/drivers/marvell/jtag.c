/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <delay_timer.h>
#include <mmio.h>
#include <mvebu_def.h>

#define MPP_CTRL_REG	0xEC6f4000
#define GPIO_DATA_EN	0xEC6f5044
#define GPIO_DATA_IN	0xEC6f5050
#define GPIO_DATA_OUT	0xEC6f5040
#define DFX_PREFIX_AP	0x06800000
#define DFX_PREFIX_CP	0x01800000
#define DFX_PREFIX_REG	0x000f8018
#define DFX_DUMMY_REG	0x000f8248

enum JTAG_SIGNALS {
	JT_TCK = 0,
	JT_TMS = 1,
	JT_TDI = 2,
	JT_TDO = 3,
	JT_TRST = 4
};

/* set jtag pins */
static void jtag_set_pin(int pin, int b)
{
	unsigned int reg;
	reg = mmio_read_32(MVEBU_AP_GPIO_REGS(0));
	mmio_write_32(MVEBU_AP_GPIO_REGS(0), (reg & ~(1 << pin)) | (b << pin));
}

static int get_tdo(void)
{
	return (mmio_read_32(MVEBU_AP_GPIO_DATA_IN(0)) >> JT_TDO) & 0x1;
}

/* tdi: input of serial data
 * tdo: output of serial data
 * tms: input of state machine
 */
static int jtag_clk(int tdi, int tms)
{
	int tdo;

	/* Update TDI and TMS at the negative edge of TCK
	 * (Sampled at the positive edge).
	 */
	jtag_set_pin(JT_TDI, tdi);
	jtag_set_pin(JT_TMS, tms);
	jtag_set_pin(JT_TCK, 0);

	udelay(15);

	/* Sample TDO at the positive edge of TCK */
	tdo = get_tdo();
	jtag_set_pin(JT_TCK, 1);

	udelay(15);

	return tdo;
}

static void jtag_scan_chain(void)
{
	unsigned int IRDataOut[4] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
	unsigned int i, idx, offset;

	/* Select DR chain.
	 * Select IR chain.
	 * capture IR.
	 * shitft IR.
	 */
	jtag_clk(0, 1);
	jtag_clk(0, 1);
	jtag_clk(0, 0);
	jtag_clk(0, 0);

	/* Stop JR */
	for (i = 0; i < 96; i++) {
		idx = i / 32;
		offset = i % 32;
		IRDataOut[idx] &= ~(0x1 << offset);
		IRDataOut[idx] |= (jtag_clk(0x1, (i + 1 == 96))) << offset;
	}

	/* update IR.
	 * IDLE.
	 */
	jtag_clk(0, 1);
	jtag_clk(0, 0);

	mdelay(2);
}

static void jtag_close(void)
{
	unsigned int reg;

	/* Make MPPs 0-6 inputs
	 * MVEBU_AP_GPIO_DATA_OUT_EN(0)
	 */
	reg = mmio_read_32(MVEBU_AP_GPIO_DATA_OUT_EN(0));
	mmio_write_32(MVEBU_AP_GPIO_DATA_OUT_EN(0), (reg | 0x77));

	/* Reproduce value of MPP controls */
	mmio_write_32(MVEBU_AP_MPP_REGS(0, 0), 0x22000000);
}

static void jtag_ir(unsigned int bit_cnt, unsigned int *pIR)
{
	int i;
	unsigned int idx, offset;

	/* select DR chain.
	 * select IR chain.
	 * capture IR.
	 * shift IR.
	 */
	jtag_clk(0, 1);
	jtag_clk(0, 1);
	jtag_clk(0, 0);
	jtag_clk(0, 0);

	/* Stop IR */
	for (i = 0; i < bit_cnt; i++) {
		idx = i / 32;
		offset = i % 32;
		jtag_clk((pIR[idx] >> offset) & 0x1, (i + 1 == bit_cnt));
	}
	/* update IR.
	 * IDLE.
	 */
	jtag_clk(0, 1);
	jtag_clk(0, 0);

	udelay(15);
}

/* jtag state machine: needed for accessing jtag DR */
static void jtag_dr(unsigned int bit_cnt, unsigned int *pDataIn, unsigned int *pDataOut)
{
	int i;
	unsigned int idx, offset;

	jtag_clk(0, 1); /* select DR chain */
	jtag_clk(0, 0); /* capture DR */
	jtag_clk(0, 0); /* shift DR */

	/* Stop DR */
	for (i = 0; i < bit_cnt; i++) {
		idx = i / 32;
		offset = i % 32;
		pDataOut[idx] &= ~(0x1 << offset);
		pDataOut[idx] |= (jtag_clk((pDataIn[idx] >> offset) & 0x1, (i + 1 == bit_cnt))) << offset;
	}

	jtag_clk(0, 1); /* update DR */
	jtag_clk(0, 0); /* IDLE */

	udelay(15);
}


static unsigned int jtag_reg_access(unsigned int address,
				    unsigned int dataIn,
				    int bIsWrite,
				    unsigned int IRLength,
				    unsigned int IRTapOffset,
				    unsigned int DRLength,
				    unsigned int DROffset,
				    int bIsMtap)
{
	/* Assuming less than 128 bits */
	unsigned int IRData[4] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
	unsigned int DRDataIn[4] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
	unsigned int DRDataOut[4] = {0xBAADBAAD, 0xBAADBAAD, 0xBAADBAAD, 0xBAADBAAD};
	unsigned int dataOut = 0;
	int i;
	unsigned int idx, offset, data_bit;

	/* Init IR */
	if (bIsMtap) {
		/* MTAP bypass is 10001 = 0x11*/
		for (i = IRLength - 4; i < IRLength - 1; i++)
			IRData[i / 32] &= ~(1 << (i % 32));
	}

	/* NTAP REGACCESS is 01011 */
	IRData[(IRTapOffset + 2) / 32] &= ~(1 << ((IRTapOffset + 2) % 32));
	IRData[(IRTapOffset + 4) / 32] &= ~(1 << ((IRTapOffset + 4) % 32));

	/* Init DR
	 * At the DR TAP offset:
	 * Write:   MSB -->  '1'+ '32 bit value' + '1' + '32 bit data'
	 * Read:    MSB -->  '0'+ '32 bit value' + '0' + '32 bit data'
	 */
	for (i = 0; i < 32; i++) {
		idx = (i + DROffset) / 32;
		offset = (i + DROffset) % 32;
		data_bit = (address >> i) & 0x1;
		if (!data_bit)
			DRDataIn[idx] &= ~(1 << offset);
	}

	for (i = 0; i < 32; i++) {
		idx = (i + 33 + DROffset) / 32;
		offset = (i + 33 + DROffset) % 32;
		data_bit = (dataIn >> i) & 0x1;
		if (!data_bit)
			DRDataIn[idx] &= ~(1 << offset);
	}

	if (!bIsWrite) {
		DRDataIn[(DROffset + 32) / 32] &= ~(1 << ((DROffset + 32) % 32));
		DRDataIn[(DROffset + 65) / 32] &= ~(1 << ((DROffset + 65) % 32));
	}

	jtag_ir(IRLength, IRData);
	jtag_dr(DRLength, DRDataIn, DRDataOut);

	for (i = 0; i < 32; i++) {
		idx = (i + DROffset) / 32;
		offset = (i + DROffset) % 32;
		data_bit = (DRDataOut[idx] >> offset) & 0x1;
		dataOut |= (data_bit << i);
	}

	return dataOut;
}

static void set_prefix_ap1(unsigned int addr)
{
	unsigned int prefix = 0;
	unsigned int new_prefix = (addr & 0xFFC00000) >> 22;
	unsigned int prefixReg = DFX_PREFIX_AP | new_prefix << 10 | new_prefix;

	if ((prefix != new_prefix) && (new_prefix != DFX_PREFIX_AP >> 22))  {
		prefix = new_prefix;
		jtag_reg_access(DFX_PREFIX_AP | DFX_PREFIX_REG, prefixReg, 1, 18, 2, 71, 2, 1);
	}
}

static void set_prefix_ap0(unsigned int addr)
{
	unsigned int prefix = 0;
	unsigned int new_prefix = (addr & 0xFFC00000) >> 22;
	unsigned int prefixReg = DFX_PREFIX_AP | new_prefix << 10 | new_prefix;

	if ((prefix != new_prefix) && (new_prefix != DFX_PREFIX_AP >> 22)) {
		prefix = new_prefix;
		jtag_reg_access(DFX_PREFIX_AP | DFX_PREFIX_REG, prefixReg, 1, 18, 8, 71, 4, 1);
	}
}

static unsigned int dfx_read_ap1(unsigned int addr)
{
	/* The chain is MTAP-NTAP-MSIB-NTAP-MSIB-MSIB */
	jtag_reg_access(addr, 0, 0, 18, 2, 71, 2, 1);
	return jtag_reg_access(DFX_PREFIX_AP | DFX_DUMMY_REG, 0x00001234, 1, 18, 2, 71, 2, 1);
}

static unsigned int dfx_read_ap0(unsigned int addr)
{
	jtag_reg_access(addr, 0, 0, 18, 8, 71, 4, 1);
	return jtag_reg_access(DFX_PREFIX_AP | DFX_DUMMY_REG, 0x00001234, 1, 18, 8, 71, 4, 1);
}

static void dfx_write_ap0(unsigned int addr, unsigned int data)
{
	/* The chain is MTAP5-NTAP5-MSIB1-NTAP5-MSIB1-MSIB1 */
	set_prefix_ap0(addr);
	jtag_reg_access(addr, data, 1, 18, 8, 71, 4, 1);
}

static void dfx_write_ap1(unsigned int addr, unsigned int data)
{
	/* The chain is MTAP-NTAP-MSIB-NTAP-MSIB-MSIB */
	set_prefix_ap1(addr);
	jtag_reg_access(addr, data, 1, 18, 2, 71, 2, 1);
}

static void jtag_trst(void)
{
	int i;

	/* 10 cycles of TMS=1 (and then 10 cycles of idle) */
	for (i = 0; i < 10; i++)
		jtag_clk(0, 1);

	for (i = 0; i < 10; i++)
		jtag_clk(0, 0);
}


static void jtag_init(void)
{
	unsigned int reg;

	/* make MPP0-MPP6 GPIO */
	reg = mmio_read_32(MVEBU_AP_MPP_REGS(0, 0));
	mmio_write_32(MVEBU_AP_MPP_REGS(0, 0), reg & 0xF0000000);
	/* Make MPP0, MPP1, MPP2, MPP4, MPP5, MPP6 output.
	 * init values of MPP5 = 1 and the rest to 0
	 */
	reg = mmio_read_32(MVEBU_AP_GPIO_REGS(0));
	mmio_write_32(MVEBU_AP_GPIO_REGS(0), reg & 0xffffffa8);

	reg = mmio_read_32(MVEBU_AP_GPIO_DATA_OUT_EN(0));
	mmio_write_32(MVEBU_AP_GPIO_DATA_OUT_EN(0), (reg & 0xffffff88) | 0x8);

	/* MPP0-MPP4 are jtag pins
	 * MPP5-6 control the eternal mux.
	 * MPP5 = 0, MPP6 = 1 - XDB controls JTAG
	 * MPP5 = 1, MPP6 = 0 - MPPs control JTAG
	 */
	reg = mmio_read_32(MVEBU_AP_GPIO_REGS(0));
	mmio_write_32(MVEBU_AP_GPIO_REGS(0), (reg | (1 << JT_TRST) | (1 << 5)));

	jtag_set_pin(JT_TCK, 1);
	jtag_set_pin(JT_TMS, 1);
	jtag_set_pin(JT_TDI, 0);

	mdelay(2);

	jtag_trst();

	jtag_scan_chain();

	dfx_write_ap0(DFX_PREFIX_AP | DFX_PREFIX_REG, DFX_PREFIX_AP);
	dfx_write_ap1(DFX_PREFIX_AP | DFX_PREFIX_REG, DFX_PREFIX_AP);
}

int jtag_init_ihb_dual_ap(void)
{
	unsigned int rd_data, new_data, sr_phy0, sr_phy1;

	jtag_init();

	/* Turn off C2C clk at AP0 side */
	dfx_write_ap0(0xEC6f4358, 0x07FFBD7F);

	/* Turn off C2C clk at AP1 side */
	dfx_write_ap1(0xEC6f4358, 0x07FFBD7F);

	/* Turn off retransmit AP0 */
	dfx_write_ap0(0xEC6C6004, 0x2);

	/* Read 0 */
	dfx_read_ap0(0xEC6C601C);

	/* Turn off retransmit AP1 */
	dfx_write_ap1(0xEC6C0004, 0x2);

	/* Read 0 */
	dfx_read_ap1(0xEC6C001C);

	/* Read link status */
	dfx_read_ap0(0xEC6C70b4);
	dfx_read_ap0(0xEC6C80b4);
	dfx_read_ap1(0xEC6C10b4);
	dfx_read_ap1(0xEC6C20b4);

	/* reset C2C phy (both sides) */
	dfx_write_ap0(0xEC6f4354, 0xFFFFAFFF);
	dfx_write_ap1(0xEC6f4354, 0xFFFFFFAF);

	/* Make AP0 host */
	rd_data = dfx_read_ap0(0xEC6F4290);
	new_data = rd_data | (0x1 << 12);
	dfx_write_ap0(0xEC6F4290, new_data);
	rd_data = dfx_read_ap0(0xEC6F42B0);
	new_data = rd_data | (0x1 << 12);
	dfx_write_ap0(0xEC6F42B0, new_data);

	/* unreset C2C phy */
	dfx_write_ap1(0xEC6f4354, 0xFFFFFFFF);
	dfx_write_ap0(0xEC6f4354, 0xFFFFFFFF);

	/* Wait until link is up */
	while (1) {
		sr_phy0 = dfx_read_ap0(0xEC6F428C);
		sr_phy1 = dfx_read_ap0(0xEC6F42AC);
		if ((sr_phy0 & 0x20) && (sr_phy1 & 0x20))
			break;
	}

	/* Read 0 */
	dfx_read_ap0(0xEC6C70b4);
	dfx_read_ap0(0xEC6C80b4);
	dfx_read_ap1(0xEC6C10b4);
	dfx_read_ap1(0xEC6C20b4);

	dfx_write_ap0(0xEC6f4358, 0x07FFBFFF);
	dfx_write_ap1(0xEC6f4358, 0x07FFBFFF);

	jtag_close();

	return 0;
}
