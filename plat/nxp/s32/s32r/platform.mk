#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/nxp/s32/s32_common.mk

PLAT_INCLUDES	+=	-Iplat/nxp/s32/s32r/include \
					-Iplat/nxp/s32/include \

PLAT_BL_COMMON_SOURCES += plat/nxp/s32/s32gen1_sramc.c \

BL2_SOURCES 	+=  \
	plat/nxp/s32/s32r/s32r_bl2_el3.c \

BL31_SOURCES += \
	       plat/nxp/s32/s32r/s32r_bl31.c \
		   plat/nxp/s32/s32r/s32r_psci.c \

# Which LinFlexD to use as a UART device
S32_LINFLEX_MODULE := 0
$(eval $(call add_define_val,S32_LINFLEX_MODULE,$(S32_LINFLEX_MODULE)))

DTB_FILE_NAME		?= fsl-s32r45-evb.dtb