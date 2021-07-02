/*
 * Copyright 2019-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <utils_def.h>
#include <lib/mmio.h>
#include <common/debug.h>
#include "s32g_mc_me.h"
#include "s32g_mc_rgm.h"

/* Apply changes to MC_ME partitions */
static void mc_me_apply_hw_changes(void)
{
	mmio_write_32(S32G_MC_ME_CTL_KEY, S32G_MC_ME_CTL_KEY_KEY);
	mmio_write_32(S32G_MC_ME_CTL_KEY, S32G_MC_ME_CTL_KEY_INVERTEDKEY);
}

/*
 * PART<n>_CORE<m> register accessors
 */

static void mc_me_part_core_addr_write(uintptr_t addr, uint32_t part,
				       uint32_t core)
{
	uint32_t addr_lo;

	addr_lo = (uint32_t)(addr & 0xFFFFFFFC);
	mmio_write_32(S32G_MC_ME_PRTN_N_CORE_M_ADDR(part, core), addr_lo);
}

static void mc_me_part_core_pconf_write_cce(uint32_t cce_bit, uint32_t p,
					    uint32_t c)
{
	uint32_t pconf;

	pconf = mmio_read_32(S32G_MC_ME_PRTN_N_CORE_M_PCONF(p, c)) &
			~S32G_MC_ME_PRTN_N_CORE_M_PCONF_CCE_MASK;
	pconf |= (cce_bit & S32G_MC_ME_PRTN_N_CORE_M_PCONF_CCE_MASK);
	mmio_write_32(S32G_MC_ME_PRTN_N_CORE_M_PCONF(p, c), pconf);
}

static void mc_me_part_core_pupd_write_ccupd(uint32_t ccupd_bit, uint32_t p,
					    uint32_t c)
{
	uint32_t pupd;

	pupd = mmio_read_32(S32G_MC_ME_PRTN_N_CORE_M_PUPD(p, c)) &
			~S32G_MC_ME_PRTN_N_CORE_M_PUPD_CCUPD_MASK;
	pupd |= (ccupd_bit & S32G_MC_ME_PRTN_N_CORE_M_PUPD_CCUPD_MASK);
	mmio_write_32(S32G_MC_ME_PRTN_N_CORE_M_PUPD(p, c), pupd);
}

/*
 * PART<n>_[XYZ] register accessors
 */

static void mc_me_part_pconf_write_pce(uint32_t pce_bit, uint32_t p)
{
	uint32_t pconf;

	pconf = mmio_read_32(S32G_MC_ME_PRTN_N_PCONF(p)) &
			~S32G_MC_ME_PRTN_N_PCONF_PCE_MASK;
	pconf |= (pce_bit & S32G_MC_ME_PRTN_N_PCONF_PCE_MASK);
	mmio_write_32(S32G_MC_ME_PRTN_N_PCONF(p), pconf);
}

static void mc_me_part_pconf_write_osse(uint32_t osse_bit, uint32_t p)
{
	uint32_t pconf;

	pconf = mmio_read_32(S32G_MC_ME_PRTN_N_PCONF(p)) &
			~S32G_MC_ME_PRTN_N_PCONF_OSSE_MASK;
	pconf |= (osse_bit & S32G_MC_ME_PRTN_N_PCONF_OSSE_MASK);
	mmio_write_32(S32G_MC_ME_PRTN_N_PCONF(p), pconf);
}

static void mc_me_part_pupd_write_pcud(uint32_t pcud_bit, uint32_t p)
{
	uint32_t pupd;

	pupd = mmio_read_32(S32G_MC_ME_PRTN_N_PUPD(p)) &
			~S32G_MC_ME_PRTN_N_PUPD_PCUD_MASK;
	pupd |= (pcud_bit & S32G_MC_ME_PRTN_N_PUPD_PCUD_MASK);
	mmio_write_32(S32G_MC_ME_PRTN_N_PUPD(p), pupd);
}

static void mc_me_part_pupd_update_and_wait(uint32_t mask, uint32_t p)
{
	uint32_t pupd, pconf, stat;

	pupd = mmio_read_32(S32G_MC_ME_PRTN_N_PUPD(p));
	pupd |= mask;
	mmio_write_32(S32G_MC_ME_PRTN_N_PUPD(p), pupd);

	mc_me_apply_hw_changes();

	/* wait for the updates to apply */
	pconf = mmio_read_32(S32G_MC_ME_PRTN_N_PCONF(p));
	do {
		stat = mmio_read_32(S32G_MC_ME_PRTN_N_STAT(p));
	} while ((stat & mask) != (pconf & mask));
}

