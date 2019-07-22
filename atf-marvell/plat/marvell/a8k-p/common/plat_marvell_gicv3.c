/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <ap810_setup.h>
#include <debug.h>
#include <mmio.h>
#include <mvebu.h>
#include <platform_def.h>

/****************************************************************************
 * GICv3 Multi chip initialization
 ****************************************************************************
 */
#define GICD_BASE(chip)			(MVEBU_REGS_BASE_AP(chip) + \
					MVEBU_GICD_BASE)
#define GICD_CHIPSR(chip)		(GICD_BASE(chip) + 0xC000)
#define GICD_CHIPSR_RTS_OFFSET		4
#define GICD_CHIPSR_RTS_MASK		0x3
#define GICD_DCHIPR(chip)		(GICD_BASE(chip) + 0xC004)
#define GICD_DCHIPR_PUP_OFFSET		0
#define GICD_CHIPR(chip)		(MVEBU_REGS_BASE_AP(0) + \
					MVEBU_GICD_BASE + 0xC008 + (chip) * 0x8)
#define GICD_CHIPR_PUP_OFFSET		1
#define GICD_CHIPR_SPI_BLOCKS_OFFSET	5
#define GICD_CHIPR_SPI_BLOCKS_MASK	0x1F
#define GICD_CHIPR_SPI_BLOCK_MIN_OFFSET	10
#define GICD_CHIPR_SPI_BLOCK_MIN_MASK	0x3F
#define GICD_CHIPR_ADDR_OFFSET		16
#define GICD_CHIPR_ADDR_MASK		0xFFFFFFFF

#define RTS_CONSISTENT			2

#define GICD_CHIPR_CONFIG_VAL(enable, spi_block_min, spi_blocks, addr)  \
	((enable) |							\
	(((spi_blocks) & GICD_CHIPR_SPI_BLOCKS_MASK) << \
		GICD_CHIPR_SPI_BLOCKS_OFFSET) | \
	(((spi_block_min) & GICD_CHIPR_SPI_BLOCK_MIN_MASK) << \
		GICD_CHIPR_SPI_BLOCK_MIN_OFFSET) | \
	(((addr) & GICD_CHIPR_ADDR_MASK) << GICD_CHIPR_ADDR_OFFSET))


/* In AP810 include 640 SPIs, 32-671 SPI, and it
 * split to SPI block, every block include 32 SPIs
 * The split depends on chip amount, if we have single
 * chip, configure all 640 SPIs to AP0.
 * If the system include more than single chip, split
 * the SPIs on the chip amount (640/4) = 160.
 */
struct spi_ownership {
	int spi_start;
	int spi_end;
};

struct spi_ownership ap_spi_own[PLAT_MARVELL_NORTHB_COUNT] = {
	{32, 351},
	{352, 667},
	{668, 689},
	{670, 671},
};

int gic_multi_chip_connection_ready(int ap_id)
{
	int timeout = 200;
	int pup_in_progress;
	int write_in_progress;

	debug_enter();

	do {
		pup_in_progress = mmio_read_32(GICD_DCHIPR(ap_id)) &
					       (1 << GICD_DCHIPR_PUP_OFFSET);
		write_in_progress = mmio_read_64(GICD_CHIPR(ap_id)) &
						 (1 << GICD_CHIPR_PUP_OFFSET);
	} while ((pup_in_progress || write_in_progress) && (timeout-- > 0));

	if (pup_in_progress) {
		ERROR("AP %d: GICv3: PUP is not ready\n", ap_id);
		return 1;
	}

	if (write_in_progress) {
		ERROR("AP %d: GICv3: Write was not accepted\n", ap_id);
		return 1;
	}

#if 0 /* TODO: enable this once CHIPSR reflects the right value */
	if (ap_id == 0) {
		/* Check that the Routing Table status is 'Consistent' */
		int rts_status = (mmio_read_32(GICD_CHIPSR(0)) >>
					       GICD_CHIPSR_RTS_OFFSET) &
					       GICD_CHIPSR_RTS_MASK;
		INFO("GICD_CHIPSR: 0x%x\n", mmio_read_32(GICD_CHIPSR(0)));
		if (rts_status != RTS_CONSISTENT) {
			INFO("Routing table status (%d) is not consistent\n",
			     rts_status);
			return 1;
		}
	}
#endif

	debug_exit();
	return 0;
}

int gic600_multi_chip_init(void)
{
	unsigned int nb_id;
	uint32_t reg;
	uint64_t val;

	/* TODO: add EERATA to UART read */
	INFO("read UART USR to clear the UART interrupt\n");
	mmio_read_64(MVEBU_AP_UART_BASE(0) + 0x7c);

	debug_enter();

	INFO("GIC-600 Multi Chip initialization\n");
	/* Go over APs and configure the routing table */
	for (nb_id = 0; nb_id < ap_get_count(); nb_id++) {
		unsigned int spi_block_min, spi_blocks;

		INFO("Configure AP %d\n", nb_id);
		spi_block_min = (ap_spi_own[nb_id].spi_start - 32) / 32;
		spi_blocks = (ap_spi_own[nb_id].spi_end -
			     ap_spi_own[nb_id].spi_start + 1) / 32;
		INFO("spi_block_min = %d - spi_blocks = %d\n",
		     spi_block_min, spi_blocks);


		reg = GICD_CHIPR_CONFIG_VAL(0x1, spi_block_min,
					    spi_blocks, nb_id);
		mmio_write_64(GICD_CHIPR(nb_id), reg);
		val = mmio_read_64(GICD_CHIPR(nb_id));

		while (((val >> 1) & 0x1) == 1)
			val = mmio_read_64(GICD_CHIPR(nb_id));

		INFO("AP%d: GICD_CHIPR(nb_id)=%x GICD_CHIPR: 0x%lx val=%x\n",
		     nb_id, GICD_CHIPR(nb_id), mmio_read_64(GICD_CHIPR(nb_id)),
		     reg);

		if (nb_id == 0) {
			INFO("GICD_CHIPSR = %x\n",
			     mmio_read_32(GICD_CHIPSR(nb_id)));
			INFO("GICD_CHIPR = %x\n",
			     mmio_read_32(GICD_CHIPR(nb_id)));
		} else {
			/* check that write was accepted
			 * and connection is ready
			 */
			if (gic_multi_chip_connection_ready(nb_id))
				return 1;
		}
	}

	debug_exit();
	return 0;
}
