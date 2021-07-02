/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef S32G3_SRAMC_H
#define S32G3_SRAMC_H

#include <lib/utils_def.h>
#include <s32g_sramc.h>

#define SRAMC2_BASE_ADDR	0x4055A000
#define SRAMC2_BASE_ADDR_H	(SRAMC0_BASE_ADDR >> 16)
#define SRAMC2_BASE_ADDR_L	((SRAMC0_BASE_ADDR & 0xFFFF))
#define SRAMC3_BASE_ADDR	0x4055E000
#define SRAMC3_BASE_ADDR_H	(SRAMC1_BASE_ADDR >> 16)
#define SRAMC3_BASE_ADDR_L	((SRAMC1_BASE_ADDR & 0xFFFF))

/* Block ranges */
#define SRAMC0_MIN_ADDR		(0x0)
#define SRAMC0_MAX_ADDR		(0x7FFF)
#define SRAMC1_MIN_ADDR		(SRAMC0_MAX_ADDR + 1)
#define SRAMC1_MAX_ADDR		(0xFFFF)
#define SRAMC2_MIN_ADDR		(SRAMC1_MAX_ADDR + 1)
#define SRAMC2_MAX_ADDR		(0x17FFF)
#define SRAMC2_MAX_ADDR_H	(SRAMC2_MAX_ADDR >> 16)
#define SRAMC2_MAX_ADDR_L	(SRAMC2_MAX_ADDR & 0xFFFF)
#define SRAMC3_MIN_ADDR		(SRAMC2_MAX_ADDR + 1)
#define SRAMC3_MIN_ADDR_H	(SRAMC3_MIN_ADDR >> 16)
#define SRAMC3_MIN_ADDR_L	(SRAMC3_MIN_ADDR & 0xFFFF)
#define SRAMC3_MAX_ADDR		(0x1FFFF)
#define SRAMC3_MAX_ADDR_H	(SRAMC3_MAX_ADDR >> 16)
#define SRAMC3_MAX_ADDR_L	(SRAMC3_MAX_ADDR & 0xFFFF)

#endif

