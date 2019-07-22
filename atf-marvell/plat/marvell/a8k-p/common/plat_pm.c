/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */
#include <debug.h>
#include <gicv3.h>
#include <mmio.h>
#include <marvell_plat_priv.h>
#include <platform.h>
#include <plat_marvell.h>

#define MVEBU_CCU_RVBAR(ap, clus, cpu)	(MVEBU_REGS_BASE_AP(ap) + 0x1800 + \
					(0x400 * clus) + 0x240 + (cpu * 0x4))
#define MVEBU_CCU_PRCR(ap, clus, cpu)	(MVEBU_REGS_BASE_AP(ap) + 0x1800 + \
					(0x400 * clus) + 0x250 + (cpu * 0x4))

#define MVEBU_RFU_GLOBL_SW_RST		0x184

int ap_init_status[PLAT_MARVELL_NORTHB_COUNT];

static int plat_marvell_cpu_on(u_register_t mpidr)
{
	int ap_id, clust_id, cpu_id;

	/* Set barrier */
	__asm__ volatile("dsb     sy");

	/* Get AP-ID number - use affinity level 2 */
	ap_id = MPIDR_AP_ID_GET(mpidr);

	/* Get cluster number - use affinity level 1 */
	clust_id = MPIDR_CLUSTER_ID_GET(mpidr);

	/* Get cpu number - use affinity level 0 */
	cpu_id =  MPIDR_CPU_ID_GET(mpidr);

	/* Set the cpu start address to BL1 entry point (align to 0x10000) */
	mmio_write_32(MVEBU_CCU_RVBAR(ap_id, clust_id, cpu_id),
		      PLAT_MARVELL_CPU_ENTRY_ADDR >> 16);

	/* Get the cpu out of reset */
	mmio_write_32(MVEBU_CCU_PRCR(ap_id, clust_id, cpu_id), 0x10001);

	return 0;
}

/*****************************************************************************
 * A8Kp handler called to check the validity of the power state
 * parameter.
 *****************************************************************************
 */
static int a8kp_validate_power_state(unsigned int power_state,
			    psci_power_state_t *req_state)
{
	int pstate = psci_get_pstate_type(power_state);
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int i;

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	/* Sanity check the requested state */
	if (pstate == PSTATE_TYPE_STANDBY) {
		/*
		 * It's possible to enter standby only on power level 0
		 * Ignore any other power level.
		 */
		if (pwr_lvl != MARVELL_PWR_LVL0)
			return PSCI_E_INVALID_PARAMS;

		req_state->pwr_domain_state[MARVELL_PWR_LVL0] =
					MARVELL_LOCAL_STATE_RET;
	} else {
		for (i = MARVELL_PWR_LVL0; i <= pwr_lvl; i++)
			req_state->pwr_domain_state[i] =
					MARVELL_LOCAL_STATE_OFF;
	}

	/*
	 * We expect the 'state id' to be zero.
	 */
	if (psci_get_pstate_id(power_state))
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

/*****************************************************************************
 * A8Kp handler called when a CPU is about to enter standby.
 *****************************************************************************
 */
static void a8kp_cpu_standby(plat_local_state_t cpu_state)
{
	ERROR("%s: needs to be implemented\n", __func__);
	panic();
}

/*****************************************************************************
 * A8Kp handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 *****************************************************************************
 */
static int a8kp_pwr_domain_on(u_register_t mpidr)
{
	/* TODO: check if cpu_powerup is needed for AP810 */

	/* proprietary CPU ON exection flow */
	plat_marvell_cpu_on(mpidr);

	return 0;
}

/*****************************************************************************
 * A8Kp handler called to validate the entry point.
 *****************************************************************************
 */
static int a8kp_validate_ns_entrypoint(uintptr_t entrypoint)
{
	return PSCI_E_SUCCESS;
}

/*****************************************************************************
 * A8Kp handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 *****************************************************************************
 */
static void a8kp_pwr_domain_off(const psci_power_state_t *target_state)
{
	ERROR("%s: needs to be implemented\n", __func__);
	panic();
}

/*****************************************************************************
 * A8Kp handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 *****************************************************************************
 */
static void a8kp_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	ERROR("%s: needs to be implemented\n", __func__);
	panic();
}

/*****************************************************************************
 * A8Kp handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 *****************************************************************************
 */
static void a8kp_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	int ap_id;
	u_register_t mpidr;

	mpidr = read_mpidr_el1();
	ap_id = MPIDR_AP_ID_GET(mpidr);

	/* If the AP is not initialized already, init the AP */
	if (ap_id != 0 && ap_init_status[ap_id] == 0) {
		ap_init_status[ap_id] = 1;
		write_cntfrq_el0(plat_get_syscnt_freq2());
		/* initialize only once in APx */
		plat_marvell_gic_driver_init();
	}

	/* arch specific configuration */
	marvell_psci_arch_init(ap_id);

	/* Per-CPU interrupt initialization */
	plat_marvell_gic_pcpu_init();
	plat_marvell_gic_cpuif_enable();
}

/*****************************************************************************
 * A8Kp handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 * TODO: At the moment we reuse the on finisher and reinitialize the secure
 * context. Need to implement a separate suspend finisher.
 *****************************************************************************
 */
static void a8kp_pwr_domain_suspend_finish(const psci_power_state_t
					   *target_state)
{
	ERROR("%s: needs to be implemented\n", __func__);
	panic();
}

/*****************************************************************************
 * This handler is called by the PSCI implementation during the `SYSTEM_SUSPEND`
 * call to get the `power_state` parameter. This allows the platform to encode
 * the appropriate State-ID field within the `power_state` parameter which can
 * be utilized in `pwr_domain_suspend()` to suspend to system affinity level.
 *****************************************************************************
 */
void a8kp_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	/* lower affinities use PLAT_MAX_OFF_STATE */
	for (int i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
}

/*****************************************************************************
 * A8Kp handlers to shutdown/reboot the system
 *****************************************************************************
 */
static void __dead2 a8kp_system_off(void)
{
	ERROR("%s:  needs to be implemented\n", __func__);
	panic();
}

void plat_marvell_system_reset(void)
{
	mmio_write_32(MVEBU_AR_RFU_BASE(0) + MVEBU_RFU_GLOBL_SW_RST, 0x0);
}

static void __dead2 a8kp_system_reset(void)
{
	plat_marvell_system_reset();

	/* we shouldn't get to this point */
	panic();
}

/*****************************************************************************
 * Export the platform handlers via plat_arm_psci_pm_ops. The ARM Standard
 * platform layer will take care of registering the handlers with PSCI.
 *****************************************************************************
 */
const plat_psci_ops_t plat_arm_psci_pm_ops = {
	.cpu_standby = a8kp_cpu_standby,
	.pwr_domain_on = a8kp_pwr_domain_on,
	.pwr_domain_off = a8kp_pwr_domain_off,
	.pwr_domain_suspend = a8kp_pwr_domain_suspend,
	.pwr_domain_on_finish = a8kp_pwr_domain_on_finish,
	.get_sys_suspend_power_state = a8kp_get_sys_suspend_power_state,
	.pwr_domain_suspend_finish = a8kp_pwr_domain_suspend_finish,
	.system_off = a8kp_system_off,
	.system_reset = a8kp_system_reset,
	.validate_power_state = a8kp_validate_power_state,
	.validate_ns_entrypoint = a8kp_validate_ns_entrypoint
};
