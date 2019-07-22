/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <bl31.h>
#include <context_mgmt.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>
#include <runtime_svc.h>
#include <stddef.h>
#include <uuid.h>
#include <gic_common.h>
#include <gicv2.h>
#include <marvell_spd.h>
#include "marvell_spd_private.h"

/*******************************************************************************
 * Array to keep track of per-cpu Secure Payload state
 ******************************************************************************/
struct marvell_spd_context marvell_spd_sp_context;

/*******************************************************************************
 * This function is the handler registered for S-EL1 interrupts by the MARVELL_SPD. It
 * validates the interrupt and upon success switch to TEE for handling the interrupt.
 ******************************************************************************/
static uint64_t marvell_spd_sel1_interrupt_handler(uint32_t id,
						uint32_t flags,
						void *handle,
						void *cookie)
{
	cpu_context_t *sw_cpu_context;
	/* Check the security state when the exception was generated */
	assert(get_interrupt_src_ss(flags) == NON_SECURE);

#if IMF_READ_INTERRUPT_ID
	/* Check the security status of the interrupt */
	assert(plat_ic_get_interrupt_type(id) == INTR_TYPE_S_EL1);
#endif

	/* Sanity check the pointer to this cpu's context */
	assert(handle == cm_get_context(NON_SECURE));

	/* Get a reference to the secure context */
	sw_cpu_context = cm_get_context(SECURE);
	assert(sw_cpu_context);

	cm_el1_sysregs_context_save(NON_SECURE);
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);
	/* Return to S-EL1 */
	SMC_RET0(sw_cpu_context);
}

/*******************************************************************************
 * This function passes control to the TEE image (BL32) for the first
 * time on the primary cpu after a cold boot. It assumes that a valid secure
 * context has already been created by marvell_spd_setup() which can be directly used.
 * It also assumes that a valid non-secure context has been initialised by PSCI
 * so it does not need to save and restore any non-secure state. This function
 * performs a synchronous entry into the Secure payload. The SP passes control
 * back to this routine through a SMC. It also passes the extents of memory made
 * available to BL32 by BL31.
 ******************************************************************************/
int32_t marvell_spd_init(void)
{
	struct marvell_spd_context *marvell_spd_ctx = &marvell_spd_sp_context;
	entry_point_info_t *marvell_spd_entry_point;
	uint64_t rc;

	/*
	 * Get information about the Secure Payload (BL32) image. Its
	 * absence is a critical failure.
	 */
	marvell_spd_entry_point = bl31_plat_get_next_image_ep_info(SECURE);
	assert(marvell_spd_entry_point);

	cm_init_my_context(marvell_spd_entry_point);

	/*
	 * Arrange for an entry into the test secure payload. It will be
	 * returned via TSP_ENTRY_DONE case
	 */
	rc = marvell_spd_synchronous_sp_entry(marvell_spd_ctx);

	return rc;
}

/*******************************************************************************
 * Secure Payload Dispatcher setup. The SPD finds out the SP entrypoint and type
 * (aarch32/aarch64) if not already known and initialises the context for entry
 * into the SP for its initialisation.
 ******************************************************************************/
int32_t marvell_spd_setup(void)
{
	entry_point_info_t *marvell_spd_ep_info;
	uint32_t linear_id;

	linear_id = plat_my_core_pos();
	if (linear_id != 0)
		return -EPERM;

	/*
	 * Get information about the Secure Payload (BL32) image. Its
	 * absence is a critical failure.  TODO: Add support to
	 * conditionally include the SPD service
	 */
	marvell_spd_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (!marvell_spd_ep_info) {
		WARN("No marvell_spd provided by BL2 boot loader,");
		WARN("Booting device without marvell_spd initialization.");
		WARN("SMC`s destined for TSP will return SMC_UNK\n");
		return 1;
	}

	/*
	 * If there's no valid entry point for SP, we return a non-zero value
	 * signalling failure initializing the service. We bail out without
	 * registering any handlers
	 */
	if (!marvell_spd_ep_info->pc)
		return 1;

	/*
	 * We could inspect the SP image and determine it's execution
	 * state i.e whether AArch32 or AArch64
	 */
	marvell_spd_init_marvell_spd_ep_state(marvell_spd_ep_info,
					MARVELL_SPD_AARCH32,
					marvell_spd_ep_info->pc,
					&marvell_spd_sp_context);

	/*
	 * All MARVELL_SPD initialization done. Now register our init function with
	 * BL31 for deferred invocation
	 */
	bl31_register_bl32_init(&marvell_spd_init);

	return 0;
}

/*******************************************************************************
 * This function is responsible for handling all SMCs in the Trusted OS/App
 * range from the non-secure state as defined in the SMC Calling Convention
 * Document. It is also responsible for communicating with the Secure payload
 * to delegate work and return results back to the non-secure state. Lastly it
 * will also return any information that the secure payload needs to do the
 * work assigned to it.
 ******************************************************************************/
uint64_t marvell_spd_smc_handler(uint32_t smc_fid,
			 uint64_t x1,
			 uint64_t x2,
			 uint64_t x3,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags)
{
	uint32_t linear_id = plat_my_core_pos();
	struct marvell_spd_context *marvell_spd_ctx = &marvell_spd_sp_context;
	cpu_context_t *ns_cpu_context;
	cpu_context_t *sw_cpu_context;

	static int64_t initialized = -1;

	if (linear_id != 0)
		SMC_RET1(handle, SMC_UNK);

	/* Get a reference to the non-secure context */
	ns_cpu_context = cm_get_context(NON_SECURE);
	assert(ns_cpu_context);

	sw_cpu_context = cm_get_context(SECURE);
	assert(sw_cpu_context);

	switch (smc_fid) {
	case MARVELL_SPD_TW_SMC:
		if (initialized != 1) {
			initialized = 1;
			uint64_t rc;

			/*
			 * Register an interrupt handler for S-EL1 interrupts
			 * when generated during code executing in the
			 * non-secure state.
			 */
			flags = 0;
			set_interrupt_rm_flag(flags, NON_SECURE);
			rc = register_interrupt_type_handler(INTR_TYPE_S_EL1,
						marvell_spd_sel1_interrupt_handler,
						flags);
			if (rc)
				panic();

			marvell_spd_synchronous_sp_exit(marvell_spd_ctx, x1);
		} else {
			marvell_spd_switch_to(NON_SECURE);
			SMC_RET0(ns_cpu_context);
		}
		break;

	case MARVELL_SPD_NTW_NEW_REQ:
#ifdef CONFIG_GICV3
		write_icc_sgi1r_el1(0xE000001);
		isb();
#else
		gicd_write_sgir(PLAT_MARVELL_GICD_BASE, 0x0200000E);
#endif /* CONFIG_GICV3 */
		/* fall through */
	case MARVELL_SPD_NTW_SMC:
		marvell_spd_switch_to(SECURE);
		SMC_RET0(sw_cpu_context);
		break;
	}
	SMC_RET1(handle, SMC_UNK);
}

/* Define a MARVELL_SPD runtime service descriptor */
DECLARE_RT_SVC(
	marvell_spd,
	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_STD,
	marvell_spd_setup,
	marvell_spd_smc_handler
);
