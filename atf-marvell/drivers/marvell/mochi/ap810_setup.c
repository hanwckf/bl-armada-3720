/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <ap810_setup.h>
#include <debug.h>
#include <mmio.h>
#include <mvebu_def.h>

#define CCU_B_GIDACR(ap, stop)			(MVEBU_A2_BANKED_STOP_BASE(ap, stop) + 0x34)

#define CCU_B_LTC_CR(ap)			(MVEBU_REGS_BASE_AP(ap) + 0x344)
#define LTC_MULTI_CHIP_TRAIN_MODE_EN		(1 << 15)

#define MRI_XBAR_PORTx_ROUTING0(ap, port)	(MVEBU_MRI_XBAR_BASE(ap) + 0x10 + 0x8 * port)

#define MVEBU_CCU_GUID(ap)			(MVEBU_REGS_BASE_AP(ap) + 0x4808)

#define MVEBU_IHBX4_CONTROL_SR(ap, mci)		(MVEBU_AP_IHBX4_CNTRL_BASE(ap, mci) + 0xc)
#define IHBX4_SR_COHERENT_PORT_OFFSET		7
#define IHBX4_SR_COHERENT_PORT_MASK		(0x1 << IHBX4_SR_COHERENT_PORT_OFFSET)
#define IHBX4_SR_IHB_READY_OFFSET		5
#define IHBX4_SR_IHB_READY_MASK			(0x1 << IHBX4_SR_IHB_READY_OFFSET)

/* AP810 revision ID */
#define MVEBU_GWD_IIDR2_REG(ap)		(MVEBU_AP_WD_BASE(ap) + 0xFCC)
#define GWD_IIDR2_REV_ID_OFFSET		12
#define GWD_IIDR2_REV_ID_MASK		0xF

/* Global AP count */
int g_ap_count = -1;
/* Global CP per AP count */
int g_cp_per_ap[] = {-1, -1, -1, -1};

/* Configure access between AP, use static configuration */
void ap810_enumeration_algo(void)
{
	uint32_t reg;
	int ap_id;

	/* In case of single AP, no need to configure MRI-xbar */
	if (ap_get_count() == 1)
		return;

	debug_enter();
	ap810_setup_banked_rgf(0);

	/* Enable training bit - for AP0 */
	reg = mmio_read_32(CCU_B_LTC_CR(0));
	mmio_write_32(CCU_B_LTC_CR(0), reg | LTC_MULTI_CHIP_TRAIN_MODE_EN);

	/* Configure MRI XBar
	 * MRI XBAR include access configuration for other APs.
	 * MRI XBAR have 5 ports, port 0 connected to global stop
	 * in the Aurora, and 4 other ports connected to other APs
	 * one other port is not connected.
	 * For every port there's register PORT_%x_ROUTING0, that mean
	 * every transaction come from port %x with AP-ID 0/1/2/3
	 * will exit on port number %y that written in register.
	 * e.g.:
	 *     AP0.PORT0_ROUTING0: {1,3,4,0}:
	 *     all transaction comes from port 0, with AP-ID 0 goes to
	 *     port 0 (return to AP0), AP-ID 1 goes to port 4,
	 *     AP-ID 2 goes to port 3, AP-ID 3 goes to port 1.
	 *
	 * QUAD AP Clique (all AP dies connected to each other)
	 * AP0: port 1 -> AP3, port 2 -> NA, port 3 -> AP2, port 4 -> AP3
	 * AP1: port 1 -> AP3, port 2 -> AP0, port 3 -> AP2, port 4 -> NA
	 * AP2: port 1 -> AP1, port 2 -> NA, port 3 -> AP0, port 4 -> AP3
	 * AP3: port 1 -> AP2, port 2 -> AP3, port 3 -> AP0, port 4 -> NA
	 * mri-xbar configurations:
	 *      AP0.PORT0_ROUTING0: {1,3,4,0}
	 *      AP1.PORT0_ROUTING0: {1,3,0,2}
	 *      AP2.PORT0_ROUTING0: {4,0,1,3}
	 *      AP3.PORT0_ROUTING0: {0,2,1,3}
	 *
	 * AP0: port 1 -> NA, port 2 -> NA, port 3 -> AP1, port 4 -> NA
	 * AP1: port 1 -> AP0, port 2 -> NA, port 3 -> NA, port 4 -> NA
	 * DUAL AP:
	 *      AP0.PORT0_ROUTING0: {3,0}
	 *      AP1.PORT0_ROUTING0: {0,1}
	 */
	if (ap_get_count() == 2) {
		mmio_write_32(MRI_XBAR_PORTx_ROUTING0(0, 0), 0x30);
		mmio_write_32(MRI_XBAR_PORTx_ROUTING0(1, 0), 0x1);
	} else if (ap_get_count() == 4) {
		mmio_write_32(MRI_XBAR_PORTx_ROUTING0(0, 0), 0x1340);
		mmio_write_32(MRI_XBAR_PORTx_ROUTING0(1, 0), 0x1302);
		mmio_write_32(MRI_XBAR_PORTx_ROUTING0(2, 0), 0x4013);
		mmio_write_32(MRI_XBAR_PORTx_ROUTING0(3, 0), 0x0213);
	}

	/* Disable training bit */
	mmio_write_32(CCU_B_LTC_CR(0), reg);

	/* Test AP access */
	if (ap_get_count() == 2) {
		/* Read status from AP1 */
		INFO("Test AP1: 0x%x\n", mmio_read_32(MRI_XBAR_PORTx_ROUTING0(1, 0)));
	} else if (ap_get_count() == 4) {
		/* Read status from AP1 */
		INFO("Test AP1: 0x%x\n", mmio_read_32(MRI_XBAR_PORTx_ROUTING0(1, 0)));
		/* Read status from AP2 */
		INFO("Test AP2: 0x%x\n", mmio_read_32(MRI_XBAR_PORTx_ROUTING0(2, 0)));
		/* Read status from AP3 */
		INFO("Test AP3: 0x%x\n", mmio_read_32(MRI_XBAR_PORTx_ROUTING0(3, 0)));
	}

	/* Update AP-ID of every AP die in the system */
	for (ap_id = 0; ap_id < ap_get_count(); ap_id++)
		mmio_write_32(MVEBU_CCU_GUID(ap_id), ap_id);
	debug_exit();
}

