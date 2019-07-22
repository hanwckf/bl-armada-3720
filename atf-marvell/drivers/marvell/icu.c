/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <icu.h>
#include <mmio.h>
#include <mvebu_def.h>

#define ICU_SET_SPI_AL(x)	(0x10 + (0x10 * x))
#define ICU_SET_SPI_AH(x)	(0x14 + (0x10 * x))
#define ICU_CLR_SPI_AL(x)	(0x18 + (0x10 * x))
#define ICU_CLR_SPI_AH(x)	(0x1c + (0x10 * x))
#define ICU_INT_CFG(x)		(0x100 + 4 * x)

#define ICU_INT_ENABLE_OFFSET	(24)
#define ICU_IS_EDGE_OFFSET	(28)
#define ICU_GROUP_OFFSET	(29)

enum icu_group {
	ICU_GRP_SEI	= 4,
	ICU_GRP_REI	= 5,
	ICU_GRP_MAX,
};

struct icu_msi {
	enum icu_group group;
	uintptr_t set_spi_addr;
	uintptr_t clr_spi_addr;
};

#define MAX_ICU_IRQS		207

/* Allocate the MSI address per interrupt group,
 * unsopprted groups get NULL address
 */
static struct icu_msi msi_addr[] = {
	{ICU_GRP_SEI,  0xf03f0230, 0xf03f0230},	/* System error interrupts */
	{ICU_GRP_REI,  0xf03f0270, 0xf03f0270},	/* RAM error interrupts */
};

static void icu_clear_irq(uintptr_t icu_base, int nr)
{
	mmio_write_32(icu_base + ICU_INT_CFG(nr), 0);
}

static void icu_set_irq(uintptr_t icu_base, const struct icu_irq *irq,
			enum icu_group group)
{
	uint32_t icu_int;

	icu_int  = (irq->spi_id) | (1 << ICU_INT_ENABLE_OFFSET);
	icu_int |= irq->is_edge << ICU_IS_EDGE_OFFSET;
	icu_int |= group << ICU_GROUP_OFFSET;

	mmio_write_32(icu_base + ICU_INT_CFG(irq->icu_id), icu_int);
}

/* This function initialize the ICU (only SEI and REI) */
void icu_init(uintptr_t cp_base, const struct icu_config *config)
{
	int i;
	const struct icu_irq *irq;
	struct icu_msi *msi;
	uintptr_t icu_base = cp_base + MVEBU_ICU_REG_OFFSET;

	/* Set the addres for SET_SPI and CLR_SPI registers in AP */
	msi = msi_addr;
	for (i = ICU_GRP_SEI; i < ICU_GRP_MAX; i++, msi++) {
		mmio_write_32(icu_base + ICU_SET_SPI_AL(msi->group), msi->set_spi_addr & 0xFFFFFFFF);
		mmio_write_32(icu_base + ICU_SET_SPI_AH(msi->group), msi->set_spi_addr >> 32);
		mmio_write_32(icu_base + ICU_CLR_SPI_AL(msi->group), msi->clr_spi_addr & 0xFFFFFFFF);
		mmio_write_32(icu_base + ICU_CLR_SPI_AH(msi->group), msi->clr_spi_addr >> 32);
	}

	/* Mask all ICU interrupts */
	for (i = 0; i < MAX_ICU_IRQS; i++)
		icu_clear_irq(icu_base, i);

	/* Configure the ICU interrupt lines for REI and SEI*/
	irq = config->sei.map;
	for (i = 0; i < config->sei.size; i++, irq++)
		icu_set_irq(icu_base, irq, ICU_GRP_SEI);

	irq = config->rei.map;
	for (i = 0; i < config->rei.size; i++, irq++)
		icu_set_irq(icu_base, irq, ICU_GRP_REI);

	return;
}
