// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020 NXP
 */
#include <clk/mc_cgm_regs.h>
#include <clk/mc_me_regs.h>
#include <clk/mc_rgm_regs.h>
#include <clk/s32gen1_clk_funcs.h>
#include <clk/s32gen1_clk_modules.h>
#include <common/debug.h>
#include <dt-bindings/clock/s32gen1-clock.h>
#include <lib/mmio.h>
#include <s32g_fp.h>

static int enable_module(struct s32gen1_clk_obj *module,
			 struct s32gen1_clk_priv *priv);

static void setup_fxosc(struct s32gen1_clk_priv *priv)
{
	void *fxosc = priv->fxosc;

	/* According to "Initialization information" chapter from
	 * S32G274A Reference Manual, "Once FXOSC is turned ON, DO NOT change
	 * any signal (on the fly) which is going to analog module input.
	 * The inputs can be changed when the analog module is OFF...When
	 * disabling the IP through Software do not change any other values in
	 * the registers for at least 4 crystal clock cycles."
	 *
	 * Just make sure that FXOSC wasn't already started by BootROM.
	 */
	uint32_t ctrl;

	if (mmio_read_32(FXOSC_CTRL(fxosc)) & FXOSC_CTRL_OSCON)
		return;

	ctrl = FXOSC_CTRL_COMP_EN;
	ctrl &= ~FXOSC_CTRL_OSC_BYP;
	ctrl |= FXOSC_CTRL_EOCV(0x1);
	ctrl |= FXOSC_CTRL_GM_SEL(0x7);
	mmio_write_32(FXOSC_CTRL(fxosc), ctrl);

	/* Switch ON the crystal oscillator. */
	mmio_write_32(FXOSC_CTRL(fxosc),
		      FXOSC_CTRL_OSCON | mmio_read_32(FXOSC_CTRL(fxosc)));

	/* Wait until the clock is stable. */
	while (!(mmio_read_32(FXOSC_STAT(fxosc)) & FXOSC_STAT_OSC_STAT))
		;
}

static void mc_me_wait_update(uint32_t partition_n, uint32_t mask,
			      struct s32gen1_clk_priv *priv)
{
	void *mc_me = priv->mc_me;
	uint32_t pupd = mmio_read_32(MC_ME_PRTN_N_PUPD(mc_me, partition_n));

	mmio_write_32(MC_ME_PRTN_N_PUPD(mc_me, partition_n), pupd | mask);
	mmio_write_32(MC_ME_CTL_KEY(mc_me), MC_ME_CTL_KEY_KEY);
	mmio_write_32(MC_ME_CTL_KEY(mc_me), MC_ME_CTL_KEY_INVERTEDKEY);

	while (mmio_read_32(MC_ME_PRTN_N_PUPD(mc_me, partition_n)) & mask)
		;
}

static void enable_partition(uint32_t partition_n,
			     struct s32gen1_clk_priv *priv)
{
	void *mc_me = priv->mc_me;
	void *rdc = priv->rdc;
	void *rgm = priv->rgm;
	uint32_t rdc_ctrl, prst, ctrl;
	uint32_t pconf = mmio_read_32(MC_ME_PRTN_N_PCONF(mc_me, partition_n));

	mmio_write_32(MC_ME_PRTN_N_PCONF(mc_me, partition_n),
		      pconf | MC_ME_PRTN_N_PCE);

	mc_me_wait_update(partition_n, MC_ME_PRTN_N_PCUD, priv);

	while (!(mmio_read_32(MC_ME_PRTN_N_STAT(mc_me, partition_n)) &
	       MC_ME_PRTN_N_PCS))
		;

	/* Unlock RDC register write */
	rdc_ctrl = mmio_read_32(RDC_RD_N_CTRL(rdc, partition_n));
	mmio_write_32(RDC_RD_N_CTRL(rdc, partition_n),
		      rdc_ctrl | RD_CTRL_UNLOCK_MASK);

	/* Enable the XBAR interface */
	rdc_ctrl = mmio_read_32(RDC_RD_N_CTRL(rdc, partition_n));
	rdc_ctrl &= ~RDC_RD_INTERCONNECT_DISABLE;
	mmio_write_32(RDC_RD_N_CTRL(rdc, partition_n), rdc_ctrl);

	/* Wait until XBAR interface enabled */
	while ((mmio_read_32(RDC_RD_N_STATUS(rdc, partition_n)) &
		RDC_RD_INTERCONNECT_DISABLE_STAT))
		;

