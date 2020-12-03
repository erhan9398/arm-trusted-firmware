/*
 * Copyright 2020-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "s32g_bl_common.h"
#include "s32g_linflexuart.h"
#include "s32g_resume.h"
#include "s32gen1-wkpu.h"
#include <bl31/bl31.h>		/* for bl31_warm_entrypoint() */
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/mmio.h>

void s32g_ncore_isol_cluster0(void);

static void reset_rtc(void)
{
	uint32_t rtc = S32G_RTC_BASE;
	uint32_t rtcs;

	mmio_write_32(rtc + RTC_APIVAL_OFFSET, 0x0);
	mmio_write_32(rtc + RTC_RTCVAL_OFFSET, 0x0);

	mmio_write_32(rtc + RTC_RTCC_OFFSET, 0x0);

	rtcs = mmio_read_32(rtc + RTC_RTCS_OFFSET);
	mmio_write_32(rtc + RTC_RTCS_OFFSET, rtcs);
}

void s32g_resume_entrypoint(void)
{
	int ret;

	ret = pmic_disable_wdg();
	if (ret)
		ERROR("Failed to disable VR5510 watchdog\n");

	/* Prepare resume operation */
	s32g_ncore_isol_cluster0();
	s32g_early_plat_init(true);

	reset_rtc();
	s32gen1_wkpu_reset();

	console_s32g_register();
	plat_gic_restore();
	bl31_warm_entrypoint();
}