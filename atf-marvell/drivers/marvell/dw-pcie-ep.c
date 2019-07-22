/*
 * Copyright (C) 2016 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <mmio.h>
#include <mvebu_def.h>

#define PCIE_CFG_VENDOR			0x0
#define PCIE_CFG_DEVICE			0x2
#define PCIE_CFG_CMD			0x4
#define  PCIE_CFG_CMD_IO_EN		(1 << 0)
#define  PCIE_CFG_CMD_MEM_EN		(1 << 1)
#define  PCIE_CFG_CMD_MASTER_EN		(1 << 2)
#define PCIE_CFG_STATUS			0x6

#define PCIE_GLOBAL_CONTROL		0x8000
#define PCIE_CFG_RETRY_EN		(1 << 9)
#define PCIE_APP_LTSSM_EN		(1 << 2)
#define PCIE_DEVICE_TYPE_OFFSET		(4)
#define PCIE_DEVICE_TYPE_MASK		(0xF)
#define PCIE_DEVICE_TYPE_EP		(0x0) /* Endpoint */
#define PCIE_DEVICE_TYPE_LEP		(0x1) /* Legacy endpoint */
#define PCIE_DEVICE_TYPE_RC		(0x4) /* Root complex */

#define PCIE_LINK_CTL_2			0xA0
#define TARGET_LINK_SPEED_MASK		0xF
#define PCIE_LINK_CAPABILITY		0x7C

#define PCIE_GEN3_EQU_CTRL		0x8A8
#define PCIE_GEN3_EQ_FB_MODE_MASK	0xf
#define PCIE_GEN3_EQ_FB_MODE_OFFSET	0x0
#define GEN3_EQU_EVAL_2MS_DISABLE	(1 << 5)
#define GEN3_EQ_PSET_REQ_VEC_MASK	0xffff00
#define GEN3_EQ_PSET_REQ_VEC_OFFSET	8


#define PCIE_ARCACHE_TRC		0x8050
#define PCIE_AWCACHE_TRC		0x8054
#define ARCACHE_SHAREABLE_CACHEABLE	0x3511
#define AWCACHE_SHAREABLE_CACHEABLE	0x5311

#define LINK_SPEED_GEN_1                0x1
#define LINK_SPEED_GEN_2                0x2
#define LINK_SPEED_GEN_3                0x3

/* iATU registers */
#define PCIE_IATU_VIEWPORT		0x900
#define PCIE_IATU_CR1			0x904
#define PCIE_IATU_CR2			0x908
#define  PCIE_IATU_CR2_EN		(0x1 << 31)
#define PCIE_IATU_LOWER_BASE		0x90C
#define PCIE_IATU_UPPER_BASE		0x910
#define PCIE_IATU_LIMIT			0x914
#define PCIE_IATU_LOWER_TARGET		0x918
#define PCIE_IATU_UPPER_TARGET		0x91C

#define DW_OUTBOUND_ATU_BASE		0x8000000000ull
#define DW_OUTBOUND_ATU_SIZE		0x100000000ull
#define DW_OUTBOUND_ATU_TARGET		0x0

#define PCIE_MSIX_CAP_ID_NEXT_CTRL_REG		0xb0
#define PCIE_MSIX_CAP_NEXT_OFFSET_MASK		0xff00

#define PCIE_SPCIE_CAP_HEADER_REG		0x158
#define PCIE_SPCIE_NEXT_OFFSET_MASK		0xfff00000
#define PCIE_SPCIE_NEXT_OFFSET_OFFSET		20

#define PCIE_LANE_EQ_CTRL01_REG			0x164
#define PCIE_LANE_EQ_CTRL23_REG			0x168
#define PCIE_LANE_EQ_SETTING			0x55555555

#define PCIE_TPH_EXT_CAP_HDR_REG		0x1b8
#define PCIE_TPH_REQ_NEXT_PTR_MASK		0xfff00000
#define PCIE_TPH_REQ_NEXT_PTR_OFFSET		20

#define PCIE_LINK_FLUSH_CONTROL_OFF_REG		0x8cc
#define PCIE_AUTO_FLUSH_EN_MASK			0x1

#define PCIE_REQ_RESET				0x8058
#define PCIE_LINK_REQ_RST_MASK			0x2

