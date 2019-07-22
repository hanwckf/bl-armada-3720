/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <asm_macros.S>
#include <marvell_def.h>
#include <marvell_pm.h>
#include <platform_def.h>

	.globl  plat_marvell_calc_core_pos_die
	.globl  plat_marvell_calc_core_pos
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_get_my_entrypoint
	.globl	plat_is_my_cpu_primary
	.globl  plat_reset_handler

	/* -----------------------------------------------------
	 *  unsigned int plat_marvell_calc_core_pos_ap(uint64_t mpidr)
	 *  Helper function to calculate the core position in the specific die.
	 *  With this function: CorePos = (ClusterId * 2) +
	 *  				  CoreId
	 * -----------------------------------------------------
	 */
func plat_marvell_calc_core_pos_die
	and     x1, x0, #MPIDR_CPU_MASK
	and     x0, x0, #MPIDR_CLUSTER_MASK
	add     x0, x1, x0, LSR #7
	ret
endfunc plat_marvell_calc_core_pos_die

	/* -----------------------------------------------------
	 *  unsigned int plat_marvell_calc_core_pos(uint64_t mpidr)
	 *  Helper function to calculate the core position.
	 *  With this function: CorePos = (ApId * 8)
	 *				  (ClusterId * 2) +
	 *  				  CoreId
	 * -----------------------------------------------------
	 */
func plat_marvell_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x2, x0, #MPIDR_CLUSTER_MASK
	and	x0, x0, #(MPIDR_AFFLVL_MASK << MPIDR_AFF2_SHIFT)
	add	x0, x1, x0, LSR #13  /* Shift Affinity level 2 by 13 bits
					so AP ID will be multiplied by 8 (2^(16-13)) */
	add	x0, x0, x2, LSR #7   /* Shift Affinity level 1 by 7 bits
                                        so Cluster will be multiplied by 2 (2^(8-7)) */
	ret
endfunc plat_marvell_calc_core_pos

	/* -----------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset. Right
	 * now this is a stub function.
	 * -----------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	mov	x0, #0
	ret
endfunc plat_secondary_cold_boot_setup

	/* ---------------------------------------------------------------------
	 * unsigned long plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish
	 * between a cold and warm boot
	 * For a cold boot, return 0.
	 * For a warm boot, read the mailbox and return the address it contains.
	 *
	 * ---------------------------------------------------------------------
	 */
func plat_get_my_entrypoint
	mov_imm x0, PLAT_MARVELL_MAILBOX_BASE	/* Read first word and compare it with magic num */
	ldr     x1, [x0]
	mov_imm x2, MVEBU_MAILBOX_MAGIC_NUM
	cmp     x1, x2
	beq     warm_boot			/* If compare failed, return 0, i.e. cold boot */
	mov     x0, #0
	ret
warm_boot:
	mov_imm x1, MBOX_IDX_SEC_ADDR		/* Get the jump address */
	subs	x1, x1, #1
	mov	x2, #(MBOX_IDX_SEC_ADDR * 8)
	lsl	x3, x2, x1
	add     x0, x0, x3
	ldr     x0, [x0]
	ret
endfunc plat_get_my_entrypoint

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current cpu is the primary
	 * cpu.
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK | (MPIDR_AFFLVL_MASK << MPIDR_AFF2_SHIFT))
	cmp	x0, #MVEBU_PRIMARY_CPU
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary

        /* -----------------------------------------------------
	 * void plat_reset_handler (void);
         *
	 * Platform specific configuration right after cpu is
	 * is our of reset.
	 *
         * The plat_reset_handler can clobber x0 - x18, x30.
         * -----------------------------------------------------
         */
func plat_reset_handler
	/*
	 * Note: the configurations below  should be done before MMU,
	 *	  I Cache and L2are enabled.
	 *	  The reset handler is executed right after reset
	 * 	  and before Caches are enabled.
	 */

	/* Enable L1/L2 ECC and Parity */
	mrs x5, s3_1_c11_c0_2  /* L2 Ctrl */
	orr x5, x5, #(1 << 21) /* Enable L1/L2 cache ECC & Parity */
	msr s3_1_c11_c0_2, x5  /* L2 Ctrl */

#if LLC_ENABLE
	/*
	 * Enable L2 UniqueClean evictions
	 *  Note: this configuration assumes that LLC is configured
	 *	  in exclusive mode.
	 *	  Later on in the code this assumption will be validated
	 */
	mrs x5, s3_1_c15_c0_0  /* L2 Ctrl */
	orr x5, x5, #(1 << 14) /* Enable UniqueClean evictions with data */
	msr s3_1_c15_c0_0, x5  /* L2 Ctrl */
#endif

	/* Instruction Barrier to allow msr command completion */
	isb

        ret
endfunc plat_reset_handler