/*
 * PART<n>_COFB<m> register accessors
 */

static void mc_me_part_cofb_clken_write_req(uint32_t req, uint32_t val,
					    uint32_t part)
{
	uint32_t clken;

	clken = mmio_read_32(S32G_MC_ME_PRTN_N_COFB_0_CLKEN(part));
	clken |= ((val & 0x1) << req);
	mmio_write_32(S32G_MC_ME_PRTN_N_COFB_0_CLKEN(part), clken);
}

/*
 * Higher-level constructs
 */

/* First part of the "Software reset partition turn-on flow chart",
 * as per S32G RefMan.
 */
void mc_me_enable_partition(uint32_t part)
{
	uint32_t reg;

	/* Partition 0 is already enabled by BootROM */
	if (part == 0)
		return;

	mc_me_part_pconf_write_pce(S32G_MC_ME_PRTN_N_PCONF_PCE_MASK, part);
	mc_me_part_pupd_update_and_wait(S32G_MC_ME_PRTN_N_PUPD_PCUD_MASK, part);

	/* Unlock RDC register write */
	mmio_write_32(RDC_RD_CTRL(part), RDC_CTRL_UNLOCK);
	/* Enable the XBAR interface */
	mmio_write_32(RDC_RD_CTRL(part),
		      mmio_read_32(RDC_RD_CTRL(part)) & ~RDC_CTRL_XBAR_DISABLE);
	/* Wait until XBAR interface is enabled */
	while (mmio_read_32(RDC_RD_CTRL(part)) & RDC_CTRL_XBAR_DISABLE)
		;
	/* Release partition reset */
	reg = mmio_read_32(S32G_MC_RGM_PRST(part));
	reg &= ~MC_RGM_PRST_PERIPH_N_RST(0);
	mmio_write_32(S32G_MC_RGM_PRST(part), reg);

	/* Clear OSSE bit */
	mc_me_part_pconf_write_osse(0, part);

	mc_me_part_pupd_update_and_wait(S32G_MC_ME_PRTN_N_PUPD_OSSUD_MASK,
					part);
	while (mmio_read_32(S32G_MC_RGM_PSTAT(part)) &
			    MC_RGM_STAT_PERIPH_N_STAT(0))
		;
	/* Lock RDC register write */
	reg = mmio_read_32(RDC_RD_CTRL(part));
	reg &= ~RDC_CTRL_UNLOCK;
	mmio_write_32(RDC_RD_CTRL(part), reg);
}

/* Second part of the "Software reset partition turn-on flow chart" from the
 * S32G RefMan.
 *
 * Partition blocks must only be enabled after mc_me_enable_partition()
 * has been called for their respective partition.
 */
void mc_me_enable_partition_block(uint32_t part, uint32_t block)
{
	mc_me_part_cofb_clken_write_req(block, 1, part);
	mc_me_part_pupd_update_and_wait(S32G_MC_ME_PRTN_N_PUPD_PCUD_MASK, part);
}

static void core_high_addr_write(uintptr_t addr, uint32_t core)
{
	uint32_t addr_hi;
	uint32_t gpr09;

	addr_hi = (uint32_t)(addr >> 32);
	gpr09 = mmio_read_32(GPR_BASE_ADDR + GPR09_OFF);

	switch (core) {
	case 0:
		gpr09 |= ((addr_hi & CA53_0_0_RVBARADDR_39_32_MASK) <<
			   CA53_0_0_RVBARADDR_39_32_OFF);
		break;
	case 1:
		gpr09 |= ((addr_hi & CA53_0_1_RVBARADDR_39_32_MASK) <<
			   CA53_0_1_RVBARADDR_39_32_OFF);
		break;
	case 2:
		gpr09 |= ((addr_hi & CA53_1_0_RVBARADDR_39_32_MASK) <<
			   CA53_1_0_RVBARADDR_39_32_OFF);
		break;
	case 3:
		gpr09 |= ((addr_hi & CA53_1_1_RVBARADDR_39_32_MASK) <<
			   CA53_1_1_RVBARADDR_39_32_OFF);
		break;
	default:
		panic();
	}

	mmio_write_32(GPR_BASE_ADDR + GPR09_OFF, gpr09);
}

