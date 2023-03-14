/*
 * Copyright 2021-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/desc_image_load.h>
#include "ddr_init.h"
#include <lib/libc/errno.h>
#include <lib/mmio.h>
#include "platform_def.h"
#include "s32_bl_common.h"
#include "s32_bl2_el3.h"
#if (ERRATA_S32_050543 == 1)
#include "s32_ddr_errata_funcs.h"
#endif
#include "s32_linflexuart.h"
#include "s32_storage.h"
#include "s32_sramc.h"

static bl_mem_params_node_t s32r_bl2_mem_params_descs[6];
REGISTER_BL_IMAGE_DESCS(s32r_bl2_mem_params_descs)

static enum reset_cause get_reset_cause(void)
{
	uint32_t mc_rgm_des = mmio_read_32(MC_RGM_DES);

	if (mc_rgm_des & DES_F_POR)
		return CAUSE_POR;

	if (mc_rgm_des & DES_F_DR_ANY)
		return CAUSE_DESTRUCTIVE_RESET_DURING_RUN;

	if (mmio_read_32(MC_RGM_FES) & FES_F_FR_ANY)
		return CAUSE_FUNCTIONAL_RESET_DURING_RUN;

	return CAUSE_ERROR;
}

void bl2_el3_early_platform_setup(u_register_t arg0, u_register_t arg1,
				  u_register_t arg2, u_register_t arg3)
{
	enum reset_cause reset_cause;
	size_t index = 0;
	bl_mem_params_node_t *params = s32r_bl2_mem_params_descs;
	size_t params_size = ARRAY_SIZE(s32r_bl2_mem_params_descs);
	int ret = 0;

	reset_cause = get_reset_cause();
	clear_reset_cause();

	s32_early_plat_init();
	console_s32_register();
	s32_io_setup();

	NOTICE("Reset status: %s\n", get_reset_cause_str(reset_cause));

	ret |= add_bl31_img_to_mem_params_descs(params, &index, params_size);
	ret |= add_bl32_img_to_mem_params_descs(params, &index, params_size);
	ret |= add_bl32_extra1_img_to_mem_params_descs(params, &index,
						      params_size);
	ret |= add_bl33_img_to_mem_params_descs(params, &index, params_size);
	ret |= add_invalid_img_to_mem_params_descs(params, &index,
						   params_size);
	if (ret)
		panic();

	bl_mem_params_desc_num = index;
}

void bl2_el3_plat_arch_setup(void)
{
	if (s32_el3_mmu_fixup(NULL, 0))
		panic();

	s32_sram_clear(S32_BL33_IMAGE_BASE, get_bl2_dtb_base());

	clear_swt_faults();

	if (ddr_init()) {
		ERROR("Failed to configure the DDR subsystem\n");
		panic();
	}

#if (ERRATA_S32_050543 == 1)
	ddr_errata_update_flag(polling_needed);
#endif
}

