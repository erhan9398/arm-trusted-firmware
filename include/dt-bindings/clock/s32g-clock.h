/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020-2021 NXP
 */

#ifndef __DT_BINDINGS_CLOCK_S32G_H
#define __DT_BINDINGS_CLOCK_S32G_H

#include <dt-bindings/clock/s32gen1-clock.h>

#define S32G_CLK_ACCEL_PLL_PHI0		S32GEN1_CC_CLK(32)
#define S32G_CLK_ACCEL_PLL_PHI1		S32GEN1_CC_CLK(33)
#define S32G_CLK_SERDES0_LANE1_TX		S32GEN1_CC_CLK(46)
#define S32G_CLK_SERDES0_LANE1_CDR		S32GEN1_CC_CLK(47)

#define S32G_CLK_PFE_MAC0_EXT_TX		S32GEN1_CC_CLK(48)
#define S32G_CLK_PFE_MAC0_EXT_RX		S32GEN1_CC_CLK(49)
#define S32G_CLK_PFE_MAC0_EXT_REF		S32GEN1_CC_CLK(50)
#define S32G_CLK_PFE_MAC1_EXT_TX		S32GEN1_CC_CLK(51)
#define S32G_CLK_PFE_MAC1_EXT_RX		S32GEN1_CC_CLK(52)
#define S32G_CLK_PFE_MAC1_EXT_REF		S32GEN1_CC_CLK(53)
#define S32G_CLK_PFE_MAC2_EXT_TX		S32GEN1_CC_CLK(54)
#define S32G_CLK_PFE_MAC2_EXT_RX		S32GEN1_CC_CLK(55)
#define S32G_CLK_PFE_MAC2_EXT_REF		S32GEN1_CC_CLK(56)

#define S32G_CLK_SERDES1_LANE0_TX		S32GEN1_CC_CLK(57)
#define S32G_CLK_SERDES1_LANE0_CDR		S32GEN1_CC_CLK(58)

#define S32G_CLK_PFE_MAC0_REF_DIV		S32GEN1_CC_CLK(59)
#define S32G_CLK_PFE_MAC1_REF_DIV		S32GEN1_CC_CLK(60)
#define S32G_CLK_PFE_MAC2_REF_DIV		S32GEN1_CC_CLK(61)
#define S32G_CLK_SERDES1_LANE1_TX		S32GEN1_CC_CLK(62)

#define S32G_CLK_SERDES1_LANE1_CDR		S32GEN1_CC_CLK(63)

#define S32G_CLK(N)			S32GEN1_PLAT_CLK(N)
#define S32G_CLK_INDEX(N)		S32GEN1_PLAT_ARRAY_INDEX(N)

/* PFE_MAC0 */
#define S32G_CLK_PFE_MAC0_TX_DIV	S32G_CLK(0)
#define S32G_CLK_PFE_MAC0_RX	S32G_CLK(1)

/* PFE_MAC1 */
#define S32G_CLK_PFE_MAC1_TX	S32G_CLK(2)
#define S32G_CLK_PFE_MAC1_RX	S32G_CLK(3)

/* PFE_MAC2 */
#define S32G_CLK_PFE_MAC2_TX	S32G_CLK(4)
#define S32G_CLK_PFE_MAC2_RX	S32G_CLK(5)

#define S32G_CLK_PFE_SYS		S32G_CLK(6)
#define S32G_CLK_PFE_PE		S32G_CLK(7)

#define S32G_CLK_MC_CGM2_MUX0	S32G_CLK(8)
#define S32G_CLK_MC_CGM2_MUX1	S32G_CLK(9)
#define S32G_CLK_MC_CGM2_MUX2	S32G_CLK(10)
#define S32G_CLK_MC_CGM2_MUX3	S32G_CLK(11)
#define S32G_CLK_MC_CGM2_MUX4	S32G_CLK(12)
#define S32G_CLK_MC_CGM2_MUX5	S32G_CLK(13)
#define S32G_CLK_MC_CGM2_MUX6	S32G_CLK(14)
#define S32G_CLK_MC_CGM2_MUX7	S32G_CLK(15)
#define S32G_CLK_MC_CGM2_MUX8	S32G_CLK(16)
#define S32G_CLK_MC_CGM2_MUX9	S32G_CLK(17)

#endif /* __DT_BINDINGS_CLOCK_S32G_H */
