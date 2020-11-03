// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020 NXP
 */
#include <clk/clk.h>
#include <clk/s32gen1_clk_funcs.h>
#include <dt-bindings/clock/s32gen1-clock-freq.h>
#include <dt-bindings/clock/s32gen1-clock.h>
#include <s32g_clocks.h>

#define CLK_INIT(ID)          \
{                             \
	.id = (ID),           \
	.drv = &fake_clk_dev, \
}

#define ARM_PLL_BASE_ADDR	PLL_ADDR(S32G_CORE_PLL)
#define PERIPH_PLL_BASE_ADDR	PLL_ADDR(S32G_PERIPH_PLL)
#define ACCEL_PLL_BASE_ADDR	PLL_ADDR(S32G_ACCEL_PLL)
#define DRAM_PLL_BASE_ADDR	PLL_ADDR(S32G_DDR_PLL)

#define ARM_DFS_BASE_ADDR	S32G_DFS_ADDR(S32G_CORE_DFS)
#define PERIPH_DFS_BASE_ADDR	S32G_DFS_ADDR(S32G_PERIPH_DFS)

static struct s32gen1_clk_priv s32_priv = {
	.accelpll = (void *)ACCEL_PLL_BASE_ADDR,
	.armdfs = (void *)ARM_DFS_BASE_ADDR,
	.armpll = (void *)ARM_PLL_BASE_ADDR,
	.cgm0 = (void *)MC_CGM0_BASE_ADDR,
	.cgm1 = (void *)MC_CGM1_BASE_ADDR,
	.cgm2 = (void *)MC_CGM2_BASE_ADDR,
	.cgm5 = (void *)MC_CGM5_BASE_ADDR,
	.ddrpll = (void *)DRAM_PLL_BASE_ADDR,
	.fxosc = (void *)S32G_FXOSC_BASE_ADDR,
	.mc_me = (void *)MC_ME_BASE_ADDR,
	.periphdfs = (void *)PERIPH_DFS_BASE_ADDR,
	.periphpll = (void *)PERIPH_PLL_BASE_ADDR,
	.rdc = (void *)RDC_BASE_ADDR,
	.rgm = (void *)MC_RGM_BASE_ADDR,
};

static struct clk_driver fake_clk_dev = {
	.data = &s32_priv,
};

/* A53 clocks */
static struct clk fxosc = CLK_INIT(S32GEN1_CLK_FXOSC);
static struct clk arm_pll_mux = CLK_INIT(S32GEN1_CLK_ARM_PLL_MUX);
static struct clk arm_pll_vco = CLK_INIT(S32GEN1_CLK_ARM_PLL_VCO);
static struct clk arm_pll_phi0 = CLK_INIT(S32GEN1_CLK_ARM_PLL_PHI0);
static struct clk mc_cgm1_mux0 = CLK_INIT(S32GEN1_CLK_MC_CGM1_MUX0);
static struct clk a53_clk = CLK_INIT(S32GEN1_CLK_A53_CORE);

/* XBAR clock */
static struct clk arm_dfs1 = CLK_INIT(S32GEN1_CLK_ARM_PLL_DFS1);
static struct clk mc_cgm0_mux0 = CLK_INIT(S32GEN1_CLK_MC_CGM0_MUX0);
static struct clk xbar_2x = CLK_INIT(S32GEN1_CLK_XBAR_2X);

/* LINFLEX clock */
static struct clk periph_pll_mux = CLK_INIT(S32GEN1_CLK_PERIPH_PLL_MUX);
static struct clk periph_pll_vco = CLK_INIT(S32GEN1_CLK_PERIPH_PLL_VCO);
static struct clk periph_pll_phi3 = CLK_INIT(S32GEN1_CLK_PERIPH_PLL_PHI3);
static struct clk periph_dfs3 = CLK_INIT(S32GEN1_CLK_PERIPH_PLL_DFS3);
static struct clk mc_cgm0_mux8 = CLK_INIT(S32GEN1_CLK_MC_CGM0_MUX8);
static struct clk mc_cgm0_mux14 = CLK_INIT(S32GEN1_CLK_MC_CGM0_MUX14);
static struct clk lin_baud = CLK_INIT(S32GEN1_CLK_LIN_BAUD);
static struct clk sdhc = CLK_INIT(S32GEN1_CLK_SDHC);

/* DDR clock */
static struct clk ddr_pll_mux = CLK_INIT(S32GEN1_CLK_DDR_PLL_MUX);
static struct clk ddr_pll_vco = CLK_INIT(S32GEN1_CLK_DDR_PLL_VCO);
static struct clk ddr_pll_phi0 = CLK_INIT(S32GEN1_CLK_DDR_PLL_PHI0);
static struct clk mc_cgm5_mux0 = CLK_INIT(S32GEN1_CLK_MC_CGM5_MUX0);
static struct clk ddr = CLK_INIT(S32GEN1_CLK_DDR);