	/* Lift reset for partition */
	prst = mmio_read_32(RGM_PRST(rgm, partition_n));
	mmio_write_32(RGM_PRST(rgm, partition_n),
		      prst & (~PRST_PERIPH_n_RST(0)));

	/* Follow steps to clear OSSE bit */
	mmio_write_32(MC_ME_PRTN_N_PCONF(mc_me, partition_n),
		      mmio_read_32(MC_ME_PRTN_N_PCONF(mc_me, partition_n)) &
		      ~MC_ME_PRTN_N_OSSE);

	mc_me_wait_update(partition_n, MC_ME_PRTN_N_OSSUD, priv);

	while (mmio_read_32(MC_ME_PRTN_N_STAT(mc_me, partition_n)) &
			MC_ME_PRTN_N_OSSS)
		;

	while (mmio_read_32(RGM_PSTAT(rgm, partition_n)) &
			PSTAT_PERIPH_n_STAT(0))
		;

	/* Lock RDC register write */
	ctrl = mmio_read_32(RDC_RD_N_CTRL(rdc, partition_n));
	mmio_write_32(RDC_RD_N_CTRL(rdc, partition_n),
		      ctrl & ~RD_CTRL_UNLOCK_MASK);
}

static void enable_part_cofb(uint32_t partition_n, uint32_t block,
			     struct s32gen1_clk_priv *priv,
			     bool check_status)
{
	void *mc_me = priv->mc_me;
	uint32_t block_mask = MC_ME_PRTN_N_REQ(block);
	uint32_t part_status;
	uint32_t clken, pconf, cofb_stat_addr;

	part_status = mmio_read_32(MC_ME_PRTN_N_STAT(mc_me, partition_n));

	/* Enable a partition only if it's disabled */
	if (!(MC_ME_PRTN_N_PCS & part_status))
		enable_partition(partition_n, priv);

	clken = mmio_read_32(MC_ME_PRTN_N_COFB0_CLKEN(mc_me, partition_n));
	mmio_write_32(MC_ME_PRTN_N_COFB0_CLKEN(mc_me, partition_n),
		      clken | block_mask);

	pconf = mmio_read_32(MC_ME_PRTN_N_PCONF(mc_me, partition_n));
	mmio_write_32(MC_ME_PRTN_N_PCONF(mc_me, partition_n),
		      pconf | MC_ME_PRTN_N_PCE);

	mc_me_wait_update(partition_n, MC_ME_PRTN_N_PCUD, priv);

	cofb_stat_addr = MC_ME_PRTN_N_COFB0_STAT(mc_me, partition_n);
	if (check_status)
		while (!(mmio_read_32(cofb_stat_addr) &  block_mask))
			;
}

static int enable_clock(struct s32gen1_clk_obj *module,
			struct s32gen1_clk_priv *priv)
{
	struct s32gen1_clk *clk = obj2clk(module);

	if (clk->module)
		return enable_module(clk->module, priv);

	if (clk->pclock)
		return enable_clock(&clk->pclock->desc, priv);

	return -EINVAL;
}

static int enable_part_block(struct s32gen1_clk_obj *module,
			     struct s32gen1_clk_priv *priv)
{
	struct s32gen1_part_block *block = obj2partblock(module);
	uint32_t cofb;

	switch (block->block) {
	case s32gen1_part_core0 ... s32gen1_part_core3:
		/* Nothing to do here */
		break;
	case s32gen1_part_block0 ... s32gen1_part_block15:
		cofb = block->block - s32gen1_part_block0;
		enable_part_cofb(block->partition, cofb,
				 priv, block->status);
		break;
	default:
		ERROR("Unknown partition block type: %d\n",
		       block->block);
		return -EINVAL;
	};

	return enable_module(block->parent, priv);
}

static uint32_t s32gen1_platclk2mux(uint32_t clk_id)
{
	return clk_id - S32GEN1_CLK_ID_BASE;
}

