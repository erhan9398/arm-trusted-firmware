/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/mmio.h>

#include <platform_def.h>

#define ICFG_IPROC_IOPAD_CTRL_4 (IPROC_ROOT + 0x9c0)
#define ICFG_IPROC_IOPAD_CTRL_5 (IPROC_ROOT + 0x9c4)
#define ICFG_IPROC_IOPAD_CTRL_6 (IPROC_ROOT + 0x9c8)
#define ICFG_IPROC_IOPAD_CTRL_7 (IPROC_ROOT + 0x9cc)

#define IOPAD_CTRL4_SDIO0_CD_IND_R 30
#define IOPAD_CTRL4_SDIO0_CD_SRC_R 31
#define IOPAD_CTRL4_SDIO0_CD_HYS_R 29
#define IOPAD_CTRL4_SDIO0_CD_PULL_R 28
#define IOPAD_CTRL4_SDIO0_CD_DRIVE_R 24
#define IOPAD_CTRL4_SDIO0_CLK_SDCARD_SRC_R 23
#define IOPAD_CTRL4_SDIO0_CLK_SDCARD_HYS_R 21
#define IOPAD_CTRL4_SDIO0_CLK_SDCARD_DRIVE_R 17

#define IOPAD_CTRL4_SDIO0_DATA0_SRC_R 15
#define IOPAD_CTRL4_SDIO0_DATA0_HYS_R 13
#define IOPAD_CTRL4_SDIO0_DATA0_DRIVE_R 9
#define IOPAD_CTRL4_SDIO0_DATA1_SRC_R 7
#define IOPAD_CTRL4_SDIO0_DATA1_HYS_R 5
#define IOPAD_CTRL4_SDIO0_DATA1_DRIVE_R 1

#define IOPAD_CTRL5_SDIO0_DATA2_SRC_R 31
#define IOPAD_CTRL5_SDIO0_DATA2_HYS_R 29
#define IOPAD_CTRL5_SDIO0_DATA2_DRIVE_R 25
#define IOPAD_CTRL5_SDIO0_DATA3_SRC_R 23
#define IOPAD_CTRL5_SDIO0_DATA3_IND_R 22
#define IOPAD_CTRL5_SDIO0_DATA3_HYS_R 21
#define IOPAD_CTRL5_SDIO0_DATA3_DRIVE_R 17
#define IOPAD_CTRL5_SDIO0_DATA4_SRC_R 15
#define IOPAD_CTRL5_SDIO0_DATA4_HYS_R 13
#define IOPAD_CTRL5_SDIO0_DATA4_DRIVE_R 9
#define IOPAD_CTRL5_SDIO0_DATA5_SRC_R 7
#define IOPAD_CTRL5_SDIO0_DATA5_HYS_R 5
#define IOPAD_CTRL5_SDIO0_DATA5_DRIVE_R 1

#define IOPAD_CTRL6_SDIO0_DATA6_SRC_R 31
#define IOPAD_CTRL6_SDIO0_DATA6_HYS_R 29
#define IOPAD_CTRL6_SDIO0_DATA6_DRIVE_R 25
#define IOPAD_CTRL6_SDIO0_DATA7_SRC_R 23
#define IOPAD_CTRL6_SDIO0_DATA7_HYS_R 21
#define IOPAD_CTRL6_SDIO0_DATA7_DRIVE_R 17

void emmc_soft_reset(void)
{
	uint32_t val = 0;

	val = (BIT(IOPAD_CTRL6_SDIO0_DATA7_SRC_R) |
	       BIT(IOPAD_CTRL6_SDIO0_DATA7_HYS_R) |
	       BIT(IOPAD_CTRL6_SDIO0_DATA7_DRIVE_R) |
	       BIT(IOPAD_CTRL6_SDIO0_DATA6_SRC_R) |
	       BIT(IOPAD_CTRL6_SDIO0_DATA6_HYS_R) |
	       BIT(IOPAD_CTRL6_SDIO0_DATA6_DRIVE_R));

	mmio_write_32(ICFG_IPROC_IOPAD_CTRL_6, val);

	val = (BIT(IOPAD_CTRL5_SDIO0_DATA3_SRC_R) |
	       BIT(IOPAD_CTRL5_SDIO0_DATA3_HYS_R) |
	       BIT(IOPAD_CTRL5_SDIO0_DATA3_DRIVE_R) |
	       BIT(IOPAD_CTRL5_SDIO0_DATA4_SRC_R) |
	       BIT(IOPAD_CTRL5_SDIO0_DATA4_HYS_R) |
	       BIT(IOPAD_CTRL5_SDIO0_DATA4_DRIVE_R) |
	       BIT(IOPAD_CTRL5_SDIO0_DATA5_SRC_R) |
	       BIT(IOPAD_CTRL5_SDIO0_DATA5_HYS_R) |
	       BIT(IOPAD_CTRL5_SDIO0_DATA5_DRIVE_R));

	mmio_write_32(ICFG_IPROC_IOPAD_CTRL_5, val);

	val = (BIT(IOPAD_CTRL4_SDIO0_DATA0_SRC_R) |
	       BIT(IOPAD_CTRL4_SDIO0_DATA0_HYS_R) |
	       BIT(IOPAD_CTRL4_SDIO0_DATA0_DRIVE_R) |
	       BIT(IOPAD_CTRL4_SDIO0_DATA1_SRC_R) |
	       BIT(IOPAD_CTRL4_SDIO0_DATA1_HYS_R) |
	       BIT(IOPAD_CTRL4_SDIO0_DATA1_DRIVE_R) |
	       BIT(IOPAD_CTRL5_SDIO0_DATA2_SRC_R) |
	       BIT(IOPAD_CTRL5_SDIO0_DATA2_HYS_R) |
	       BIT(IOPAD_CTRL5_SDIO0_DATA2_DRIVE_R));

	mmio_write_32(ICFG_IPROC_IOPAD_CTRL_6, val);

	val = (BIT(IOPAD_CTRL4_SDIO0_CLK_SDCARD_SRC_R) |
	       BIT(IOPAD_CTRL4_SDIO0_CLK_SDCARD_HYS_R) |
	       BIT(IOPAD_CTRL4_SDIO0_CLK_SDCARD_DRIVE_R) |
	       BIT(IOPAD_CTRL4_SDIO0_CD_SRC_R) |
	       BIT(IOPAD_CTRL4_SDIO0_CD_HYS_R));

	/*
	 * set pull-down, clear pull-up=0
	 * bit 12: pull-down  bit 11: pull-up
	 * Note:  In emulation, this pull-down setting was not
	 * sufficient.  Board design likely requires pull down on
	 * this pin for eMMC.
	 */

	val |= BIT(IOPAD_CTRL4_SDIO0_CD_PULL_R);

	mmio_write_32(ICFG_IPROC_IOPAD_CTRL_4, val);
}
