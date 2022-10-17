/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "bl31_sram.h"
#include "s32_plat_funcs.h"
#include "s32_sramc.h"
#include "s32g_bl_common.h"
#include "s32g_clocks.h"
#include "s32g_mc_me.h"
#include "s32g_resume.h"
#include "s32g_vr5510.h"
#include "s32gen1-wkpu.h"

#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/nxp/s32g/bl31_ssram/ssram_mailbox.h>
#include <plat/common/platform.h>

static void bl31sram_entry(void)
{
	bl31_sram_entry_t entry;

	entry = (void *)BL31SRAM_BASE;
	entry();
}

static void set_warm_entry(void)
{
	uintptr_t warm_entry, short_boot;

	warm_entry = BL31SSRAM_MAILBOX + offsetof(struct s32g_ssram_mailbox,
						  bl31_warm_entrypoint);
	short_boot = BL31SSRAM_MAILBOX + offsetof(struct s32g_ssram_mailbox,
						  short_boot);
	mmio_write_64(warm_entry, (uintptr_t)s32g_resume_entrypoint);
	mmio_write_8(short_boot, (uint8_t)s32gen1_is_wkp_short_boot());
}

static void __dead2 platform_suspend(unsigned int current_cpu)
{
	size_t i;
	size_t ncores = PLATFORM_CORE_COUNT;

	for (i = 0; i < PLATFORM_CORE_COUNT; i++)
		gicv3_cpuif_disable(i);

	plat_gic_save();
	set_warm_entry();
	pmic_prepare_for_suspend();
	s32gen1_wkpu_enable_irqs();

	/* Shutting down cores */
	/* M7 cores */
	s32_turn_off_mcores(0u);

	if (is_lockstep_enabled())
		ncores /= 2;

	/* A53 cores */
	for (i = 0; i < ncores; i++) {
		if (i != current_cpu)
			s32_turn_off_core(S32_MC_ME_CA53_PART, i);
	}

	/* PFE blocks */
	s32_disable_cofb_clk(S32G_MC_ME_PFE_PART, 0);
	/* Keep the DDR clock */
	s32_disable_cofb_clk(S32_MC_ME_USDHC_PART,
			      S32_MC_ME_PRTN_N_REQ(S32_MC_ME_DDR_0_REQ));

	/* Switching all MC_CGM muxes to FIRC */
	s32g_sw_clks2firc();

	/* Turn off DFS */
	s32g_disable_dfs(S32_PERIPH_DFS);
	s32g_disable_dfs(S32_CORE_DFS);

	/* Turn off PLL */
	s32g_disable_pll(S32_ACCEL_PLL, 2);
	s32g_disable_pll(S32_PERIPH_PLL, 8);
	s32g_disable_pll(S32_CORE_PLL, 2);

	bl31sram_entry();
	plat_panic_handler();
}

static void copy_bl31sram_image(void)
{
#if S32CC_EMU == 0
	uint32_t npages;
	int ret;

	/* Clear all BL31SRAM sections */
	ret = s32_sram_clear(BL31SRAM_BASE, BL31SRAM_LIMIT);
	if (ret)
		ERROR("Failed to initialize SRAM from BL31SRAM stage\n");

	npages = bl31sram_len / PAGE_SIZE;
	if (bl31sram_len % PAGE_SIZE)
		npages++;

	ret = xlat_change_mem_attributes(BL31SRAM_BASE,
					 npages * PAGE_SIZE,
					 MT_MEMORY | MT_RW | MT_EXECUTE_NEVER);
	if (ret)
		ERROR("Failed to change the attributes of BL31 SRAM memory\n");

	/* Copy bl31 sram stage */
	memcpy((void *)BL31SRAM_BASE, bl31sram, bl31sram_len);
	ret = xlat_change_mem_attributes(BL31SRAM_BASE,
			npages * PAGE_SIZE,
			MT_CODE | MT_SECURE);
	if (ret)
		ERROR("Failed to change the attributes of BL31 SRAM memory\n");
#endif
}

void s32_plat_suspend(unsigned int cpu)
{
	copy_bl31sram_image();
	platform_suspend(cpu);
}
