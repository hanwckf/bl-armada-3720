/*
 * Copyright (C) 2016 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include "comphy.h"
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <mvebu_def.h>
#include "pci_ep.h"

#define SD_ADDR(base, lane)			(base + 0x1000 * lane)
#define HPIPE_ADDR(base, lane)			(SD_ADDR(base, lane) + 0x800)
#define COMPHY_ADDR(base, lane)			(base + 0x28 * lane)


void reg_set(uintptr_t addr, uint32_t data, uint32_t mask)
{
	uint32_t reg_data;
	reg_data = mmio_read_32(addr);
	reg_data &= ~mask;
	reg_data |= data;
	mmio_write_32(addr, reg_data);
}

uint32_t polling_with_timeout(uintptr_t addr, uint32_t val, uint32_t mask, unsigned long usec_timout)
{
	uint32_t data;

	do {
		udelay(1);
		data = mmio_read_32(addr) & mask;
	} while (data != val  && --usec_timout > 0);

	if (usec_timout == 0)
		return data;
	return 0;
}

void comphy_mux_set_pcie(uintptr_t comphy_addr, int lane)
{
	uint32_t val, mask;

	/* Set the PIPE as PCIe Gen3 */
	mask  = 0xF << (4 * lane);
	val   = 0x4 << (4 * lane);
	reg_set(comphy_addr + COMMON_SELECTOR_PIPE_OFFSET, val, mask);

	/* Disconnect the Ethernet PHYs */
	mask  = 0xF << (4 * lane);
	val   = 0;
	reg_set(comphy_addr + COMMON_SELECTOR_PHY_OFFSET, val, mask);
}