static int cgm_mux_clk_config(void *cgm_addr, uint32_t mux, uint32_t source)
{
	uint32_t css, csc;

	css = mmio_read_32(CGM_MUXn_CSS(cgm_addr, mux));

	/* Platform ID translation */
	source = s32gen1_platclk2mux(source);

	/* Already configured */
	if (MC_CGM_MUXn_CSS_SELSTAT(css) == source &&
	    MC_CGM_MUXn_CSS_SWTRG(css) == MC_CGM_MUXn_CSS_SWTRG_SUCCESS &&
	    !(css & MC_CGM_MUXn_CSS_SWIP))
		return 0;

	/* Ongoing clock switch? */
	while (mmio_read_32(CGM_MUXn_CSS(cgm_addr, mux)) & MC_CGM_MUXn_CSS_SWIP)
		;

	csc = mmio_read_32(CGM_MUXn_CSC(cgm_addr, mux));
	/* Clear previous source. */

	csc &= ~(MC_CGM_MUXn_CSC_SELCTL_MASK);

	/* Select the clock source and trigger the clock switch. */
	mmio_write_32(CGM_MUXn_CSC(cgm_addr, mux),
		      csc | MC_CGM_MUXn_CSC_SELCTL(source) |
		      MC_CGM_MUXn_CSC_CLK_SW);

	/* Wait for configuration bit to auto-clear. */
	while (mmio_read_32(CGM_MUXn_CSC(cgm_addr, mux)) &
		MC_CGM_MUXn_CSC_CLK_SW)
		;

	/* Is the clock switch completed? */
	while (mmio_read_32(CGM_MUXn_CSS(cgm_addr, mux)) & MC_CGM_MUXn_CSS_SWIP)
		;

	/*
	 * Check if the switch succeeded.
	 * Check switch trigger cause and the source.
	 */
	css = mmio_read_32(CGM_MUXn_CSS(cgm_addr, mux));
	if ((MC_CGM_MUXn_CSS_SWTRG(css) == MC_CGM_MUXn_CSS_SWTRG_SUCCESS) &&
	    (MC_CGM_MUXn_CSS_SELSTAT(css) == source))
		return 0;

	ERROR("Failed to change the clock source of mux %d to %d (CGM = %p)\n",
	       mux, source, cgm_addr);

	return -EINVAL;
}

static int enable_cgm_mux(struct s32gen1_mux *mux,
			  struct s32gen1_clk_priv *priv)
{
	void *module_addr;

	module_addr = get_base_addr(mux->module, priv);

	if (!module_addr) {
		ERROR("Failed to get the base address of the module %d\n",
		       mux->module);
		return -EINVAL;
	}

	return cgm_mux_clk_config(module_addr, mux->index, mux->source_id);
}

static int enable_mux(struct s32gen1_clk_obj *module,
		      struct s32gen1_clk_priv *priv)
{
	int ret;
	struct s32gen1_mux *mux = obj2mux(module);
	struct s32gen1_clk *clk = get_clock(mux->source_id);

	if (!clk) {
		ERROR("Invalid parent (%u) for mux %u\n",
		      mux->source_id, mux->index);
		return -EINVAL;
	}

	ret = enable_module(&clk->desc, priv);
	if (ret)
		return ret;

	switch (mux->module) {
	/* PLL mux will be enabled by PLL setup */
	case S32GEN1_ACCEL_PLL:
	case S32GEN1_ARM_PLL:
	case S32GEN1_DDR_PLL:
	case S32GEN1_PERIPH_PLL:
		return 0;
	case S32GEN1_CGM0:
	case S32GEN1_CGM1:
	case S32GEN1_CGM2:
	case S32GEN1_CGM5:
		return enable_cgm_mux(mux, priv);
	default:
		ERROR("Unknown mux parent type: %d\n", mux->module);
		return -EINVAL;
	};

	return -EINVAL;
}

static void cgm_mux_div_config(void *cgm_addr, uint32_t mux,
			       uint32_t dc, uint32_t div_index)
{
	uint32_t updstat;
	uint32_t dc_val = mmio_read_32(CGM_MUXn_DCm(cgm_addr, mux, div_index));

	dc_val &= (MC_CGM_MUXn_DCm_DIV_MASK | MC_CGM_MUXn_DCm_DE);

	if (dc_val == (MC_CGM_MUXn_DCm_DE | MC_CGM_MUXn_DCm_DIV(dc)))
		return;

	/* Set the divider */
	mmio_write_32(CGM_MUXn_DCm(cgm_addr, mux, div_index),
		      MC_CGM_MUXn_DCm_DE | MC_CGM_MUXn_DCm_DIV(dc));

	/* Wait for divider gets updated */
	do {
		updstat = mmio_read_32(CGM_MUXn_DIV_UPD_STAT(cgm_addr, mux));
	} while (MC_CGM_MUXn_DIV_UPD_STAT_DIVSTAT(updstat));
}

