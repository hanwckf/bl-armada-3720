/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <armada_common.h>
#include <ap810_setup.h>
#include <debug.h>
#include <delay_timer.h>
#include <mci.h>
#include <mmio.h>
#include <marvell_plat_priv.h>
#include <plat_marvell.h>

/* MCI related defines */
#define MVEBU_AP_SYSTEM_SOFT_RESET_REG(ap)	(MVEBU_AP_MISC_SOC_BASE(ap) + 0x54)
/* For every MCI have 2 reset parameters MAIN/PHY SW reset */
#define SOFT_RESET_IHBx_MAIN_SW_RESET(mci)	((0x1) << ((mci * 2) + 3))
#define SOFT_RESET_IHBx_PHY_SW_RESET(mci)	((0x1) << ((mci * 2) + 4))

/* MCIx_REG_START_ADDRESS */
#define MVEBU_MCI_REG_START_ADDRESS(ap, mci)	(MVEBU_AR_RFU_BASE(ap) + 0x4158 + mci * 0x4)
#define MVEBU_MCI_REG_START_ADDR_SHIFT		12

#define MVEBU_IHB_CNTRL_REG_1(ap, mci_idx)	(MVEBU_MCI_PHY(ap, mci_idx) + 0x4)
#define IHB_CNTRL_REG1_DEVICE_WDAT(val)		(((val) & 0xff) << 0)
#define IHB_CNTRL_REG1_DEVICE_ADDR_BYTE_SEL(val)(((val) & 0x3) << 16)
#define IHB_CNTRL_REG1_DEVICE_ADDR(val)		(((val) & 0xff) << 18)
#define IHB_CNTRL_REG1_DEVICE_REG_WEN(val)	(((val) & 0x1) << 28)
#define IHB_CNTRL_REG1_DEVICE_WEN_DONE(val)	(((val) & 0x1) << 29)

#define MVEBU_IHB_PWM_CTRL_REG3(ap, mci_idx)	(MVEBU_MCI_PHY(ap, mci_idx) + 0x1c)
#define IHB_PWM_CTRL_REG3_AUTO_SPEED_OFFSET	0
#define IHB_PWM_CTRL_REG3_AUTO_SPEED_MASK	(0xf << IHB_PWM_CTRL_REG3_AUTO_SPEED_OFFSET)

#define MCI_RETRY_COUNT				10

/* SYSRST_OUTn Config definitions */
#define MVEBU_SYSRST_OUT_CONFIG_REG(ap)		(MVEBU_AP_MISC_SOC_BASE(ap) + 0x4)
#define WD_MASK_SYS_RST_OUT			(1 << 2)

#define MVEBU_AP_WD_EN_REG(ap)			(MVEBU_AP_WD_BASE(ap))
#define MVEBU_AP_WD_TIMEOUT_REG(ap)		(MVEBU_AP_WD_BASE(ap) + 0x8)

#define WD_ENABLE				1
#define WD_DISABLE				0

static uint32_t mci_get_link_speed(int ap_idx, int mci_idx)
{
	return mmio_read_32(MVEBU_IHB_PWM_CTRL_REG3(ap_idx, mci_idx)) & IHB_PWM_CTRL_REG3_AUTO_SPEED_MASK;
}

static void mci_phy_config(int ap_idx, int mci_idx)
{
	mmio_write_32(MVEBU_IHB_CNTRL_REG_1(ap_idx, mci_idx),
				IHB_CNTRL_REG1_DEVICE_WDAT(0x50) |
				IHB_CNTRL_REG1_DEVICE_ADDR_BYTE_SEL(0x1) |
				IHB_CNTRL_REG1_DEVICE_ADDR(0x21) |
				IHB_CNTRL_REG1_DEVICE_REG_WEN(0x1));
	mdelay(5);

	mmio_write_32(MVEBU_IHB_CNTRL_REG_1(ap_idx, mci_idx),
				IHB_CNTRL_REG1_DEVICE_WDAT(0x50) |
				IHB_CNTRL_REG1_DEVICE_ADDR_BYTE_SEL(0x1) |
				IHB_CNTRL_REG1_DEVICE_ADDR(0x21) |
				IHB_CNTRL_REG1_DEVICE_REG_WEN(0x0));
	mdelay(5);
}