int comphy_pcie_power_up(uint32_t lane, struct pci_hw_cfg *hw)
{
	uint32_t mask, data, ret = 1;
	uintptr_t hpipe_addr = HPIPE_ADDR(hw->hpipe_base, lane);
	uintptr_t comphy_addr = COMPHY_ADDR(hw->comphy_base, lane);
	uintptr_t addr;

	printf("Setting up Comphy lane %d as PCIe\n", lane);

	INFO("PCIe clock = %x\n", hw->clk_out);
	INFO("PCIe RC    = %d\n", !hw->is_end_point);
	INFO("PCIe Width = %d\n", hw->lane_width);

	comphy_mux_set_pcie(comphy_addr, lane);

	/* enable PCIe by4 and by2 */
	if (lane == 0) {
		if (hw->lane_width == 4) {
			reg_set(hw->comphy_base + COMMON_PHY_SD_CTRL1,
				0x1 << COMMON_PHY_SD_CTRL1_PCIE_X4_EN_OFFSET,
				COMMON_PHY_SD_CTRL1_PCIE_X4_EN_MASK);
		} else if (hw->lane_width == 2) {
			reg_set(hw->comphy_base + COMMON_PHY_SD_CTRL1,
				0x1 << COMMON_PHY_SD_CTRL1_PCIE_X2_EN_OFFSET,
				COMMON_PHY_SD_CTRL1_PCIE_X2_EN_MASK);
		}
	}

	/* if PCIe clock is output and clock source from SerDes lane 5,
	 * need to configure the clock-source MUX.
	 * By default, the clock source is from lane 4
	 */
	if (hw->clk_out && hw->clk_src && (lane == 5))
		reg_set(hw->dfx_base + DFX_DEV_GEN_CTRL12,
			0x3 << DFX_DEV_GEN_PCIE_CLK_SRC_OFFSET,
			DFX_DEV_GEN_PCIE_CLK_SRC_MASK);

	INFO("stage: RFU configurations - hard reset comphy\n");
	/* RFU configurations - hard reset comphy */
	mask = COMMON_PHY_CFG1_PWR_UP_MASK;
	data = 0x1 << COMMON_PHY_CFG1_PWR_UP_OFFSET;
	mask |= COMMON_PHY_CFG1_PIPE_SELECT_MASK;
	data |= 0x1 << COMMON_PHY_CFG1_PIPE_SELECT_OFFSET;
	mask |= COMMON_PHY_CFG1_PWR_ON_RESET_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_PWR_ON_RESET_OFFSET;
	mask |= COMMON_PHY_CFG1_CORE_RSTN_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_CORE_RSTN_OFFSET;
	mask |= COMMON_PHY_PHY_MODE_MASK;
	data |= 0x0 << COMMON_PHY_PHY_MODE_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG1_REG, data, mask);

	/* release from hard reset */
	mask = COMMON_PHY_CFG1_PWR_ON_RESET_MASK;
	data = 0x1 << COMMON_PHY_CFG1_PWR_ON_RESET_OFFSET;
	mask |= COMMON_PHY_CFG1_CORE_RSTN_MASK;
	data |= 0x1 << COMMON_PHY_CFG1_CORE_RSTN_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG1_REG, data, mask);

	/* Wait 1ms - until band gap and ref clock ready */
	mdelay(1);
	/* Start comphy Configuration */
	INFO("stage: Comphy configuration\n");
	/* Set PIPE soft reset */
	mask = HPIPE_RST_CLK_CTRL_PIPE_RST_MASK;
	data = 0x1 << HPIPE_RST_CLK_CTRL_PIPE_RST_OFFSET;
	/* Set PHY datapath width mode for V0 */
	mask |= HPIPE_RST_CLK_CTRL_FIXED_PCLK_MASK;
	data |= 0x1 << HPIPE_RST_CLK_CTRL_FIXED_PCLK_OFFSET;
	/* Set Data bus width USB mode for V0 */
	mask |= HPIPE_RST_CLK_CTRL_PIPE_WIDTH_MASK;
	data |= 0x0 << HPIPE_RST_CLK_CTRL_PIPE_WIDTH_OFFSET;
	/* Set CORE_CLK output frequency for 250Mhz */
	mask |= HPIPE_RST_CLK_CTRL_CORE_FREQ_SEL_MASK;
	data |= 0x0 << HPIPE_RST_CLK_CTRL_CORE_FREQ_SEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_RST_CLK_CTRL_REG, data, mask);
	/* Set PLL ready delay for 0x2 */
	data = 0x2 << HPIPE_CLK_SRC_LO_PLL_RDY_DL_OFFSET;
	mask = HPIPE_CLK_SRC_LO_PLL_RDY_DL_MASK;
	if (hw->lane_width != 1) {
		data |= 0x1 << HPIPE_CLK_SRC_LO_BUNDLE_PERIOD_SEL_OFFSET;
		mask |= HPIPE_CLK_SRC_LO_BUNDLE_PERIOD_SEL_MASK;
		data |= 0x1 << HPIPE_CLK_SRC_LO_BUNDLE_PERIOD_SCALE_OFFSET;
		mask |= HPIPE_CLK_SRC_LO_BUNDLE_PERIOD_SCALE_MASK;
	}
	reg_set(hpipe_addr + HPIPE_CLK_SRC_LO_REG, data, mask);

	/* Set PIPE mode interface to PCIe3 - 0x1  & set lane order */
	data = 0x1 << HPIPE_CLK_SRC_HI_MODE_PIPE_OFFSET;
	mask = HPIPE_CLK_SRC_HI_MODE_PIPE_MASK;
	if (hw->lane_width != 1) {
		mask |= HPIPE_CLK_SRC_HI_LANE_STRT_MASK;
		mask |= HPIPE_CLK_SRC_HI_LANE_MASTER_MASK;
		mask |= HPIPE_CLK_SRC_HI_LANE_BREAK_MASK;
		if (lane == 0) {
			data |= 0x1 << HPIPE_CLK_SRC_HI_LANE_STRT_OFFSET;
			data |= 0x1 << HPIPE_CLK_SRC_HI_LANE_MASTER_OFFSET;
		} else if (lane == (hw->lane_width - 1)) {
			data |= 0x1 << HPIPE_CLK_SRC_HI_LANE_BREAK_OFFSET;
		}
	}
	reg_set(hpipe_addr + HPIPE_CLK_SRC_HI_REG, data, mask);
	/* Config update polarity equalization */
	reg_set(hpipe_addr + HPIPE_LANE_EQ_CFG1_REG,
		0x1 << HPIPE_CFG_UPDATE_POLARITY_OFFSET, HPIPE_CFG_UPDATE_POLARITY_MASK);
	/* Set PIPE version 4 to mode enable */
	reg_set(hpipe_addr + HPIPE_DFE_CTRL_28_REG,
		0x1 << HPIPE_DFE_CTRL_28_PIPE4_OFFSET, HPIPE_DFE_CTRL_28_PIPE4_MASK);
	/* TODO: check if pcie clock is output/input - for bringup use input */
	/* Enable PIN clock 100M_125M */
	mask = 0;
	data = 0;
	/* Only if clock is output, configure the clock-source mux */
	if (hw->clk_out) {
		mask |= HPIPE_MISC_CLK100M_125M_MASK;
		data |= 0x1 << HPIPE_MISC_CLK100M_125M_OFFSET;
	}
	/* Set PIN_TXDCLK_2X Clock Frequency Selection for outputs 500MHz clock */
	mask |= HPIPE_MISC_TXDCLK_2X_MASK;
	data |= 0x0 << HPIPE_MISC_TXDCLK_2X_OFFSET;
	/* Enable 500MHz Clock */
	mask |= HPIPE_MISC_CLK500_EN_MASK;
	data |= 0x1 << HPIPE_MISC_CLK500_EN_OFFSET;
	if (hw->clk_out) { /* output */
		/* Set reference clock comes from group 1 */
		mask |= HPIPE_MISC_REFCLK_SEL_MASK;
		data |= 0x0 << HPIPE_MISC_REFCLK_SEL_OFFSET;
	} else {
		/* Set reference clock comes from group 2 */
		mask |= HPIPE_MISC_REFCLK_SEL_MASK;
		data |= 0x1 << HPIPE_MISC_REFCLK_SEL_OFFSET;
	}
	mask |= HPIPE_MISC_ICP_FORCE_MASK;
	data |= 0x1 << HPIPE_MISC_ICP_FORCE_OFFSET;
	reg_set(hpipe_addr + HPIPE_MISC_REG, data, mask);
	if (hw->clk_out) { /* output */
		/* Set reference frequcency select - 0x2 for 25MHz */
		mask = HPIPE_PWR_PLL_REF_FREQ_MASK;
		data = 0x2 << HPIPE_PWR_PLL_REF_FREQ_OFFSET;
	} else {
		/* Set reference frequcency select - 0x0 for 100MHz */
		mask = HPIPE_PWR_PLL_REF_FREQ_MASK;
		data = 0x0 << HPIPE_PWR_PLL_REF_FREQ_OFFSET;
	}
	/* Set PHY mode to PCIe */
	mask |= HPIPE_PWR_PLL_PHY_MODE_MASK;
	data |= 0x3 << HPIPE_PWR_PLL_PHY_MODE_OFFSET;
	reg_set(hpipe_addr + HPIPE_PWR_PLL_REG, data, mask);

	/* ref clock alignment */
	if (hw->lane_width != 1) {
		mask = HPIPE_LANE_ALIGN_OFF_MASK;
		data = 0x0 << HPIPE_LANE_ALIGN_OFF_OFFSET;
		reg_set(hpipe_addr + HPIPE_LANE_ALIGN_REG, data, mask);
	}

	/* Set the amount of time spent in the LoZ state -
	 * set for 0x7 only if the PCIe clock is output
	 */
	if (hw->clk_out)
		reg_set(hpipe_addr + HPIPE_GLOBAL_PM_CTRL,
			0x7 << HPIPE_GLOBAL_PM_RXDLOZ_WAIT_OFFSET,
			HPIPE_GLOBAL_PM_RXDLOZ_WAIT_MASK);

	/* Set Maximal PHY Generation Setting(8Gbps) */
	mask = HPIPE_INTERFACE_GEN_MAX_MASK;
	data = 0x2 << HPIPE_INTERFACE_GEN_MAX_OFFSET;
	/* Bypass frame detection and sync detection for RX DATA */
	mask = HPIPE_INTERFACE_DET_BYPASS_MASK;
	data = 0x1 << HPIPE_INTERFACE_DET_BYPASS_OFFSET;
	/* Set Link Train Mode (Tx training control pins are used) */
	mask |= HPIPE_INTERFACE_LINK_TRAIN_MASK;
	data |= 0x1 << HPIPE_INTERFACE_LINK_TRAIN_OFFSET;
	reg_set(hpipe_addr + HPIPE_INTERFACE_REG, data, mask);

	/* Set Idle_sync enable */
	mask = HPIPE_PCIE_IDLE_SYNC_MASK;
	data = 0x1 << HPIPE_PCIE_IDLE_SYNC_OFFSET;
	/* Select bits for PCIE Gen3(32bit) */
	mask |= HPIPE_PCIE_SEL_BITS_MASK;
	data |= 0x2 << HPIPE_PCIE_SEL_BITS_OFFSET;
	reg_set(hpipe_addr + HPIPE_PCIE_REG0, data, mask);

	/* Enable Tx_adapt_g1 */
	mask = HPIPE_TX_TRAIN_CTRL_G1_MASK;
	data = 0x1 << HPIPE_TX_TRAIN_CTRL_G1_OFFSET;
	/* Enable Tx_adapt_gn1 */
	mask |= HPIPE_TX_TRAIN_CTRL_GN1_MASK;
	data |= 0x1 << HPIPE_TX_TRAIN_CTRL_GN1_OFFSET;
	/* Disable Tx_adapt_g0 */
	mask |= HPIPE_TX_TRAIN_CTRL_G0_MASK;
	data |= 0x0 << HPIPE_TX_TRAIN_CTRL_G0_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_CTRL_REG, data, mask);

	/* Set reg_tx_train_chk_init */
	mask = HPIPE_TX_TRAIN_CHK_INIT_MASK;
	data = 0x0 << HPIPE_TX_TRAIN_CHK_INIT_OFFSET;
	/* Enable TX_COE_FM_PIN_PCIE3_EN */
	mask |= HPIPE_TX_TRAIN_COE_FM_PIN_PCIE3_MASK;
	data |= 0x1 << HPIPE_TX_TRAIN_COE_FM_PIN_PCIE3_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_REG, data, mask);

	INFO("stage: TRx training parameters\n");
	/* Set Preset sweep configurations */
	mask = HPIPE_TX_TX_STATUS_CHECK_MODE_MASK;
	data = 0x1 << HPIPE_TX_STATUS_CHECK_MODE_OFFSET;

	mask |= HPIPE_TX_NUM_OF_PRESET_MASK;
	data |= 0x7 << HPIPE_TX_NUM_OF_PRESET_OFFSET;

	mask |= HPIPE_TX_SWEEP_PRESET_EN_MASK;
	data |= 0x1 << HPIPE_TX_SWEEP_PRESET_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_CTRL_11_REG, data, mask);

	/* Tx train start configuration */
	mask = HPIPE_TX_TRAIN_START_SQ_EN_MASK;
	data = 0x1 << HPIPE_TX_TRAIN_START_SQ_EN_OFFSET;

	mask |= HPIPE_TX_TRAIN_START_FRM_DET_EN_MASK;
	data |= 0x0 << HPIPE_TX_TRAIN_START_FRM_DET_EN_OFFSET;

	mask |= HPIPE_TX_TRAIN_START_FRM_LOCK_EN_MASK;
	data |= 0x0 << HPIPE_TX_TRAIN_START_FRM_LOCK_EN_OFFSET;

	mask |= HPIPE_TX_TRAIN_WAIT_TIME_EN_MASK;
	data |= 0x1 << HPIPE_TX_TRAIN_WAIT_TIME_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_CTRL_5_REG, data, mask);

	/* Enable Tx train P2P */
	mask = HPIPE_TX_TRAIN_P2P_HOLD_MASK;
	data = 0x1 << HPIPE_TX_TRAIN_P2P_HOLD_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_CTRL_0_REG, data, mask);

	/* Configure Tx train timeout */
	mask = HPIPE_TRX_TRAIN_TIMER_MASK;
	data = 0x17 << HPIPE_TRX_TRAIN_TIMER_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_CTRL_4_REG, data, mask);

	/* Disable G0/G1/GN1 adaptation */
	mask = HPIPE_TX_TRAIN_CTRL_G1_MASK | HPIPE_TX_TRAIN_CTRL_GN1_MASK
		| HPIPE_TX_TRAIN_CTRL_G0_OFFSET;
	data = 0;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_CTRL_REG, data, mask);

	/* Disable DTL frequency loop */
	mask = HPIPE_PWR_CTR_DTL_FLOOP_EN_MASK;
	data = 0x0 << HPIPE_PWR_CTR_DTL_FLOOP_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_PWR_CTR_DTL_REG, data, mask);

	/* Configure G3 DFE */
	mask = HPIPE_G3_DFE_RES_MASK;
	data = 0x3 << HPIPE_G3_DFE_RES_OFFSET;
	reg_set(hpipe_addr + HPIPE_G3_SETTING_4_REG, data, mask);

	/* Use TX/RX training result for DFE */
	mask = HPIPE_DFE_RES_FORCE_MASK;
	data = 0x0 << HPIPE_DFE_RES_FORCE_OFFSET;
	reg_set(hpipe_addr + HPIPE_DFE_REG0,  data, mask);

	/* Configure initial and final coefficient value for receiver */
	mask = HPIPE_G3_RX_SELMUPI_MASK;
	data = 0x1 << HPIPE_G3_RX_SELMUPI_OFFSET;

	mask |= HPIPE_G3_RX_SELMUPF_MASK;
	data |= 0x1 << HPIPE_G3_RX_SELMUPF_OFFSET;

	mask |= HPIPE_G3_SETTING_BIT_MASK;
	data |= 0x0 << HPIPE_G3_SETTING_BIT_OFFSET;
	reg_set(hpipe_addr + HPIPE_G3_SETTINGS_1_REG,  data, mask);

	/* Trigger sampler enable pulse */
	mask = HPIPE_SMAPLER_MASK;
	data = 0x1 << HPIPE_SMAPLER_OFFSET;
	reg_set(hpipe_addr + HPIPE_SAMPLER_N_PROC_CALIB_CTRL_REG, data, mask);
	udelay(5);
	reg_set(hpipe_addr + HPIPE_SAMPLER_N_PROC_CALIB_CTRL_REG, 0, mask);

	/* FFE resistor tuning for different bandwidth  */
	mask = HPIPE_G3_FFE_DEG_RES_LEVEL_MASK;
	data = 0x1 << HPIPE_G3_FFE_DEG_RES_LEVEL_OFFSET;
	mask |= HPIPE_G3_FFE_LOAD_RES_LEVEL_MASK;
	data |= 0x3 << HPIPE_G3_FFE_LOAD_RES_LEVEL_OFFSET;