static int enable_cgm_div(struct s32gen1_clk_obj *module,
			  struct s32gen1_clk_priv *priv)
{
	struct s32gen1_cgm_div *div = obj2cgmdiv(module);
	struct s32gen1_mux *mux;
	void *cgm_addr;
	int ret;
	uint64_t pfreq;
	uint32_t dc;

	if (!div->parent) {
		ERROR("Failed to identify CGM divider's parent\n");
		return -EINVAL;
	}

	if (!div->freq) {
		ERROR("The frequency of the divider %d is not set\n",
		       div->index);
		return -EINVAL;
	}

	ret = enable_module(div->parent, priv);
	if (ret)
		return ret;

	pfreq = get_module_rate(div->parent, priv);
	if (!pfreq) {
		ERROR("Failed to get the frequency of CGM MUX\n");
		return -EINVAL;
	}

	mux = get_cgm_div_mux(div);
	if (!mux)
		return -EINVAL;

	dc = (uint32_t)fp2u(fp_div(u2fp(pfreq), u2fp(div->freq)));
	if (fp2u(fp_div(u2fp(pfreq), u2fp(dc))) != div->freq) {
		ERROR("Cannot set CGM divider (mux:%d, div:%d) for input = %lu & output = %lu\n",
		      mux->index, div->index, (unsigned long)pfreq, div->freq);
		return -EINVAL;
	}

	cgm_addr = get_base_addr(mux->module, priv);
	if (!cgm_addr) {
		ERROR("Failed to get CGM base address of the module %d\n",
		       mux->module);
	}

	cgm_mux_div_config(cgm_addr, mux->index,
			   dc - 1, div->index);
	return ret;
}

static int get_dfs_mfi_mfn(unsigned long dfs_freq, struct s32gen1_dfs_div *div,
			   uint32_t *mfi, uint32_t *mfn)
{
	struct fp_data div_freq, factor;

	unsigned long in = dfs_freq;
	unsigned long out = div->freq;

	/**
	 * factor = IN / OUT / 2
	 * MFI = integer(factor)
	 * MFN = (factor - MFI) * 36
	 */
	factor = fp_div(u2fp(in), u2fp(out));
	factor = fp_div(factor, u2fp(2));
	*mfi = fp2u(factor);
	*mfn = fp2u(fp_mul(fp_sub(factor, u2fp(*mfi)), u2fp(36)));

	/* div_freq = in / (2 * (*mfi + *mfn / 36.0)) */
	div_freq = fp_add(u2fp(*mfi), fp_div(u2fp(*mfn), u2fp(36)));
	div_freq = fp_mul(u2fp(2), div_freq);
	div_freq = fp_div(u2fp(in), div_freq);

	if (fp2u(div_freq) != div->freq) {
		ERROR("Failed to find MFI and MFN settings for DFS DIV freq %lu\n",
		       div->freq);
		return -EINVAL;
	}

	return 0;
}

static int init_dfs_port(void *dfs_addr, uint32_t port,
			 uint32_t mfi, uint32_t mfn)
{
	uint32_t portsr, portreset, portolsr;
	bool init_dfs;
	uint32_t mask, old_mfi, old_mfn;
	uint32_t dvport = mmio_read_32(DFS_DVPORTn(dfs_addr, port));

	old_mfi = DFS_DVPORTn_MFI(dvport);
	old_mfn = DFS_DVPORTn_MFN(dvport);

	portsr = mmio_read_32(DFS_PORTSR(dfs_addr));
	portolsr = mmio_read_32(DFS_PORTOLSR(dfs_addr));

	/* Skip configuration if it's not needed */
	if (portsr & BIT(port) && !(portolsr & BIT(port)) &&
	    mfi == old_mfi && mfn == old_mfn)
		return 0;

	init_dfs = (!portsr);

	if (init_dfs)
		mask = DFS_PORTRESET_PORTRESET_MAXVAL;
	else
		mask = DFS_PORTRESET_PORTRESET_SET(BIT(port));

	mmio_write_32(DFS_PORTOLSR(dfs_addr), mask);
	mmio_write_32(DFS_PORTRESET(dfs_addr), mask);

	while (mmio_read_32(DFS_PORTSR(dfs_addr)) & mask)
		;

	if (init_dfs)
		mmio_write_32(DFS_CTL(dfs_addr), DFS_CTL_RESET);

	mmio_write_32(DFS_DVPORTn(dfs_addr, port),
		      DFS_DVPORTn_MFI_SET(mfi) | DFS_DVPORTn_MFN_SET(mfn));

	if (init_dfs)
		/* DFS clk enable programming */
		mmio_write_32(DFS_CTL(dfs_addr), ~(uint32_t)DFS_CTL_RESET);

	portreset = mmio_read_32(DFS_PORTRESET(dfs_addr));
	portreset &= ~BIT(port);
	mmio_write_32(DFS_PORTRESET(dfs_addr), portreset);

	while ((mmio_read_32(DFS_PORTSR(dfs_addr)) & BIT(port)) != BIT(port))
		;

	portolsr = mmio_read_32(DFS_PORTOLSR(dfs_addr));
	if (portolsr & DFS_PORTOLSR_LOL(port)) {
		ERROR("Failed to lock DFS divider\n");
		return -EINVAL;
	}

	return 0;
}

