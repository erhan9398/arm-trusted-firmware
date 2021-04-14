/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020-2021 NXP
 */
#ifndef S32GEN1_CLOCK_FREQ_H
#define S32GEN1_CLOCK_FREQ_H

#define MHZ					(1000000UL)

/* Frequencies */
#define S32GEN1_PERIPH_PLL_VCO_FREQ		(2000 * MHZ)
#define S32GEN1_FXOSC_FREQ			(40 * MHZ)
#define S32GEN1_XBAR_2X_FREQ			(800 * MHZ)
#define S32GEN1_LIN_BAUD_CLK_FREQ		(125 * MHZ)
#define S32GEN1_SDHC_CLK_FREQ			(400 * MHZ)

#define S32GEN1_A53_MAX_FREQ			(1000 * MHZ)
#define S32GEN1_ARM_PLL_VCO_MAX_FREQ		(2000 * MHZ)
#define S32GEN1_ARM_PLL_PHI0_MAX_FREQ		(1000 * MHZ)
#define S32GEN1_PERIPH_PLL_PHI0_MIN_FREQ	(100 * MHZ)
#define S32GEN1_PERIPH_PLL_PHI2_MIN_FREQ	(40 * MHZ)
#define S32GEN1_PERIPH_DFS3_FREQ		(800 * MHZ)
#define S32GEN1_QSPI_MAX_FREQ			(200 * MHZ)
#define S32GEN1_DDR_PLL_VCO_FREQ		(1333333333UL)
#define S32GEN1_DDR_FREQ			(666666666UL)
#define S32G274A_REV2_DDR_PLL_VCO_FREQ		(1600 * MHZ)
#define S32G274A_REV2_DDR_FREQ			(800 * MHZ)
#define S32G274A_REV2_QSPI_FREQ			(200 * MHZ)
#define S32G274A_QSPI_FREQ			(133333333)

#endif