void dw_pcie_configure(uintptr_t regs_base, uint32_t cap_speed)
{
	uint32_t reg;

	/*
	 * Set the correct hints for lane equalization.
	 *
	 * These registers consist of the following fields:
	 *	- Downstream Port Transmitter Preset - Used for equalization by
	 *	  this port when the Port is operating as a downstream Port.
	 *	- Downstream Port Receiver Preset Hint - May be used as a hint
	 *	  for receiver equalization by this port when the Port is
	 *	  operating as a downstream Port.
	 *	- Upstream Port Transmitter Preset - Field contains the
	 *	  transmit preset value sent or received during link
	 *	  equalization.
	 *	- Upstream Port Receiver Preset Hint - Field contains the
	 *	  receiver preset hint value sent or received during link
	 *	  equalization.
	 *
	 * The default values for this registers aren't optimal for our
	 * hardware, so we set the optimal values according to HW measurements.
	 */
	mmio_write_32(regs_base + PCIE_LANE_EQ_CTRL01_REG,
		      PCIE_LANE_EQ_SETTING);
	mmio_write_32(regs_base + PCIE_LANE_EQ_CTRL23_REG,
		      PCIE_LANE_EQ_SETTING);

	/*  Set link to GEN 3 */;
	reg  = mmio_read_32(regs_base + PCIE_LINK_CTL_2);
	reg &= ~TARGET_LINK_SPEED_MASK;
	reg |= cap_speed;
	mmio_write_32(regs_base + PCIE_LINK_CTL_2, reg);

	reg  = mmio_read_32(regs_base + PCIE_LINK_CAPABILITY);
	reg &= ~TARGET_LINK_SPEED_MASK;
	reg |= cap_speed;
	mmio_write_32(regs_base + PCIE_LINK_CAPABILITY, reg);

	reg = mmio_read_32(regs_base + PCIE_GEN3_EQU_CTRL);
	reg |= GEN3_EQU_EVAL_2MS_DISABLE;

	/*
	 * According to the electrical measuremnts, the best presets for our
	 * receiver are preset3 to preset8, so we are changing the vector of
	 * presets to evaluate during the link equalization training preset3-8.
	 */
	reg &= ~GEN3_EQ_PSET_REQ_VEC_MASK;
	reg |= 0x3f0 << GEN3_EQ_PSET_REQ_VEC_OFFSET;
	reg &= ~PCIE_GEN3_EQ_FB_MODE_MASK;
	reg |= 0x1 << PCIE_GEN3_EQ_FB_MODE_OFFSET;
	mmio_write_32(regs_base + PCIE_GEN3_EQU_CTRL, reg);

	/*
	 * There is an issue in CPN110 that does not allow to
	 * enable/disable the link and perform "hot reset" unless
	 * the auto flush is disabled. So in order to enable the option
	 * to perform hot reset and link disable/enable we need to set
	 * auto flush to disable.
	 */
	reg = mmio_read_32(regs_base + PCIE_LINK_FLUSH_CONTROL_OFF_REG);
	reg &= ~PCIE_AUTO_FLUSH_EN_MASK;
	mmio_write_32(regs_base + PCIE_LINK_FLUSH_CONTROL_OFF_REG, reg);

	/*
	 * When the port is configured as Endpoint,
	 * the hot reset and link disable/enable must
	 * penetrate and reset the MAC configurations and thus
	 * need to unmask the reset.
	 */
	reg = mmio_read_32(regs_base + PCIE_REQ_RESET);
	reg &= ~PCIE_LINK_REQ_RST_MASK;
	mmio_write_32(regs_base + PCIE_REQ_RESET, reg);

	/*
	 * Remove VPD capability from the capability list,
	 * since we don't support it.
	 */
	reg = mmio_read_32(regs_base + PCIE_MSIX_CAP_ID_NEXT_CTRL_REG);
	reg &= ~PCIE_MSIX_CAP_NEXT_OFFSET_MASK;
	mmio_write_32(regs_base + PCIE_MSIX_CAP_ID_NEXT_CTRL_REG, reg);

	/*
	 * The below two configurations are intended to remove SRIOV capability
	 * from the capability list, since we don't support it.
	 * The capability list is a linked list where each capability points
	 * to the next capability, so in the SRIOV capability need to set the previous
	 * capability to point to the next capability and this way
	 * the SRIOV capability will be skipped.
	 */
	reg = mmio_read_32(regs_base + PCIE_TPH_EXT_CAP_HDR_REG);
	reg &= ~PCIE_TPH_REQ_NEXT_PTR_MASK;
	reg |= 0x24c << PCIE_TPH_REQ_NEXT_PTR_OFFSET;
	mmio_write_32(regs_base + PCIE_TPH_EXT_CAP_HDR_REG, reg);

	reg = mmio_read_32(regs_base + PCIE_SPCIE_CAP_HEADER_REG);
	reg &= ~PCIE_SPCIE_NEXT_OFFSET_MASK;
	reg |= 0x1b8 << PCIE_SPCIE_NEXT_OFFSET_OFFSET;
	mmio_write_32(regs_base + PCIE_SPCIE_CAP_HEADER_REG, reg);
}