static int enable_dfs_div(struct s32gen1_clk_obj *module,
			  struct s32gen1_clk_priv *priv)
{
	int ret;
	struct s32gen1_dfs_div *div = obj2dfsdiv(module);
	void *dfs_addr;
	struct s32gen1_dfs *dfs;
	uint32_t mfi, mfn;
	uint32_t ctl;
	unsigned long dfs_freq;

	ret = enable_module(div->parent, priv);
	if (ret)
		return ret;

	dfs = get_div_dfs(div);
	if (!dfs)
		return -EINVAL;

	if (!dfs->source) {
		ERROR("Failed to identify DFS divider's parent\n");
		return -EINVAL;
	}

	dfs_addr = get_base_addr(dfs->instance, priv);
	if (!dfs_addr)
		return -EINVAL;

	ctl = mmio_read_32(DFS_CTL(dfs_addr));

	/* Enabled DFS */
	if (!(ctl & DFS_CTL_RESET))
		dfs_freq = get_module_rate(&dfs->desc, priv);
	else
		dfs_freq = get_module_rate(dfs->source, priv);

	if (!dfs_freq) {
		ERROR("Failed to get DFS's freqeuncy\n");
		return -EINVAL;
	}

	ret = get_dfs_mfi_mfn(dfs_freq, div, &mfi, &mfn);
	if (ret)
		return -EINVAL;

	return init_dfs_port(dfs_addr, div->index, mfi, mfn);
}

static int enable_dfs(struct s32gen1_clk_obj *module,
		      struct s32gen1_clk_priv *priv)
{
	struct s32gen1_dfs *dfs = obj2dfs(module);

	if (!dfs->source) {
		ERROR("Failed to identify DFS's parent\n");
		return -EINVAL;
	}

	return enable_module(dfs->source, priv);
}

static bool is_pll_enabled(void *pll_base)
{
	uint32_t pllcr, pllsr;

	pllcr = mmio_read_32(PLLDIG_PLLCR(pll_base));
	pllsr = mmio_read_32(PLLDIG_PLLSR(pll_base));

	/* Enabled and locked PLL */
	return !(pllcr & PLLDIG_PLLCR_PLLPD) && (pllsr & PLLDIG_PLLSR_LOCK);
}

int get_pll_mfi_mfn(unsigned long pll_vco, unsigned long ref_freq,
		    uint32_t *mfi, uint32_t *mfn)

{
	struct fp_data dmfn, vco;

	/* FRAC-N mode */
	*mfi = (pll_vco / ref_freq);

	/* dmfn = (double)(pll_vco % ref_freq) / ref_freq * 18432.0 */
	dmfn = fp_div(u2fp(pll_vco % ref_freq), u2fp(ref_freq));
	dmfn = fp_mul(dmfn, u2fp(18432));

	*mfn = (uint32_t)fp2u(dmfn);

	/* Round MFN value */
	if (fp_sub(dmfn, u2fp(*mfn)).val >= FP_PRECISION / 2)
		*mfn += 1;

	/* vco = ref_freq * (*mfi + (double)*mfn / 18432.0) */
	vco = fp_div(u2fp(*mfn), u2fp(18432));
	vco = fp_add(u2fp(*mfi), vco);
	vco = fp_mul(u2fp(ref_freq), vco);

	if (fp2u(vco) != pll_vco) {
		ERROR("Failed to find MFI and MFN settings for PLL freq %lu\n",
		       pll_vco);
		return -EINVAL;
	}

	return 0;
}

