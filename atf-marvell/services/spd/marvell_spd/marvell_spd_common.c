/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context_mgmt.h>
#include <string.h>
#include <gic_common.h>
#include <gicv2.h>
#include "marvell_spd_private.h"

/*******************************************************************************
 * Given a secure payload entrypoint info pointer, entry point PC, register
 * width, cpu id & pointer to a context data structure, this function will
 * initialize tsp context and entry point info for the secure payload
 ******************************************************************************/
void marvell_spd_init_marvell_spd_ep_state(struct entry_point_info *marvell_spd_entry_point,
				uint32_t rw,
				uint64_t pc,
				struct marvell_spd_context *marvell_spd_ctx)
{
	uint32_t ep_attr;

	/* Passing a NULL context is a critical programming error */
	assert(marvell_spd_ctx);
	assert(marvell_spd_entry_point);
	assert(pc);

	/*
	 * We support AArch32 marvell_spd for now.
	 * TODO: Add support for AArch64 marvell_spd
	 */
	assert(rw == MARVELL_SPD_AARCH32);

	/* Associate this context with the cpu specified */
	marvell_spd_ctx->mpidr = read_mpidr_el1();


	cm_set_context(&marvell_spd_ctx->cpu_ctx, SECURE);

	/* initialise an entrypoint to set up the CPU context */
	ep_attr = SECURE | EP_ST_ENABLE;
	if (read_sctlr_el3() & SCTLR_EE_BIT)
		ep_attr |= EP_EE_BIG;
	SET_PARAM_HEAD(marvell_spd_entry_point, PARAM_EP, VERSION_1, ep_attr);

	marvell_spd_entry_point->pc = pc;
	marvell_spd_entry_point->spsr = SPSR_MODE32(MODE32_svc, SPSR_T_ARM,
							SPSR_E_LITTLE,
							DAIF_FIQ_BIT |
							DAIF_IRQ_BIT |
							DAIF_ABT_BIT);
	memset(&marvell_spd_entry_point->args, 0, sizeof(marvell_spd_entry_point->args));
}

/*******************************************************************************
 * This function takes an SP context pointer and:
 * 1. Applies the S-EL1 system register context from marvell_spd_ctx->cpu_ctx.
 * 2. Saves the current C runtime state (callee saved registers) on the stack
 *    frame and saves a reference to this state.
 * 3. Calls el3_exit() so that the EL3 system and general purpose registers
 *    from the marvell_spd_ctx->cpu_ctx are used to enter the secure payload image.
 ******************************************************************************/
uint64_t marvell_spd_synchronous_sp_entry(struct marvell_spd_context *marvell_spd_ctx)
{
	uint64_t rc;

	assert(marvell_spd_ctx != NULL);
	assert(marvell_spd_ctx->c_rt_ctx == 0);

	/* Apply the Secure EL1 system register context and switch to it */
	assert(cm_get_context(SECURE) == &marvell_spd_ctx->cpu_ctx);
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	rc = marvell_spd_enter_sp(&marvell_spd_ctx->c_rt_ctx);

	return rc;
}


/*******************************************************************************
 * This function takes an SP context pointer and:
 * 1. Saves the S-EL1 system register context tp marvell_spd_ctx->cpu_ctx.
 * 2. Restores the current C runtime state (callee saved registers) from the
 *    stack frame using the reference to this state saved in tspd_enter_sp().
 * 3. It does not need to save any general purpose or EL3 system register state
 *    as the generic smc entry routine should have saved those.
 ******************************************************************************/
void marvell_spd_synchronous_sp_exit(struct marvell_spd_context *marvell_spd_ctx, uint64_t ret)
{
	assert(marvell_spd_ctx != NULL);
	/* Save the Secure EL1 system register context */
	assert(cm_get_context(SECURE) == &marvell_spd_ctx->cpu_ctx);
	cm_el1_sysregs_context_save(SECURE);

	assert(marvell_spd_ctx->c_rt_ctx != 0);
	marvell_spd_exit_sp(marvell_spd_ctx->c_rt_ctx, ret);

	/* Should never reach here */
	assert(0);
}

void marvell_spd_switch_to(uint64_t dst_world)
{
	assert(dst_world == NON_SECURE || dst_world == SECURE);

	cm_el1_sysregs_context_save(!dst_world);
	cm_el1_sysregs_context_restore(dst_world);
	cm_set_next_eret_context(dst_world);
}