#ifdef OCP_COMPHY_TUNE
	/* FFE-res tuning and cap select  */
	mask |= HPIPE_G3_FFE_CAP_SEL_MASK;
	data |= 0xf << HPIPE_G3_FFE_CAP_SEL_OFFSET;
	mask |= HPIPE_G3_FFE_CAP_SEL_MASK;
	data |= 0x4 << HPIPE_G3_FFE_RES_SEL_OFFSET;
	mask |= HPIPE_G3_FFE_SETTING_FORCE_MASK;
	data |= 0x1 << HPIPE_G3_FFE_SETTING_FORCE_OFFSET;
#endif
	reg_set(hpipe_addr + HPIPE_G3_SETTING_3_REG, data, mask);

#ifdef OCP_COMPHY_TUNE
	/* preset0,1 main cursor fine Tune*/
	mask = HPIPE_CFG_CURSOR_PRESET0_MASK;
	data = 0x23 << HPIPE_CFG_CURSOR_PRESET0_OFFSET;
	mask |= HPIPE_CFG_CURSOR_PRESET1_MASK;
	data |= 0x28 << HPIPE_CFG_CURSOR_PRESET1_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG0_REG, data, mask);

	/* preset2,3 main cursor fine Tune*/
	mask = HPIPE_CFG_CURSOR_PRESET2_MASK;
	data = 0x26 << HPIPE_CFG_CURSOR_PRESET2_OFFSET;
	mask |= HPIPE_CFG_CURSOR_PRESET3_MASK;
	data |= 0x2a << HPIPE_CFG_CURSOR_PRESET3_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG1_REG, data, mask);

	/* preset4,5 main cursor fine Tune*/
	mask = HPIPE_CFG_CURSOR_PRESET4_MASK;
	data = 0x30 << HPIPE_CFG_CURSOR_PRESET4_OFFSET;
	mask |= HPIPE_CFG_CURSOR_PRESET5_MASK;
	data |= 0x2c << HPIPE_CFG_CURSOR_PRESET5_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG2_REG, data, mask);

	/* preset6,7 main cursor fine Tune*/
	mask = HPIPE_CFG_CURSOR_PRESET6_MASK;
	data = 0x2a << HPIPE_CFG_CURSOR_PRESET6_OFFSET;
	mask |= HPIPE_CFG_CURSOR_PRESET7_MASK;
	data |= 0x22 << HPIPE_CFG_CURSOR_PRESET7_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG3_REG, data, mask);

	/* preset8,9 main cursor fine Tune*/
	mask = HPIPE_CFG_CURSOR_PRESET8_MASK;
	data = 0x24 << HPIPE_CFG_CURSOR_PRESET8_OFFSET;
	mask |= HPIPE_CFG_CURSOR_PRESET9_MASK;
	data |= 0x28 << HPIPE_CFG_CURSOR_PRESET9_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG4_REG, data, mask);

	/* preset10,11 main cursor fine Tune*/
	mask = HPIPE_CFG_CURSOR_PRESET10_MASK;
	data = 0x21 << HPIPE_CFG_CURSOR_PRESET10_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG5_REG, data, mask);

	/* preset0 pre and post cursor fine Tune*/
	mask = HPIPE_CFG_PRE_CURSOR_PRESET0_MASK;
	data = 0x0 << HPIPE_CFG_PRE_CURSOR_PRESET0_OFFSET;
	mask |= HPIPE_CFG_POST_CURSOR_PRESET0_MASK;
	data |= 0xd << HPIPE_CFG_POST_CURSOR_PRESET0_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG6_REG, data, mask);

	/* preset1 pre and post cursor fine Tune*/
	mask = HPIPE_CFG_PRE_CURSOR_PRESET1_MASK;
	data = 0x0 << HPIPE_CFG_PRE_CURSOR_PRESET1_OFFSET;
	mask |= HPIPE_CFG_POST_CURSOR_PRESET1_MASK;
	data |= 0x8 << HPIPE_CFG_POST_CURSOR_PRESET1_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG7_REG, data, mask);

	/* preset2 pre and post cursor fine Tune*/
	mask = HPIPE_CFG_PRE_CURSOR_PRESET2_MASK;
	data = 0x0 << HPIPE_CFG_PRE_CURSOR_PRESET2_OFFSET;
	mask |= HPIPE_CFG_POST_CURSOR_PRESET2_MASK;
	data |= 0xa << HPIPE_CFG_POST_CURSOR_PRESET2_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG8_REG, data, mask);

	/* preset3 pre and post cursor fine Tune*/
	mask = HPIPE_CFG_PRE_CURSOR_PRESET3_MASK;
	data = 0x0 << HPIPE_CFG_PRE_CURSOR_PRESET3_OFFSET;
	mask |= HPIPE_CFG_POST_CURSOR_PRESET3_MASK;
	data |= 0x6 << HPIPE_CFG_POST_CURSOR_PRESET3_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG9_REG, data, mask);

	/* preset4 pre and post cursor fine Tune*/
	mask = HPIPE_CFG_PRE_CURSOR_PRESET4_MASK;
	data = 0x0 << HPIPE_CFG_PRE_CURSOR_PRESET4_OFFSET;
	mask |= HPIPE_CFG_POST_CURSOR_PRESET4_MASK;
	data |= 0x0 << HPIPE_CFG_POST_CURSOR_PRESET4_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG10_REG, data, mask);

	/* preset5 pre and post cursor fine Tune*/
	mask = HPIPE_CFG_PRE_CURSOR_PRESET5_MASK;
	data = 0x4 << HPIPE_CFG_PRE_CURSOR_PRESET5_OFFSET;
	mask |= HPIPE_CFG_POST_CURSOR_PRESET5_MASK;
	data |= 0x0 << HPIPE_CFG_POST_CURSOR_PRESET5_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG11_REG, data, mask);

	/* preset6 pre and post cursor fine Tune*/
	mask = HPIPE_CFG_PRE_CURSOR_PRESET6_MASK;
	data = 0x6 << HPIPE_CFG_PRE_CURSOR_PRESET6_OFFSET;
	mask |= HPIPE_CFG_POST_CURSOR_PRESET6_MASK;
	data |= 0x0 << HPIPE_CFG_POST_CURSOR_PRESET6_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG12_REG, data, mask);

	/* preset7 pre and post cursor fine Tune*/
	mask = HPIPE_CFG_PRE_CURSOR_PRESET7_MASK;
	data = 0x4 << HPIPE_CFG_PRE_CURSOR_PRESET7_OFFSET;
	mask |= HPIPE_CFG_POST_CURSOR_PRESET7_MASK;
	data |= 0xa << HPIPE_CFG_POST_CURSOR_PRESET7_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG13_REG, data, mask);

	/* preset8 pre and post cursor fine Tune*/
	mask = HPIPE_CFG_PRE_CURSOR_PRESET8_MASK;
	data = 0x6 << HPIPE_CFG_PRE_CURSOR_PRESET8_OFFSET;
	mask |= HPIPE_CFG_POST_CURSOR_PRESET8_MASK;
	data |= 0x6 << HPIPE_CFG_POST_CURSOR_PRESET8_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG14_REG, data, mask);

	/* preset9 pre and post cursor fine Tune*/
	mask = HPIPE_CFG_PRE_CURSOR_PRESET9_MASK;
	data = 0x8 << HPIPE_CFG_PRE_CURSOR_PRESET9_OFFSET;
	mask |= HPIPE_CFG_POST_CURSOR_PRESET9_MASK;
	data |= 0x0 << HPIPE_CFG_POST_CURSOR_PRESET9_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG15_REG, data, mask);

	/* preset10 pre and post cursor fine Tune*/
	mask = HPIPE_CFG_PRE_CURSOR_PRESET10_MASK;
	data = 0x0 << HPIPE_CFG_PRE_CURSOR_PRESET10_OFFSET;
	mask |= HPIPE_CFG_POST_CURSOR_PRESET10_MASK;
	data |= 0xf << HPIPE_CFG_POST_CURSOR_PRESET10_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_PRESET_CFG16_REG, data, mask);