static void disable_pll_hw(void *pll_addr)
{
	mmio_write_32(PLLDIG_PLLCR(pll_addr), PLLDIG_PLLCR_PLLPD);
}

static void enable_pll_hw(void *pll_addr)
{
	/* Enable the PLL. */
	mmio_write_32(PLLDIG_PLLCR(pll_addr), 0x0);

	/* Poll until PLL acquires lock. */
	while (!(mmio_read_32(PLLDIG_PLLSR(pll_addr)) & PLLDIG_PLLSR_LOCK))
		;
}

static uint32_t get_enabled_odivs(void *pll_addr, uint32_t ndivs)
{
	uint32_t i;
	uint32_t mask = 0;
	uint32_t pllodiv;

	for (i = 0; i < ndivs; i++) {
		pllodiv = mmio_read_32(PLLDIG_PLLODIV(pll_addr, i));
		if (pllodiv & PLLDIG_PLLODIV_DE)
			mask |= BIT(i);
	}

	return mask;
}

static void disable_odiv(void *pll_addr, uint32_t div_index)
{
	uint32_t pllodiv = mmio_read_32(PLLDIG_PLLODIV(pll_addr, div_index));

	mmio_write_32(PLLDIG_PLLODIV(pll_addr, div_index),
		      pllodiv & ~PLLDIG_PLLODIV_DE);
}

static void enable_odiv(void *pll_addr, uint32_t div_index)
{
	uint32_t pllodiv = mmio_read_32(PLLDIG_PLLODIV(pll_addr, div_index));

	mmio_write_32(PLLDIG_PLLODIV(pll_addr, div_index),
		      pllodiv | PLLDIG_PLLODIV_DE);
}

static void disable_odivs(void *pll_addr, uint32_t ndivs)
{
	uint32_t i;

	for (i = 0; i < ndivs; i++)
		disable_odiv(pll_addr, i);
}

static void enable_odivs(void *pll_addr, uint32_t ndivs, uint32_t mask)
{
	uint32_t i;

	for (i = 0; i < ndivs; i++) {
		if (mask & BIT(i))
			enable_odiv(pll_addr, i);
	}
}

static int adjust_odiv_settings(struct s32gen1_pll *pll, void *pll_addr,
				struct s32gen1_clk_priv *priv,
				uint32_t odivs_mask, unsigned long old_vco)
{
	uint32_t i, pllodiv, div;
	struct fp_data old_odiv_freq, odiv_freq;
	int ret = 0;

	if (!old_vco)
		return 0;

	for (i = 0; i < pll->ndividers; i++) {
		if (!(odivs_mask & BIT(i)))
			continue;

		pllodiv = mmio_read_32(PLLDIG_PLLODIV(pll_addr, i));

		div = PLLDIG_PLLODIV_DIV(pllodiv);
		old_odiv_freq = fp_div(u2fp(old_vco), u2fp(div + 1));

		div = (uint32_t)fp2u(fp_div(u2fp(pll->vco_freq),
					    old_odiv_freq));
		odiv_freq = fp_div(u2fp(pll->vco_freq), u2fp(div));

		if (old_odiv_freq.val != odiv_freq.val) {
			ERROR("Failed to adjust ODIV %d to match previous frequency\n",
			      i);
			ERROR("Previous freq: %llu Nearest freq: %llu\n",
			      old_odiv_freq.val, odiv_freq.val);
		}

		pllodiv = PLLDIG_PLLODIV_DIV_SET(div - 1);
		mmio_write_32(PLLDIG_PLLODIV(pll_addr, i), pllodiv);
	}

	return ret;
}

static int clk2pllclk(uint32_t clk_id, uint32_t *pll_clk_id)
{
	switch (clk_id) {
	case S32GEN1_CLK_FIRC:
		*pll_clk_id = 0;
		return 0;
	case S32GEN1_CLK_FXOSC:
		*pll_clk_id = 1;
		return 0;
	};

	return -EINVAL;
}

int pllclk2clk(uint32_t pll_clk_id, uint32_t *clk_id)
{
	switch (pll_clk_id) {
	case 0:
		*clk_id = S32GEN1_CLK_FIRC;
		return 0;
	case 1:
		*clk_id = S32GEN1_CLK_FXOSC;
		return 0;
	};

	return -EINVAL;
}

