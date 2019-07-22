/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */
#ifndef __MARVELL_SPD_PRIVATE_H__
#define __MARVELL_SPD_PRIVATE_H__

#include <arch.h>
#include <context.h>
#include <interrupt_mgmt.h>
#include <platform_def.h>

/*******************************************************************************
 * Secure Payload execution state information i.e. aarch32 or aarch64
 ******************************************************************************/
#define MARVELL_SPD_AARCH32		MODE_RW_32
#define MARVELL_SPD_AARCH64		MODE_RW_64

/*******************************************************************************
 * Number of cpus that the present on this platform. TODO: Rely on a topology
 * tree to determine this in the future to avoid assumptions about mpidr
 * allocation
 ******************************************************************************/
#define MARVELL_SPD_CORE_COUNT		PLATFORM_CORE_COUNT

#define MARVELL_SPD_C_RT_CTX_X19		0x0
#define MARVELL_SPD_C_RT_CTX_X20		0x8
#define MARVELL_SPD_C_RT_CTX_X21		0x10
#define MARVELL_SPD_C_RT_CTX_X22		0x18
#define MARVELL_SPD_C_RT_CTX_X23		0x20
#define MARVELL_SPD_C_RT_CTX_X24		0x28
#define MARVELL_SPD_C_RT_CTX_X25		0x30
#define MARVELL_SPD_C_RT_CTX_X26		0x38
#define MARVELL_SPD_C_RT_CTX_X27		0x40
#define MARVELL_SPD_C_RT_CTX_X28		0x48
#define MARVELL_SPD_C_RT_CTX_X29		0x50
#define MARVELL_SPD_C_RT_CTX_X30		0x58
#define MARVELL_SPD_C_RT_CTX_SIZE		0x60
#define MARVELL_SPD_C_RT_CTX_ENTRIES		(MARVELL_SPD_C_RT_CTX_SIZE >> DWORD_SHIFT)

#ifndef __ASSEMBLY__

/*******************************************************************************
 * Structure which helps the SPD to maintain the per-cpu state of the SP.
 * 'mpidr'          - mpidr to associate a context with a cpu
 * 'c_rt_ctx' - spaces to restore C runtime context from after returning
 *              from a synchronous entry into the SP.
 * 'cpu_ctx'  - space to maintain SP architectural state
 ******************************************************************************/
struct marvell_spd_context {
	uint32_t state;
	uint64_t mpidr;
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
};


/*******************************************************************************
 * Function & Data prototypes
 ******************************************************************************/
uint64_t marvell_spd_enter_sp(uint64_t *c_rt_ctx);
void __dead2 marvell_spd_exit_sp(uint64_t c_rt_ctx, uint64_t ret);
uint64_t marvell_spd_synchronous_sp_entry(struct marvell_spd_context *marvell_spd_ctx);
void __dead2 marvell_spd_synchronous_sp_exit(struct marvell_spd_context *marvell_spd_ctx, uint64_t ret);
void marvell_spd_init_marvell_spd_ep_state(struct entry_point_info *marvell_spd_entry_point,
				uint32_t rw,
				uint64_t pc,
				struct marvell_spd_context *marvell_spd_ctx);
uint64_t marvell_spd_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2,
			 uint64_t x3, uint64_t x4, void *cookie, void *handle,
			 uint64_t flags);
void marvell_spd_switch_to(uint64_t dst_world);
struct marvell_spd_context marvell_spd_sp_context;
#endif /*__ASSEMBLY__*/

#endif /* __MARVELL_SPD_PRIVATE_H__ */