/* Get AP count, by read how many coherent
 * ports connected to AP0. For now assume
 * that AP0 is connected to all the APs in the system
 */
int ap_get_count(void)
{
	uint32_t reg;
	int count;

	if (g_ap_count != -1)
		return g_ap_count;

	debug_enter();
	count = 1; /* start with the local AP */
	reg = mmio_read_32(MVEBU_DFX_SAR_REG(0, 0));
	reg = (reg >> MVEBU_SAR_0_COHERENT_EN_OFFSET) & MVEBU_SAR_0_COHERENT_EN_MASK;

	/* Count the coherent ports that enabled */
	while (reg) {
		count += reg & 1;
		reg >>= 1;
	}

	g_ap_count = count;

	INFO("Found %d APs\n", g_ap_count);

	debug_exit();
	return g_ap_count;
}

/* Returns static count of CPs assigned to a specific AP
 * This CP count is defined by CP_NUM set during the compilation
 * time and dynamically detected number of interconnected APs
 */
int ap810_get_cp_per_ap_static_cnt(int ap_id)
{
	const int ap_count = ap_get_count();
	int cps_per_ap_id = CP110_DIE_NUM / ap_count;
	int reminder = CP110_DIE_NUM % ap_count;

	/* The reminder of the integer division counts the CPs that
	 * cannot be evntly distributed across interconnected APs.
	 * So only low numbered APs will receive these CPs.
	 * If for instance the reminder is 2, the AP0 and AP1 will
	 * increase the number of their connected CP by 1,
	 * but AP2 and AP3 will keep this number intact.
	 */
	if (reminder && (reminder > ap_id))
		cps_per_ap_id += 1;

	return cps_per_ap_id;
}

int ap810_get_cp_per_ap_cnt(int ap_id)
{
	int mci_id, cp_per_ap = 0;
	uint32_t reg;

	if (g_cp_per_ap[ap_id] != -1)
		return g_cp_per_ap[ap_id];

	debug_enter();

	for (mci_id = 0; mci_id < MCI_MAX_UNIT_ID; mci_id++) {
		INFO("AP%d MCI-%d ", ap_id, mci_id);

		reg = mmio_read_32(MVEBU_IHBX4_CONTROL_SR(ap_id, mci_id));
		/* If MCI port is link down, skip this port */
		if (!(reg & IHBX4_SR_IHB_READY_MASK)) {
			INFO("- Port disabled\n");
			continue;
		}

		/* If MCI port is a coherent port (connected to AP),
		 * skip this port
		 */
		if (reg & IHBX4_SR_COHERENT_PORT_MASK) {
			INFO("- Port connected to AP\n");
			continue;
		}
		INFO("- Found MCI port connected to CP\n");
		cp_per_ap++;
	}
	INFO("Found %d CPs connected to AP-%d\n", cp_per_ap, ap_id);
	/* In case the build was created for a ceratain number of CPs,
	 * need to ignore the number of detected ones.
	 * This is useful mainly for debug, but also prevents from fail
	 * when early initialization stages processed lower number of CPs
	 * then the number of CPs actually detected on system.
	 */
	if (cp_per_ap > ap810_get_cp_per_ap_static_cnt(ap_id)) {
		cp_per_ap = ap810_get_cp_per_ap_static_cnt(ap_id);
		INFO("Limiting AP-%d CPs number to %d (CP_NUM=%d)\n", ap_id, cp_per_ap, CP110_DIE_NUM);
	}

	g_cp_per_ap[ap_id] = cp_per_ap;

	debug_exit();

	return cp_per_ap;
}

/* function to open access RGF to access another ring*/
void ap810_setup_banked_rgf(int ap_id)
{
	int val, stop;

	debug_enter();
	/* Open access for all the banked RGF
	 * (remote ring access registers)
	 * 0xf for QUAD - 0x3 for DUAL - 0x1 for single (default)
	 * Open access for all IO & proccess stops, because MC & SG
	 * stop can't start transcations to another ring
	 */
	val = AP810_MAX_AP_MASK >> (AP810_MAX_AP_NUM - ap_get_count());
	for (stop = 0; stop < AP810_S_END; stop++) {
		switch (stop) {
		case AP810_S0_SMC0:
		case AP810_S0_SMC1:
		case AP810_S0_SMC2:
		case AP810_S0_SG:
			continue;
		default:
			mmio_write_32(CCU_B_GIDACR(ap_id, stop), val);
		}
	}
	debug_exit();
}

int ap810_rev_id_get(int ap_index)
{
	/* Returns:
	 * - 0 (AP810_REV_ID_A0) for A0
	 */
	return (mmio_read_32(MVEBU_GWD_IIDR2_REG(ap_index)) >>
		GWD_IIDR2_REV_ID_OFFSET) &
		GWD_IIDR2_REV_ID_MASK;
}
