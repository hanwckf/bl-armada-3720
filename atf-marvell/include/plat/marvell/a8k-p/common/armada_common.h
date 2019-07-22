/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef __ARMADA_COMMON_H__
#define __ARMADA_COMMON_H__

#include <amb_adec.h>
#include <io_win.h>
#include <iob.h>
#include <ccu.h>
#include <gwin.h>

int marvell_get_mci_map(int ap_id, int cp_id);

uint32_t marvell_get_io_win_gcr_target(int ap_idx);
uint32_t marvell_get_ccu_gcr_target(int ap_idx);

/*
 * The functions below are defined as Weak and may be overridden
 * in specific Marvell standard platform
 */
int marvell_get_amb_memory_map(struct addr_map_win **win,
			       uint32_t *size, uintptr_t base);
int marvell_get_io_win_memory_map(int ap_idx, struct addr_map_win **win,
				  uint32_t *size);
int marvell_get_iob_memory_map(struct addr_map_win **win, uint32_t *size,
			       uintptr_t base);
int marvell_get_ccu_memory_map(int ap_idx, struct addr_map_win **win,
			       uint32_t *size);
int marvell_get_gwin_memory_map(int ap, struct addr_map_win **win,
				uint32_t *size);

#endif /* __ARMADA_COMMON_H__ */