bool s32g_core_in_reset(uint32_t core)
{
	uint32_t stat, rst;

	rst = S32G_MC_RGM_RST_CA53_BIT(core);
	stat = mmio_read_32(S32G_MC_RGM_PSTAT(S32G_MC_RGM_RST_DOMAIN_CA53));
	return ((stat & rst) != 0);
}

static bool s32g_core_clock_running(uint32_t part, uint32_t core)
{
	uint32_t stat;

	stat = mmio_read_32(S32G_MC_ME_PRTN_N_CORE_M_STAT(part, core));
	return ((stat & S32G_MC_ME_PRTN_N_CORE_M_STAT_CCS_MASK) != 0);
}

static void enable_a53_partition(void)
{
	uint32_t pconf;

	pconf = mmio_read_32(S32G_MC_ME_PRTN_N_STAT(S32G_MC_ME_CA53_PART));

	/* Already enabled */
	if (pconf & S32G_MC_ME_PRTN_N_PCONF_PCE_MASK)
		return;

	mc_me_part_pconf_write_pce(S32G_MC_ME_PRTN_N_PCONF_PCE_MASK,
				   S32G_MC_ME_CA53_PART);
	mc_me_part_pupd_write_pcud(S32G_MC_ME_PRTN_N_PUPD_PCUD_MASK,
				   S32G_MC_ME_CA53_PART);
	mc_me_apply_hw_changes();
}

static void enable_a53_core_clock(uint32_t core)
{
	uint32_t pconf;
	uint32_t part = S32G_MC_ME_CA53_PART;

	/* For S32G2 we have the following mapping:
	 * MC_ME_PRTN1_CORE0_* -> CA53 cluster0 core0/1
	 * MC_ME_PRTN1_CORE2_* -> CA53 cluster1 core0/1
	 * For G32G3 we have the following mapping:
	 * MC_ME_PRTN1_CORE0_* -> CA53 cluster0 core0/1/2/3
	 * MC_ME_PRTN1_CORE2_* -> CA53 cluster1 core0/1/2/3
	 */
	uint32_t pconf_index = (core % 4) & ~1;

	pconf = mmio_read_32(S32G_MC_ME_PRTN_N_CORE_M_PCONF(part, pconf_index));

	if (pconf & S32G_MC_ME_PRTN_N_CORE_M_PCONF_CCE_MASK)
		return;

	/* When in performance (i.e., not in lockstep) mode, the following
	 * bits from the reset sequence are only defined for the first core
	 * of each CA53 cluster. Make sure this part of the sequence only runs
	 * on even-numbered cores.
	 */
	/* Enable clock and make changes effective */
	mc_me_part_core_pconf_write_cce(1, part, pconf_index);
	mc_me_part_core_pupd_write_ccupd(1, part, pconf_index);
	mc_me_apply_hw_changes();
	/* Wait for the core clock to become active */
	while (!s32g_core_clock_running(part, pconf_index))
		;
}

/** Reset and initialize secondary A53 core identified by its number
 *  in one of the MC_ME partitions
 */
void s32g_kick_secondary_ca53_core(uint32_t core, uintptr_t entrypoint)
{
	uint32_t rst;
	uint32_t rst_mask = S32G_MC_RGM_RST_CA53_BIT(core);
	const uint32_t part = S32G_MC_ME_CA53_PART;

	enable_a53_partition();

	/* GPR09 provides the 8 high-order bits for the core's start addr */
	core_high_addr_write(entrypoint, core);
	/* The MC_ME provides the 32 low-order bits for the core's
	 * start address
	 */
	mc_me_part_core_addr_write(entrypoint, part, core);

	enable_a53_core_clock(core);

	/* Release the core reset */
	rst = mmio_read_32(S32G_MC_RGM_PRST(S32G_MC_RGM_RST_DOMAIN_CA53));

	/* Forced reset */
	if (!(rst & rst_mask)) {
		rst |= rst_mask;
		mmio_write_32(S32G_MC_RGM_PRST(S32G_MC_RGM_RST_DOMAIN_CA53),
			      rst);
		while (!s32g_core_in_reset(core))
			;
	}

	rst = mmio_read_32(S32G_MC_RGM_PRST(S32G_MC_RGM_RST_DOMAIN_CA53));
	rst &= ~rst_mask;
	mmio_write_32(S32G_MC_RGM_PRST(S32G_MC_RGM_RST_DOMAIN_CA53), rst);
	/* Wait for reset bit to deassert */
	while (s32g_core_in_reset(core))
		;
}