int dw_pcie_link_up(uintptr_t regs_base, uint32_t cap_speed, int is_end_point)
{
	uint32_t reg;

	/* Disable LTSSM state machine to enable configuration */
	reg = mmio_read_32(regs_base + PCIE_GLOBAL_CONTROL);
	reg &= ~(PCIE_APP_LTSSM_EN);
	reg &= ~(PCIE_DEVICE_TYPE_MASK << PCIE_DEVICE_TYPE_OFFSET);
	if (!is_end_point)
		reg |= (PCIE_DEVICE_TYPE_RC << PCIE_DEVICE_TYPE_OFFSET);
	mmio_write_32(regs_base + PCIE_GLOBAL_CONTROL, reg);

	/* Set the PCIe master AXI attributes */
	mmio_write_32(regs_base + PCIE_ARCACHE_TRC, ARCACHE_SHAREABLE_CACHEABLE);
	mmio_write_32(regs_base + PCIE_AWCACHE_TRC, AWCACHE_SHAREABLE_CACHEABLE);

	/* DW pre link configurations */
	dw_pcie_configure(regs_base, cap_speed);

	/* Configuration done. Start LTSSM */
	reg = mmio_read_32(regs_base + PCIE_GLOBAL_CONTROL);
	reg |= PCIE_APP_LTSSM_EN;
	mmio_write_32(regs_base + PCIE_GLOBAL_CONTROL, reg);

	/* As the end-point we dont need to check if a
	 * link was established*/

	return 1;
}

void dw_pcie_open_out_atu(uintptr_t dw_base, int win_id, uint64_t local_base,
			  uint64_t remote_base, uint64_t size)
{
	mmio_write_32(dw_base + PCIE_IATU_VIEWPORT, win_id);
	mmio_write_32(dw_base + PCIE_IATU_LOWER_BASE, local_base & UINT32_MAX);
	mmio_write_32(dw_base + PCIE_IATU_UPPER_BASE, local_base >> 32);
	mmio_write_32(dw_base + PCIE_IATU_LOWER_TARGET, remote_base & UINT32_MAX);
	mmio_write_32(dw_base + PCIE_IATU_UPPER_TARGET, remote_base >> 32);
	mmio_write_32(dw_base + PCIE_IATU_LIMIT, size - 1);
	mmio_write_32(dw_base + PCIE_IATU_CR2, PCIE_IATU_CR2_EN);
}

void dw_pcie_master_enable(uintptr_t dw_base)
{
	mmio_write_32(dw_base + PCIE_CFG_CMD, PCIE_CFG_CMD_MEM_EN |
					      PCIE_CFG_CMD_IO_EN |
					      PCIE_CFG_CMD_MASTER_EN);
}

void dw_pcie_delay_cfg(uintptr_t dw_base)
{
	uint32_t ctrl;
	ctrl = mmio_read_32(dw_base + PCIE_GLOBAL_CONTROL);
	ctrl |= PCIE_CFG_RETRY_EN;
	mmio_write_32(dw_base + PCIE_GLOBAL_CONTROL, ctrl);
}

void dw_pcie_ep_init(uintptr_t dw_base, uint8_t delay_cfg, uint8_t master_en)
{
	dw_pcie_link_up(dw_base, LINK_SPEED_GEN_3, 1);

	if (master_en) {
		dw_pcie_open_out_atu(dw_base, 0, DW_OUTBOUND_ATU_BASE,
				     DW_OUTBOUND_ATU_TARGET, DW_OUTBOUND_ATU_SIZE);
		dw_pcie_master_enable(dw_base);
	}

	if (delay_cfg)
		dw_pcie_delay_cfg(dw_base);

	return;
}