static void ap810_win_route_open(int ap_id, uint64_t base_addr, uint64_t addr_sz, uint32_t target_id)
{
	struct addr_map_win gwin_temp_win;
	struct addr_map_win ccu_temp_win;

	ccu_temp_win.base_addr = base_addr;
	ccu_temp_win.win_size = addr_sz;

	/* If this is a remote AP */
	if (ap_id > 0) {
		gwin_temp_win.base_addr = base_addr;
		gwin_temp_win.win_size  = addr_sz;
		gwin_temp_win.target_id = ap_id;
		gwin_temp_win_insert(0, &gwin_temp_win, 1);

		/* Open temp window for access to GWIN */
		ccu_temp_win.target_id = GLOBAL_TID;
		ccu_temp_win_insert(0, &ccu_temp_win, 1);
	}

	ccu_temp_win.target_id = target_id;
	ccu_temp_win_insert(ap_id, &ccu_temp_win, 1);
}

static void ap810_win_route_close(int ap_id, uint64_t base_addr, uint64_t addr_sz, uint32_t target_id)
{
	struct addr_map_win gwin_temp_win;
	struct addr_map_win ccu_temp_win;

	ccu_temp_win.base_addr = base_addr;
	ccu_temp_win.win_size  = addr_sz;
	ccu_temp_win.target_id = target_id;
	ccu_temp_win_remove(ap_id, &ccu_temp_win, 1);

	/* If this is a remote AP */
	if (ap_id > 0) {
		gwin_temp_win.base_addr = base_addr;
		gwin_temp_win.win_size  = addr_sz;
		gwin_temp_win.target_id = ap_id;
		gwin_temp_win_remove(0, &gwin_temp_win, 1);

		/* Open temp window for access to GWIN */
		ccu_temp_win.target_id = GLOBAL_TID;
		ccu_temp_win_remove(0, &ccu_temp_win, 1);
	}
}

static void ap810_mci_phy_soft_reset(int ap_id, int mci_idx)
{
	uint32_t reg;

	/* For every MCI, there is MAIN SW reset & PHY SW reset */
	reg = mmio_read_32(MVEBU_AP_SYSTEM_SOFT_RESET_REG(ap_id));
	reg &= ~(SOFT_RESET_IHBx_MAIN_SW_RESET(mci_idx) | SOFT_RESET_IHBx_PHY_SW_RESET(mci_idx));
	mmio_write_32(MVEBU_AP_SYSTEM_SOFT_RESET_REG(ap_id), reg);

	/* Wait 5ms before get into reset */
	mdelay(5);
	reg |= (SOFT_RESET_IHBx_MAIN_SW_RESET(mci_idx) | SOFT_RESET_IHBx_PHY_SW_RESET(mci_idx));
	mmio_write_32(MVEBU_AP_SYSTEM_SOFT_RESET_REG(ap_id), reg);
}

static void a8kp_mci_turn_off_links(uintptr_t mci_base)
{
	int ap_id, cp_id, mci_id;

	/* Go over the APs and turn off the link of MCIs */
	for (ap_id = 0; ap_id < ap_get_count(); ap_id++) {
		ap810_win_route_open(ap_id, mci_base, MVEBU_MCI_REG_SIZE_REMAP, IO_0_TID);

		/* Go over the MCIs  */
		for (cp_id = 0; cp_id < ap810_get_cp_per_ap_static_cnt(ap_id); cp_id++) {
			struct addr_map_win iowin_temp_win = {
				.base_addr = mci_base,
				.win_size = MVEBU_MCI_REG_SIZE_REMAP,
			};
			/* Get the MCI index */
			mci_id = marvell_get_mci_map(ap_id, cp_id);
			INFO("Turn link off for AP-%d MCI-%d\n", ap_id, mci_id);

			/* Open temp window IO_WIN unit with relevant target ID */
			iowin_temp_win.target_id = mci_id;
			iow_temp_win_insert(ap_id, &iowin_temp_win, 1);

			/* Open window for MCI indirect access from APx */
			mmio_write_32(MVEBU_MCI_REG_START_ADDRESS(ap_id, mci_id),
					mci_base >> MVEBU_MCI_REG_START_ADDR_SHIFT);
			/* Turn link off */
			mci_turn_link_down();
			/* Remove the temporary IO-WIN window */
			iow_temp_win_remove(ap_id, &iowin_temp_win, 1);
		}

		ap810_win_route_close(ap_id, mci_base, MVEBU_MCI_REG_SIZE_REMAP, IO_0_TID);
	}
}

