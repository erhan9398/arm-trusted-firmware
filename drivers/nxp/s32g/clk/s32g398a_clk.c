// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2021 NXP
 */
#include <dt-bindings/clock/s32gen1-clock.h>
#include <dt-bindings/clock/s32gen1-scmi-clock.h>
#include <dt-bindings/clock/s32g3-clock.h>
#include <clk/s32gen1_clk_funcs.h>
#include <clk/s32gen1_clk_modules.h>

/* GMAC_TS_CLK */
static struct s32gen1_fixed_clock gmac_ext_ts =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk gmac_ext_ts_clk =
		S32GEN1_MODULE_CLK(gmac_ext_ts);
static struct s32gen1_mux cgm6_mux0 =
		S32GEN1_MUX_INIT(S32GEN1_CGM6, 0, 3,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_PERIPH_PLL_PHI4,
				 S32GEN1_CLK_GMAC0_EXT_TS);
static struct s32gen1_clk cgm6_mux0_clk =
		S32GEN1_MODULE_CLK(cgm6_mux0);
static struct s32gen1_cgm_div gmac_ts_div =
		S32GEN1_CGM_DIV_INIT(cgm6_mux0_clk, 0);
static struct s32gen1_clk gmac_ts_clk =
		S32GEN1_FREQ_MODULE_CLK(gmac_ts_div, 5 * MHZ, 200 * MHZ);

/* GMAC0_TX_CLK */
static struct s32gen1_fixed_clock gmac0_ext_tx =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk gmac0_ext_tx_clk =
		S32GEN1_MODULE_CLK(gmac0_ext_tx);

static struct s32gen1_fixed_clock gmac0_ext_ref =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk gmac0_ext_ref_clk =
		S32GEN1_MODULE_CLK(gmac0_ext_ref);

static struct s32gen1_mux cgm6_mux1 =
		S32GEN1_MUX_INIT(S32GEN1_CGM6, 1, 5,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_PERIPH_PLL_PHI5,
				 S32GEN1_CLK_SERDES0_LANE0_TX,
				 S32GEN1_CLK_GMAC0_EXT_TX,
				 S32GEN1_CLK_GMAC0_EXT_REF);
static struct s32gen1_clk cgm6_mux1_clk =
		S32GEN1_MODULE_CLK(cgm6_mux1);
static struct s32gen1_cgm_div gmac_tx_div =
		S32GEN1_CGM_DIV_INIT(cgm6_mux1_clk, 0);
static struct s32gen1_clk gmac_tx_clk =
		S32GEN1_FREQ_MODULE_CLK(gmac_tx_div, 2500000, 125 * MHZ);

/* GMAC0_RX_CLK */
static struct s32gen1_fixed_clock gmac0_ext_rx =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk gmac0_ext_rx_clk =
		S32GEN1_MODULE_CLK(gmac0_ext_rx);

static struct s32gen1_mux cgm6_mux2 =
		S32GEN1_MUX_INIT(S32GEN1_CGM6, 2, 4,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_GMAC0_REF_DIV,
				 S32GEN1_CLK_GMAC0_EXT_RX,
				 S32GEN1_CLK_SERDES0_LANE0_CDR);
static struct s32gen1_clk cgm6_mux2_clk =
		S32GEN1_MODULE_CLK(cgm6_mux2);
static struct s32gen1_clk gmac_rx_clk =
		S32GEN1_CHILD_CLK(cgm6_mux2_clk, 2500000, 125 * MHZ);

/* GMAC0_REF_DIV_CLK */
static struct s32gen1_mux cgm6_mux3 =
		S32GEN1_SHARED_MUX_INIT(S32GEN1_CGM6, 3, 2,
					S32GEN1_CLK_FIRC,
					S32GEN1_CLK_GMAC0_EXT_REF);
static struct s32gen1_clk cgm6_mux3_clk =
		S32GEN1_MODULE_CLK(cgm6_mux3);
static struct s32gen1_clk gmac0_ref_div_clk =
		S32GEN1_CHILD_CLK(cgm6_mux3_clk, 0, 50 * MHZ);
static struct s32gen1_clk gmac0_ref_clk =
		S32GEN1_CHILD_CLK(cgm6_mux3_clk, 0, 50 * MHZ);

static struct s32gen1_clk *s32g398a_clocks[] = {
	[ARR_CLK(S32G_CLK_MC_CGM6_MUX0)] = &cgm6_mux0_clk,
	[ARR_CLK(S32G_CLK_MC_CGM6_MUX1)] = &cgm6_mux1_clk,
	[ARR_CLK(S32G_CLK_MC_CGM6_MUX2)] = &cgm6_mux2_clk,
	[ARR_CLK(S32G_CLK_MC_CGM6_MUX3)] = &cgm6_mux3_clk,
};

static struct s32gen1_clk *s32g398a_cc_clocks[] = {
	[CC_ARR_CLK(S32GEN1_CLK_GMAC0_TX)] = &gmac_tx_clk,
	[CC_ARR_CLK(S32GEN1_CLK_GMAC0_EXT_TS)] = &gmac_ext_ts_clk,
	[CC_ARR_CLK(S32GEN1_CLK_GMAC0_TS)] = &gmac_ts_clk,
	[CC_ARR_CLK(S32GEN1_CLK_GMAC0_EXT_TX)] = &gmac0_ext_tx_clk,
	[CC_ARR_CLK(S32GEN1_CLK_GMAC0_EXT_REF)] = &gmac0_ext_ref_clk,
	[CC_ARR_CLK(S32GEN1_CLK_GMAC0_EXT_RX)] = &gmac0_ext_rx_clk,
	[CC_ARR_CLK(S32GEN1_CLK_GMAC0_RX)] = &gmac_rx_clk,
	[CC_ARR_CLK(S32GEN1_CLK_GMAC0_REF_DIV)] = &gmac0_ref_div_clk,
	[CC_ARR_CLK(S32GEN1_CLK_GMAC0_REF)] = &gmac0_ref_clk,
};

struct s32gen1_clk *s32g_get_plat_cc_clock(uint32_t id)
{
	id = s32gen1_platclk2mux(id);

	if (id >= ARRAY_SIZE(s32g398a_cc_clocks))
		return NULL;

	return s32g398a_cc_clocks[id];
}

struct s32gen1_clk *s32g_get_plat_clock(uint32_t id)
{
	if (id < S32GEN1_PLAT_CLK_ID_BASE)
		return NULL;

	id -= S32GEN1_PLAT_CLK_ID_BASE;

	if (id >= ARRAY_SIZE(s32g398a_clocks))
		return NULL;

	return s32g398a_clocks[id];
}

int cc_compound_clk_get_pid(uint32_t id, uint32_t *parent_id)
{
	if (!parent_id)
		return -EINVAL;

	switch (id) {
	case S32GEN1_SCMI_CLK_GMAC0_RX_SGMII:
	case S32GEN1_SCMI_CLK_GMAC0_RX_RGMII:
		*parent_id = S32G_CLK_MC_CGM6_MUX2;
		break;
	case S32GEN1_SCMI_CLK_GMAC0_TX_RGMII:
	case S32GEN1_SCMI_CLK_GMAC0_TX_SGMII:
		*parent_id = S32G_CLK_MC_CGM6_MUX1;
		break;
	case S32GEN1_SCMI_CLK_GMAC0_TS_RGMII:
	case S32GEN1_SCMI_CLK_GMAC0_TS_SGMII:
		*parent_id = S32G_CLK_MC_CGM6_MUX0;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