static int program_pll(struct s32gen1_pll *pll, void *pll_addr,
		       struct s32gen1_clk_priv *priv, uint32_t clk_src)
{
	unsigned long sfreq;
	struct s32gen1_clk *sclk = get_clock(clk_src);
	uint32_t rdiv = 1, mfi, mfn;
	int ret;
	uint32_t odivs_mask;
	unsigned long old_vco;

	if (!sclk)
		return -EINVAL;

	sfreq = get_module_rate(&sclk->desc, priv);
	if (!sfreq)
		return -EINVAL;

	ret = get_pll_mfi_mfn(pll->vco_freq, sfreq, &mfi, &mfn);
	if (ret)
		return -EINVAL;

	if (clk2pllclk(clk_src, &clk_src)) {
		ERROR("Failed to translate PLL clock\n");
		return -EINVAL;
	}

	odivs_mask = get_enabled_odivs(pll_addr, pll->ndividers);

	old_vco = get_module_rate(&pll->desc, priv);

	/* Disable ODIVs*/
	disable_odivs(pll_addr, pll->ndividers);

	/* Disable PLL */
	disable_pll_hw(pll_addr);

	/* Program PLLCLKMUX */
	mmio_write_32(PLLDIG_PLLCLKMUX(pll_addr), clk_src);

	/* Program VCO */
	mmio_write_32(PLLDIG_PLLDV(pll_addr),
		      PLLDIG_PLLDV_RDIV_SET(rdiv) | PLLDIG_PLLDV_MFI(mfi));
	mmio_write_32(PLLDIG_PLLFD(pll_addr),
		      PLLDIG_PLLFD_MFN_SET(mfn) | PLLDIG_PLLFD_SMDEN);

	ret = adjust_odiv_settings(pll, pll_addr, priv, odivs_mask, old_vco);

	enable_pll_hw(pll_addr);

	/* Enable out dividers */
	enable_odivs(pll_addr, pll->ndividers, odivs_mask);

	return ret;
}

static struct s32gen1_mux *get_pll_mux(struct s32gen1_pll *pll)
{
	struct s32gen1_clk_obj *source = pll->source;
	struct s32gen1_clk *clk;

	if (!source) {
		ERROR("Failed to identify PLL's parent\n");
		return NULL;
	}

	if (source->type != s32gen1_clk_t) {
		ERROR("The parent of the PLL isn't a clock\n");
		return NULL;
	}

	clk = obj2clk(source);

	if (!clk->module) {
		ERROR("The clock isn't connected to a module\n");
		return NULL;
	}

	source = clk->module;

	if (source->type != s32gen1_mux_t &&
	    source->type != s32gen1_shared_mux_t) {
		ERROR("The parent of the PLL isn't a MUX\n");
		return NULL;
	}

	return obj2mux(source);
}

static int enable_pll(struct s32gen1_clk_obj *module,
		      struct s32gen1_clk_priv *priv)
{
	int ret;
	struct s32gen1_pll *pll = obj2pll(module);
	struct s32gen1_mux *mux;
	void *pll_addr;
	uint32_t clk_src;

	mux = get_pll_mux(pll);
	if (!mux)
		return -EINVAL;

	if (pll->instance != mux->module) {
		ERROR("MUX type is not in sync with PLL ID\n");
		return -EINVAL;
	}

	/* Enable MUX & OSC */
	ret = enable_module(pll->source, priv);
	if (ret)
		return ret;

	pll_addr = get_base_addr(pll->instance, priv);
	if (!pll_addr) {
		ERROR("Failed to detect PLL instance\n");
		return -EINVAL;
	}

	clk_src = mmio_read_32(PLLDIG_PLLCLKMUX(pll_addr));
	if (pllclk2clk(clk_src, &clk_src)) {
		ERROR("Failed to get PLL clock id\n");
		return -EINVAL;
	}

	if (clk_src == mux->source_id &&
	    is_pll_enabled(pll_addr) &&
	    get_module_rate(module, priv) == pll->vco_freq) {
		return 0;
	}

	return program_pll(pll, pll_addr, priv, mux->source_id);
}