static void a8kp_mci_mpp_reset(int mpp)
{
	uint32_t val;

	/* Set MPP to low */
	val = mmio_read_32(MVEBU_AP_GPIO_DATA_IN(0));
	val &= ~(0x1 << mpp);
	mmio_write_32(MVEBU_AP_GPIO_DATA_IN(0), val);

	/* Clear data out */
	val = mmio_read_32(MVEBU_AP_GPIO_DATA_OUT_VAL(0));
	val &= ~(0x1 << mpp);
	mmio_write_32(MVEBU_AP_GPIO_DATA_OUT_VAL(0), val);

	/* Enable data out */
	val = mmio_read_32(MVEBU_AP_GPIO_DATA_OUT_EN(0));
	val &= ~(0x1 << mpp);
	mmio_write_32(MVEBU_AP_GPIO_DATA_OUT_EN(0), val);

	INFO("Get out CPs from reset\n");
	val = mmio_read_32(MVEBU_AP_GPIO_DATA_OUT_VAL(0));
	val |= (0x1 << mpp);
	mmio_write_32(MVEBU_AP_GPIO_DATA_OUT_VAL(0), val);

	/* Wait until CP release from reset */
	mdelay(2);
}

/* MCI initialize for all APs, the sequence split to 3 parts:
 * 1. Turn off the link on all MCIs
 * 2. Reset the CPs via MPP
 * 3. Re-init the MCI phy in AP side & in CP side
 */
static int mci_wa_initialize(void)
{
	int ap_id, mci_id, cp_id;
	uintptr_t mci_base = MVEBU_MCI_REG_BASE_REMAP(0);

	debug_enter();

	/* 1st stage - Turn off the link on all MCIs */
	a8kp_mci_turn_off_links(mci_base);

	/* 2nd stage - reset CPs via MPP */
	a8kp_mci_mpp_reset(MPP_MCI_RELEASE_FROM_RESET);

	/* 3rd stage - Re-init the MCI phy in AP side & in CP side */
	for (ap_id = 0; ap_id < ap_get_count(); ap_id++) {
		ap810_win_route_open(ap_id, mci_base, MVEBU_MCI_REG_SIZE_REMAP, IO_0_TID);

		/* Go over the MCIs in every APx */
		for (cp_id = 0; cp_id < ap810_get_cp_per_ap_static_cnt(ap_id); cp_id++) {
			uint32_t reg;
			struct addr_map_win iowin_temp_win = {
				.base_addr = mci_base,
				.win_size = MVEBU_MCI_REG_SIZE_REMAP,
			};
			/* Get the MCI index */
			mci_id = marvell_get_mci_map(ap_id, cp_id);
			INFO("Turn link on & ID assign AP%d MCI-%d\n", ap_id, mci_id);

			/* Config MCI phy on AP side */
			mci_phy_config(ap_id, mci_id);

			/* Reset MCI phy on AP side */
			ap810_mci_phy_soft_reset(ap_id, mci_id);

			/* Open temp window IO_WIN unit with relevant target ID */
			iowin_temp_win.target_id = mci_id;
			iow_temp_win_insert(ap_id, &iowin_temp_win, 1);

			/* Open window for MCI indirect access from APx */
			mmio_write_32(MVEBU_MCI_REG_START_ADDRESS(ap_id, mci_id),
					mci_base >> MVEBU_MCI_REG_START_ADDR_SHIFT);

			/* Turn on link on CP side */
			mci_turn_link_on();

			/* Wait 20ms, until link is stable*/
			mdelay(20);

			/* Check the link status on CP side */
			reg = mci_get_link_status();
			if (reg == -1) {
				ERROR("bad link on MCI-%d - status register is %x\n", mci_id, reg);
				return -1;
			}

			reg = mci_get_link_speed(ap_id, mci_id);
			if (reg != 0x3) {
				ERROR("link speed is not correct on MCI-%d - link speed is %x\n", mci_id, reg);
				return -1;
			}

			INFO("MCI-%d link is 8G (speed = %x)\n", mci_id, reg);

			/* Remove the temporary IO-WIN window */
			iow_temp_win_remove(ap_id, &iowin_temp_win, 1);
		}

		ap810_win_route_close(ap_id, mci_base, MVEBU_MCI_REG_SIZE_REMAP, IO_0_TID);
	}

	debug_exit();
	return 0;
}