void s32g_reset_core(uint8_t part, uint8_t core)
{
	uint32_t resetc;
	uint32_t statv;
	uintptr_t prst;
	uintptr_t pstat;

	if (part == S32G_MC_ME_CA53_PART) {
		resetc = S32G_MC_RGM_RST_CA53_BIT(core);
		prst = S32G_MC_RGM_PRST(S32G_MC_RGM_RST_DOMAIN_CA53);
		pstat = S32G_MC_RGM_PSTAT(S32G_MC_RGM_RST_DOMAIN_CA53);
	} else {
		/* M7 cores */
		resetc = S32G_MC_RGM_RST_CM7_BIT(core);
		prst = S32G_MC_RGM_PRST(S32G_MC_RGM_RST_DOMAIN_CM7);
		pstat = S32G_MC_RGM_PSTAT(S32G_MC_RGM_RST_DOMAIN_CM7);
	}
	statv = resetc;

	/* Assert the core reset */
	resetc |= mmio_read_32(prst);
	mmio_write_32(prst, resetc);

	/* Wait reset status */
	while (!(mmio_read_32(pstat) & statv))
		;
}

void s32g_turn_off_core(uint8_t part, uint8_t core)
{
	uint32_t stat;

	/* Assumption : The core is already in WFI */
	stat = mmio_read_32(S32G_MC_ME_PRTN_N_CORE_M_STAT(part, core));

	/* The clock isn't enabled */
	if (!(stat & S32G_MC_ME_PRTN_N_CORE_M_STAT_CCS_MASK))
		return;

	/* Wait for WFI */
	do {
		stat = mmio_read_32(S32G_MC_ME_PRTN_N_CORE_M_STAT(part, core));
	} while (!(stat & S32G_MC_ME_PRTN_N_CORE_M_STAT_WFI_MASK));

	/* Disable the core clock */
	mc_me_part_core_pconf_write_cce(0, part, core);
	mc_me_part_core_pupd_write_ccupd(1, part, core);

	/* Write valid key sequence to trigger the update. */
	mc_me_apply_hw_changes();

	/* Wait for the core clock to become inactive */
	while (s32g_core_clock_running(part, core))
		;

	s32g_reset_core(part, core);
}

void s32g_disable_cofb_clk(uint8_t part, uint32_t keep_blocks)
{
	uint32_t pconf;

	if (!mmio_read_32(S32G_MC_ME_PRTN_N_COFB0_CLKEN(part)))
		return;

	/* Disable all blocks */
	mmio_write_32(S32G_MC_ME_PRTN_N_COFB0_CLKEN(part), keep_blocks);

	pconf = mmio_read_32(S32G_MC_ME_PRTN_N_PCONF(part));

	/* Keep the partition on if not all the blocks are disabled */
	if (keep_blocks == 0)
		pconf &= ~S32G_MC_ME_PRTN_N_PCONF_PCE_MASK;

	/* Disable the clock to IPs */
	mmio_write_32(S32G_MC_ME_PRTN_N_PCONF(part), pconf);

	/* Initiate the clock hardware process */
	mc_me_part_pupd_write_pcud(S32G_MC_ME_PRTN_N_PUPD_PCUD_MASK, part);

	/* Write valid key sequence to trigger the update. */
	mc_me_apply_hw_changes();

	/* Make sure the COFB clock is gated */
	while (mmio_read_32(S32G_MC_ME_PRTN_N_COFB0_STAT(part)) != keep_blocks)
		;
}

void s32g_set_stby_master_core(uint8_t part, uint8_t core)
{
	/* Set the master core for the standby sequence */
	mmio_write_32(MC_ME_MAIN_COREID,
		      MC_ME_COREID_PIDX(part) |
		      MC_ME_COREID_CIDX(core));

	/* Initiate standby */
	mmio_write_32(MC_ME_MODE_CONF, MC_ME_MODE_CONF_STANDBY);
	mmio_write_32(MC_ME_MODE_UPD, MC_ME_MODE_UPD_UPD);

	/* Write valid key sequence to trigger the update. */
	mc_me_apply_hw_changes();
}

void s32g_destructive_reset(void)
{
	/* Prevent reset escalation */
	mmio_write_32(S32G_MC_RGM_DRET_ADDR, 0);

	mmio_write_32(MC_ME_MODE_CONF, MC_ME_MODE_CONF_DRST);
	mmio_write_32(MC_ME_MODE_UPD, MC_ME_MODE_UPD_UPD);

	/* Write valid key sequence to trigger the reset. */
	mc_me_apply_hw_changes();
}
