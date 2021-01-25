/*
 * Copyright 2020-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SSRAM_MAILBOX_H
#define SSRAM_MAILBOX_H

#include <platform_def.h>

#define CSR_SETTING_OFFSET offsetof(struct s32g_ssram_mailbox, csr_settings)
#define BL31SSRAM_CSR_BASE (BL31SSRAM_MAILBOX + CSR_SETTING_OFFSET)
#define BL31SSRAM_CSR_SIZE (0x298)

typedef void (*s32g_warm_entrypoint_t)(void);

struct s32g_ssram_mailbox {
	s32g_warm_entrypoint_t bl31_warm_entrypoint;
	uint8_t csr_settings[BL31SSRAM_CSR_SIZE];
};

#endif