#endif /* OCP_COMPHY_TUNE */

	/* Pattern lock lost timeout disable */
	mask = HPIPE_FRAME_DET_LOCK_LOST_TO_MASK;
	data = 0x0 << HPIPE_FRAME_DET_LOCK_LOST_TO_OFFSET;
	reg_set(hpipe_addr + HPIPE_FRAME_DET_CONTROL_REG, data, mask);

	/* Configure DFE adaptations */
	mask = HPIPE_CDR_RX_MAX_DFE_ADAPT_0_MASK;
	data = 0x0 << HPIPE_CDR_RX_MAX_DFE_ADAPT_0_OFFSET;
	mask |= HPIPE_CDR_RX_MAX_DFE_ADAPT_1_MASK;
	data |= 0x0 << HPIPE_CDR_RX_MAX_DFE_ADAPT_1_OFFSET;
	mask |= HPIPE_CDR_MAX_DFE_ADAPT_0_MASK;
	data |= 0x0 << HPIPE_CDR_MAX_DFE_ADAPT_0_OFFSET;
	mask |= HPIPE_CDR_MAX_DFE_ADAPT_1_MASK;
	data |= 0x1 << HPIPE_CDR_MAX_DFE_ADAPT_1_OFFSET;
	reg_set(hpipe_addr + HPIPE_CDR_CONTROL_REG, data, mask);

	mask = HPIPE_DFE_TX_MAX_DFE_ADAPT_MASK;
	data = 0x0 << HPIPE_DFE_TX_MAX_DFE_ADAPT_OFFSET;
	reg_set(hpipe_addr + HPIPE_DFE_CONTROL_REG, data, mask);

	/* Genration 2 setting 1*/
	mask = HPIPE_G2_SET_1_G2_RX_SELMUPI_MASK;
	data = 0x0 << HPIPE_G2_SET_1_G2_RX_SELMUPI_OFFSET;
	mask |= HPIPE_G2_SET_1_G2_RX_SELMUPP_MASK;
	data |= 0x1 << HPIPE_G2_SET_1_G2_RX_SELMUPP_OFFSET;
	mask |= HPIPE_G2_SET_1_G2_RX_SELMUFI_MASK;
	data |= 0x0 << HPIPE_G2_SET_1_G2_RX_SELMUFI_OFFSET;
	reg_set(hpipe_addr + HPIPE_G2_SET_1_REG, data, mask);

	/* DFE enable */
	mask = HPIPE_G2_DFE_RES_MASK;
	data = 0x3 << HPIPE_G2_DFE_RES_OFFSET;
	reg_set(hpipe_addr + HPIPE_G2_SETTINGS_4_REG, data, mask);

	/* Configure DFE Resolution */
	mask = HPIPE_LANE_CFG4_DFE_EN_SEL_MASK;
	data = 0x1 << HPIPE_LANE_CFG4_DFE_EN_SEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_CFG4_REG, data, mask);

	/* VDD calibration control */
	mask = HPIPE_EXT_SELLV_RXSAMPL_MASK;
	data = 0x16 << HPIPE_EXT_SELLV_RXSAMPL_OFFSET;
	reg_set(hpipe_addr + HPIPE_VDD_CAL_CTRL_REG, data, mask);

	/* Set PLL Charge-pump Current Control */
	mask = HPIPE_G3_SETTING_5_G3_ICP_MASK;
	data = 0x4 << HPIPE_G3_SETTING_5_G3_ICP_OFFSET;
	reg_set(hpipe_addr + HPIPE_G3_SETTING_5_REG, data, mask);

	/* Set lane rqualization remote setting */
	mask = HPIPE_LANE_CFG_FOM_DIRN_OVERRIDE_MASK;
	data = 0x1 << HPIPE_LANE_CFG_FOM_DIRN_OVERRIDE_OFFSET;
	mask |= HPIPE_LANE_CFG_FOM_ONLY_MODE_MASK;
	data |= 0x1 << HPIPE_LANE_CFG_FOM_ONLY_MODE_OFFFSET;
	mask |= HPIPE_LANE_CFG_FOM_PRESET_VECTOR_MASK;
	data |= 0x6 << HPIPE_LANE_CFG_FOM_PRESET_VECTOR_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_EQ_REMOTE_SETTING_REG, data, mask);

	mask = HPIPE_CFG_EQ_BUNDLE_DIS_MASK;
	data = 0x1 << HPIPE_CFG_EQ_BUNDLE_DIS_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_EQ_CFG2_REG, data, mask);

	if (!hw->is_end_point) {
		/* Set phy in root complex mode */
		mask = HPIPE_CFG_PHY_RC_EP_MASK;
		data = 0x1 << HPIPE_CFG_PHY_RC_EP_OFFSET;
		reg_set(hpipe_addr + HPIPE_LANE_EQU_CONFIG_0_REG, data, mask);
	}