/* Armada-8k-plus have bug on MCI, that link between AP & CP is
 * not stable and should be re-initialize.
 */
void a8kp_mci_wa_initialize(void)
{
	int retry = MCI_RETRY_COUNT;

	/* Retry until success, if the MCI initialization failed, reset is required */
	while (retry > 0) {
		if (mci_wa_initialize() == 0)
			break;
		ERROR("MCIx failed to create link - retry again %d of %d\n", retry, MCI_RETRY_COUNT);
		retry--;
	}

	if (retry == 0) {
		ERROR("MCIx failed to create link after %d times, reset is required\n", MCI_RETRY_COUNT);
		panic();
	}
}

/* Configure the threshold of every MCI */
static int a8kp_mci_configure_threshold(void)
{
	int ap_id, mci_id, cp_id;
	uintptr_t mci_base = MVEBU_MCI_REG_BASE_REMAP(0);

	debug_enter();

	/* Run MCI WA for performance improvements */
	for (ap_id = 0; ap_id < ap_get_count(); ap_id++) {
		ap810_win_route_open(ap_id, mci_base, MVEBU_MCI_REG_SIZE_REMAP, IO_0_TID);

		/* Go over the MCIs in every APx */
		for (cp_id = 0; cp_id < ap810_get_cp_per_ap_cnt(ap_id); cp_id++) {
			struct addr_map_win iowin_temp_win = {
				.base_addr = mci_base,
				.win_size = MVEBU_MCI_REG_SIZE_REMAP,
			};
			/* Get the MCI index */
			mci_id = marvell_get_mci_map(ap_id, cp_id);
			INFO("Configure threshold & ID assin AP%d MCI-%d\n", ap_id, mci_id);

			/* Open temp window IO_WIN unit with relevant target ID */
			iowin_temp_win.target_id = mci_id;
			iow_temp_win_insert(ap_id, &iowin_temp_win, 1);

			/* Open window for MCI indirect access from APx */
			mmio_write_32(MVEBU_MCI_REG_START_ADDRESS(ap_id, mci_id),
					mci_base >> MVEBU_MCI_REG_START_ADDR_SHIFT);

			/* Run MCI WA for performance improvements */
			mci_initialize(mci_id);

			/* Remove the temporary IO-WIN window */
			iow_temp_win_remove(ap_id, &iowin_temp_win, 1);
		}

		ap810_win_route_close(ap_id, mci_base, MVEBU_MCI_REG_SIZE_REMAP, IO_0_TID);
	}

	debug_exit();
	return 0;
}

static void ap810_configure_watchdog(int enable)
{
	debug_enter();

	/* As workaround for MCI failures, need to enable the watchdog before
	** first access to CPx, this WA needed for AP810-A0 only
	** */
	if (ap810_rev_id_get(MVEBU_AP0) == MVEBU_AP810_REV_ID_A0) {
		if (enable)
			mmio_write_32(MVEBU_AP_WD_TIMEOUT_REG(MVEBU_AP0), 0x1000000);
		mmio_write_32(MVEBU_AP_WD_EN_REG(MVEBU_AP0), enable);
	}

	debug_exit();
}

/* CP110 has configuration space address set by the default to 0xf200_0000
 * In Armada-8k-plus family there is an option to connect more than
 * a single CP110 to AP810.
 * Need to update the configuration space according to the address map of
 * Armada-8k-plus family.
 * This function opens a temporary windows in GWIN/CCU/IO-WIN to access different
 * CPs, changes the configuration space of every CP & closes the temporary windows
 */