static void config_pll_out_div(void *pll_addr, uint32_t div_index, uint32_t dc)
{
	uint32_t pllodiv;
	uint32_t div;

	pllodiv = mmio_read_32(PLLDIG_PLLODIV(pll_addr, div_index));
	div = PLLDIG_PLLODIV_DIV(pllodiv);

	if (div + 1 == dc && pllodiv & PLLDIG_PLLODIV_DE)
		return;

	if (pllodiv & PLLDIG_PLLODIV_DE)
		disable_odiv(pll_addr, div_index);

	pllodiv = PLLDIG_PLLODIV_DIV_SET(dc - 1);
	mmio_write_32(PLLDIG_PLLODIV(pll_addr, div_index), pllodiv);

	enable_odiv(pll_addr, div_index);
}

static int enable_pll_div(struct s32gen1_clk_obj *module,
			  struct s32gen1_clk_priv *priv)
{
	int ret;
	struct s32gen1_pll_out_div *div = obj2plldiv(module);
	struct s32gen1_clk_obj *parent = div->parent;
	struct s32gen1_pll *pll;
	uint64_t pfreq;
	uint32_t dc;
	void *pll_addr;

	ret = enable_module(parent, priv);
	if (ret)
		return ret;

	pll = get_div_pll(div);
	if (!pll) {
		ERROR("The parent of the PLL DIV is invalid\n");
		return 0;
	}

	pll_addr = get_base_addr(pll->instance, priv);
	if (!pll_addr) {
		ERROR("Failed to detect PLL instance\n");
		return -EINVAL;
	}

	pfreq = get_module_rate(parent, priv);
	if (!pfreq) {
		ERROR("Failed to get the frequency of PLL\n");
		return -EINVAL;
	}

	dc = fp2u(fp_div(u2fp(pfreq), u2fp(div->freq)));
	if (fp2u(fp_div(u2fp(pfreq), u2fp(dc))) != div->freq) {
		ERROR("Cannot set PLL divider for input = %lu & output = %lu\n",
		       (unsigned long)pfreq, div->freq);
		return -EINVAL;
	}

	config_pll_out_div(pll_addr, div->index, dc);

	return 0;
}

static int enable_osc(struct s32gen1_clk_obj *module,
		      struct s32gen1_clk_priv *priv)
{
	struct s32gen1_osc *osc = obj2osc(module);

	switch (osc->source) {
	/* FIRC and SIRC oscillators are enabled by default */
	case S32GEN1_FIRC:
	case S32GEN1_SIRC:
		return 0;
	case S32GEN1_FXOSC:
		setup_fxosc(priv);
		return 0;
	default:
		ERROR("Invalid oscillator %d\n", osc->source);
		return -EINVAL;
	};

	return -EINVAL;
}

static int enable_fixed_div(struct s32gen1_clk_obj *module,
			    struct s32gen1_clk_priv *priv)
{
	struct s32gen1_fixed_div *pll = obj2fixeddiv(module);

	return enable_module(pll->parent, priv);
}

static int enable_module(struct s32gen1_clk_obj *module,
			 struct s32gen1_clk_priv *priv)
{
	switch (module->type) {
	case s32gen1_clk_t:
		return enable_clock(module, priv);
	case s32gen1_part_block_t:
		return enable_part_block(module, priv);
	case s32gen1_shared_mux_t:
	case s32gen1_mux_t:
		return enable_mux(module, priv);
	case s32gen1_cgm_div_t:
		return enable_cgm_div(module, priv);
	case s32gen1_dfs_div_t:
		return enable_dfs_div(module, priv);
	case s32gen1_dfs_t:
		return enable_dfs(module, priv);
	case s32gen1_pll_t:
		return enable_pll(module, priv);
	case s32gen1_osc_t:
		return enable_osc(module, priv);
	case s32gen1_fixed_clk_t:
		return 0;
	case s32gen1_fixed_div_t:
		return enable_fixed_div(module, priv);
	case s32gen1_pll_out_div_t:
		return enable_pll_div(module, priv);
	default:
		ERROR("Undefined module type: %d\n", module->type);
		return -EINVAL;
	};

	return -EINVAL;
}

int s32gen1_enable(struct clk *c, int enable)
{
	int ret;
	struct s32gen1_clk *clk;
	struct s32gen1_clk_priv *priv;

	if (!c)
		return -EINVAL;

	priv = s32gen1_get_clk_priv(c);

	clk = get_clock(c->id);
	if (!clk) {
		ERROR("Clock %u is not part of the clcok tree\n", c->id);
		return 0;
	}

	ret = enable_module(&clk->desc, priv);
	if (ret)
		ERROR("Failed to enable clock: %u\n", c->id);

	return ret;
}