#ifdef OCP_COMPHY_TUNE
	mask = HPIPE_CFG_EQ_FS_MASK;
	data = 0x30 << HPIPE_CFG_EQ_FS_OFFSET;
	mask |= HPIPE_CFG_EQ_LF_MASK;
	data |= 0x10 << HPIPE_CFG_EQ_LF_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_EQU_CONFIG_0_REG, data, mask);
#endif /* OCP_COMPHY_TUNE */

	INFO("stage: Comphy power up\n");
	/* for PCIe by4 or by2 - release from reset only after finish to configure all lanes */
	if ((hw->lane_width == 1) || (lane == (hw->lane_width - 1))) {
		uint32_t i, start_lane, end_lane;

		if (hw->lane_width != 1) {
			/* allows writing to all lanes in one write */
			reg_set(hw->comphy_base + COMMON_PHY_SD_CTRL1,
				0x0 << COMMON_PHY_SD_CTRL1_COMPHY_0_4_PORT_OFFSET,
				COMMON_PHY_SD_CTRL1_COMPHY_0_4_PORT_MASK);
			start_lane = 0;
			end_lane = hw->lane_width;

			/* Release from PIPE soft reset for PCIe by4 or by2 -
			 * release from soft reset all lanes - can't use
			 * read modify write
			 */
			reg_set(HPIPE_ADDR(hw->hpipe_base, 0) + HPIPE_RST_CLK_CTRL_REG,
				0x24, 0xffffffff);
		} else {
			start_lane = lane;
			end_lane = lane + 1;

			/* Release from PIPE soft reset for PCIe by4 or by2 - 
			 * release from soft reset all lanes
			 */
			reg_set(hpipe_addr + HPIPE_RST_CLK_CTRL_REG,
				0x0 << HPIPE_RST_CLK_CTRL_PIPE_RST_OFFSET,
				HPIPE_RST_CLK_CTRL_PIPE_RST_MASK);
		}


		if (hw->lane_width != 1) {
			/* disable writing to all lanes with one write */
			reg_set(hw->comphy_base + COMMON_PHY_SD_CTRL1,
				0x3210 << COMMON_PHY_SD_CTRL1_COMPHY_0_4_PORT_OFFSET,
				COMMON_PHY_SD_CTRL1_COMPHY_0_4_PORT_MASK);
		}

		INFO("stage: Check PLL\n");
		/* Read lane status */
		for (i = start_lane; i < end_lane; i++) {
			addr = HPIPE_ADDR(hw->hpipe_base, i) + HPIPE_LANE_STATUS1_REG;
			data = HPIPE_LANE_STATUS1_PCLK_EN_MASK;
			mask = data;
			data = polling_with_timeout(addr, data, mask, 15000);
			if (data != 0) {
				INFO("Read from reg = 0x%lx - value = 0x%x\n",
				     hpipe_addr + HPIPE_LANE_STATUS1_REG, data);
				ERROR("HPIPE_LANE_STATUS1_PCLK_EN_MASK is 0\n");
				ret = 0;
			}
		}
	}

	return ret;
}