static void update_cp110_default_win(void)
{
	int ap_id, cp_id, mci_id;
	uintptr_t cp110_base, cp110_temp_base;

	debug_enter();

	/* CP110 default configuration address space */
	cp110_temp_base = MVEBU_CP_DEFAULT_BASE_ADDR;

	/* Go over the APs and update every CP with
	 * the new configuration address
	 */
	for (ap_id = 0; ap_id < ap_get_count(); ap_id++) {
		ap810_win_route_open(ap_id, cp110_temp_base, MVEBU_CP_DEFAULT_BASE_SIZE, IO_0_TID);

		/* Go over the connected CPx in the APx */
		for (cp_id = 0; cp_id < ap810_get_cp_per_ap_cnt(ap_id); cp_id++) {
			struct addr_map_win iowin_temp_win = {
				.base_addr = cp110_temp_base,
				.win_size = MVEBU_CP_DEFAULT_BASE_SIZE,
			};
			/* Get the MCI index */
			mci_id = marvell_get_mci_map(ap_id, cp_id);
			INFO("AP-%d MCI-%d CP-%d\n", ap_id, mci_id, cp_id);

			/* Enable watchdog before access to CP registers */
			ap810_configure_watchdog(WD_ENABLE);

			/* Open temp window in IO_WIN unit with relevant target ID */
			iowin_temp_win.target_id = mci_id;
			iow_temp_win_insert(ap_id, &iowin_temp_win, 1);

			/* Calculate the new CP110 - base address */
			cp110_base = MVEBU_CP_REGS_BASE(ap_id, cp_id);
			/* Go and update the CP110 configuration address space */
			iob_cfg_space_update(ap_id, cp_id, cp110_temp_base, cp110_base);

			/* Remove the temporary IO-WIN window */
			iow_temp_win_remove(ap_id, &iowin_temp_win, 1);
			/* Disable watchdog timer, that mean all the access to
			** the CP works good
			** */
			ap810_configure_watchdog(WD_DISABLE);
		}

		ap810_win_route_close(ap_id, cp110_temp_base, MVEBU_CP_DEFAULT_BASE_SIZE, IO_0_TID);
	}

	debug_exit();
}

static void ap810_addr_decode_init(void)
{
	int ap_id;

	debug_enter();

	for (ap_id = 0; ap_id < ap_get_count(); ap_id++) {
		INFO("Initialize address decode for AP-%d\n", ap_id);
		/* configure IO-WIN windows */
		init_io_win(ap_id);
		/* configure GWIN windows */
		init_gwin(ap_id);
		/* configure CCU windows */
		init_ccu(ap_id);
	}

	debug_exit();
}

void marvell_bl1_setup_mpps(void)
{
	/* Enable UART MPPs.
	 ** In a normal system, this is done by Bootrom.
	 */
	mmio_write_32(MVEBU_AP_MPP_REGS(0, 1), 0x3000);
	mmio_write_32(MVEBU_AP_MPP_REGS(0, 2), 0x30000);
}

static void ap810_soc_misc_configurations(void)
{
	uint32_t reg, ap;

	debug_enter();

	for (ap = 0; ap < ap_get_count(); ap++) {
		/* Un-mask Watchdog reset from influencing the SYSRST_OUTn.
		 * Otherwise, upon WD timeout, the WD reset singal won't trigger reset
		 */
		reg = mmio_read_32(MVEBU_SYSRST_OUT_CONFIG_REG(ap));
		reg &= ~(WD_MASK_SYS_RST_OUT);
		mmio_write_32(MVEBU_SYSRST_OUT_CONFIG_REG(ap), reg);
	}

	debug_exit();
}

void bl1_plat_arch_setup(void)
{
	marvell_bl1_plat_arch_setup();

	plat_delay_timer_init();

#if PALLADIUM
	/* Palladium does not run BLE so ap810_enumeration_algo()
	 * isn't executed - need to make sure that we init the
	 * enumeration algorithm.
	 */
	ap810_enumeration_algo();
#endif

	/* No need to run MCI WA for palladium */
#if !PALLADIUM
	/* Re-init MCI connection due bug in Armada-8k-plus */
	if (ap810_rev_id_get(MVEBU_AP0) == MVEBU_AP810_REV_ID_A0)
		a8kp_mci_wa_initialize();
#endif

	/* Initialize the MCI threshold to improve performance */
	a8kp_mci_configure_threshold();

	/* misc configuration of the SoC */
	ap810_soc_misc_configurations();

	/* Update configuration space of CP110 from 0xf200_0000, to the
	 * new address according to address map of Armada-8k-plus family.
	 */
	update_cp110_default_win();

	/* configure AP810 address decode - call it after
	 * update_cp110_default_win to make sure that temporary windows do
	 * not override any window that will be configured in GWIN/CCU/IOWIN
	 */
	ap810_addr_decode_init();
}