static int enable_a53_clock(void)
{
	int ret;
	unsigned long rate;

	ret = s32gen1_set_parent(&arm_pll_mux, &fxosc);
	if (ret)
		return ret;

	ret = s32gen1_set_parent(&mc_cgm1_mux0, &arm_pll_phi0);
	if (ret)
		return ret;

	rate = s32gen1_set_rate(&fxosc, S32GEN1_FXOSC_FREQ);
	if (rate != S32GEN1_FXOSC_FREQ)
		return -EINVAL;

	rate = s32gen1_set_rate(&arm_pll_vco, S32GEN1_ARM_PLL_VCO_MAX_FREQ);
	if (rate != S32GEN1_ARM_PLL_VCO_MAX_FREQ)
		return -EINVAL;

	rate = s32gen1_set_rate(&a53_clk, S32GEN1_A53_MAX_FREQ);
	if (rate != S32GEN1_A53_MAX_FREQ)
		return -EINVAL;

	return s32gen1_enable(&a53_clk, 1);
}

static int enable_xbar_clock(void)
{
	int ret;
	unsigned long rate;

	ret = s32gen1_set_parent(&mc_cgm0_mux0, &arm_dfs1);
	if (ret)
		return ret;

	rate = s32gen1_set_rate(&xbar_2x, S32GEN1_XBAR_2X_FREQ);
	if (rate != S32GEN1_XBAR_2X_FREQ)
		return -EINVAL;

	return s32gen1_enable(&xbar_2x, 1);
}

static int enable_lin_clock(void)
{
	int ret;
	unsigned long rate;

	ret = s32gen1_set_parent(&periph_pll_mux, &fxosc);
	if (ret)
		return ret;

	ret = s32gen1_set_parent(&mc_cgm0_mux8, &periph_pll_phi3);
	if (ret)
		return ret;

	rate = s32gen1_set_rate(&periph_pll_vco,
				S32GEN1_PERIPH_PLL_VCO_FREQ);
	if (rate != S32GEN1_PERIPH_PLL_VCO_FREQ)
		return -EINVAL;

	rate = s32gen1_set_rate(&lin_baud, S32GEN1_LIN_BAUD_CLK_FREQ);
	if (rate != S32GEN1_LIN_BAUD_CLK_FREQ)
		return -EINVAL;

	return s32gen1_enable(&lin_baud, 1);
}

static int enable_sdhc_clock(void)
{
	int ret;
	unsigned long rate;

	ret = s32gen1_set_parent(&periph_pll_mux, &fxosc);
	if (ret)
		return ret;

	ret = s32gen1_set_parent(&mc_cgm0_mux14, &periph_dfs3);
	if (ret)
		return ret;

	rate = s32gen1_set_rate(&periph_pll_vco,
				S32GEN1_PERIPH_PLL_VCO_FREQ);
	if (rate != S32GEN1_PERIPH_PLL_VCO_FREQ)
		return -EINVAL;

	rate = s32gen1_set_rate(&periph_dfs3, S32GEN1_PERIPH_DFS3_FREQ);
	if (rate != S32GEN1_PERIPH_DFS3_FREQ)
		return -EINVAL;

	rate = s32gen1_set_rate(&sdhc, S32GEN1_SDHC_CLK_FREQ);
	if (rate != S32GEN1_SDHC_CLK_FREQ)
		return -EINVAL;

	return s32gen1_enable(&sdhc, 1);
}

static int enable_ddr_clock(void)
{
	int ret;
	unsigned long rate;

	ret = s32gen1_set_parent(&ddr_pll_mux, &fxosc);
	if (ret)
		return ret;

	ret = s32gen1_set_parent(&mc_cgm5_mux0, &ddr_pll_phi0);
	if (ret)
		return ret;

	rate = s32gen1_set_rate(&ddr_pll_vco,
				S32GEN1_DDR_PLL_VCO_FREQ);
	if (rate != S32GEN1_DDR_PLL_VCO_FREQ)
		return -EINVAL;

	rate = s32gen1_set_rate(&ddr, S32GEN1_DDR_FREQ);
	if (rate != S32GEN1_DDR_FREQ)
		return -EINVAL;

	return s32gen1_enable(&ddr, 1);
}

int s32g_plat_clock_init(bool skip_ddr_clk)
{
	int ret;

	ret = enable_a53_clock();
	if (ret)
		return ret;

	ret = enable_xbar_clock();
	if (ret)
		return ret;

	ret = enable_lin_clock();
	if (ret)
		return ret;

	ret = enable_sdhc_clock();
	if (ret)
		return ret;

	if (!skip_ddr_clk)
		return  enable_ddr_clock();

	return 0;
}
