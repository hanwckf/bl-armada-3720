/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef _ICU_H_
#define _ICU_H_

#include <stdint.h>

#define MVEBU_ICU_REG_OFFSET                   (0x1E0000)

struct icu_irq {
	uint8_t	icu_id;
	uint8_t	spi_id;
	uint8_t	is_edge;
};

struct icu_config_entry {
	const struct icu_irq	*map;
	int			size;
};

struct icu_config {
	struct icu_config_entry sei;
	struct icu_config_entry rei;
};

void icu_init(uintptr_t cp_base, const struct icu_config *config);

#endif /* _ICU_H_ */

