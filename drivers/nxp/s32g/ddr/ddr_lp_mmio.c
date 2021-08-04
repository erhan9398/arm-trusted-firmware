/*
 * Copyright 2021 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <ddr/ddr_lp.h>
#include <ddr/ddr_init.h>
#include <lib/mmio.h>

/* Store Configuration Status Registers. */
void store_csr(uintptr_t store_at)
{
	size_t i;
	uint16_t csr;

	mmio_write_32(MICROCONT_MUX_SEL, UNLOCK_CSR_ACCESS);
	mmio_write_32(DDR_PHYA_UCCLKHCLKENABLES, HCLKEN_MASK | UCCLKEN_MASK);

	for (i = 0; i < csr_to_store_size; i++) {
		csr = mmio_read_16(DDRSS_BASE_ADDR + csr_to_store[i]);
		mmio_write_16(store_at, csr);
		store_at += sizeof(uint16_t);
	}

	mmio_write_32(DDR_PHYA_UCCLKHCLKENABLES, HCLKEN_MASK);
	mmio_write_32(MICROCONT_MUX_SEL, LOCK_CSR_ACCESS);
}

/* Load Configuration Status Registers. */
void load_csr(uintptr_t load_from)
{
	size_t i;
	uint16_t csr;

	for (i = 0; i < csr_to_store_size; i++) {
		csr = mmio_read_16(load_from);
		load_from += sizeof(uint16_t);
		mmio_write_16(DDRSS_BASE_ADDR + csr_to_store[i], csr);
	}
}

/* Transition the DDR SubSystem from normal mode to retention mode. */
void ddrss_to_io_retention_mode(void)
{
	uint32_t sbrctl, pwrctl, swctl, dfimisc, tmp32;

	/* Disable AXI Ports */
	mmio_write_32(DDRC_UMCTL2_MP_BASE_ADDR + OFFSET_DDRC_PCTRL_0, 0);
	mmio_write_32(DDRC_UMCTL2_MP_BASE_ADDR + OFFSET_DDRC_PCTRL_1, 0);
	mmio_write_32(DDRC_UMCTL2_MP_BASE_ADDR + OFFSET_DDRC_PCTRL_2, 0);

	do {
		tmp32 = mmio_read_32(DDRC_UMCTL2_MP_BASE_ADDR +
				     OFFSET_DDRC_STAT);
	} while (tmp32 != 0);

	/* Disable Scrubber */
	sbrctl = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL, sbrctl &
		      (~SCRUB_EN_MASK));
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRSTAT);
	} while ((tmp32 & SCRUB_BUSY_MASK) != 0);

	/* Move to Self Refresh */
	pwrctl = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL, pwrctl |
		      SELFREF_SW_MASK);

	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_STAT);
	} while ((tmp32 & OPERATING_MODE_MASK) != OPERATING_MODE_SELF_REFRESH);

	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_STAT);
	} while ((tmp32 & SELFREF_TYPE_MASK) != SELFREF_TYPE_NOT_AUTO_SR_CTRL);

	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_STAT);
	} while ((tmp32 & SELFREF_STATE_MASK) != SELFREF_STATE_SRPD);

	/* Transition Phy to LP3 */
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC, 0);
	swctl = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL, swctl &
		      (~SW_DONE_MASK));

	dfimisc = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC, dfimisc |
		      DFI_FREQUENCY(0x1f));

	dfimisc = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC, dfimisc |
		      DFI_INIT_START_MASK);
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFISTAT);
	} while ((tmp32 & DFI_INIT_COMPLETE_MASK) != 0);

	dfimisc = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC, dfimisc |
		      DFI_FREQUENCY(0x1f));
	dfimisc = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC, dfimisc &
		      (~DFI_INIT_START_MASK));
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFISTAT);
	} while ((tmp32 & DFI_INIT_COMPLETE_MASK) == 0);

	swctl = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL, swctl |
		      SW_DONE_MASK);
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & SW_DONE_ACK_MASK) == 0);

	/* Set PwrOkIn to 0 */
	tmp32 = mmio_read_32(DDR_RET_CONTROL_REG);
	mmio_write_32(DDR_RET_CONTROL_REG, tmp32 & (~DDR_RET_CONTROL_MASK));

	tmp32 = mmio_read_32(DDR_CONFIG_0_REG);
	mmio_write_32(DDR_CONFIG_0_REG, tmp32 | DDR_CONFIG_0_MEM_RET);
}

/* Transition the DDR SubSystem from retention mode to normal mode. */
uint32_t ddrss_to_normal_mode(uintptr_t csr_array)
{
	uint32_t pwrctl, init0, ret;

	ret = load_register_cfg(ddrc_cfg_size, ddrc_cfg);
	if (ret != 0)
		return ret;

	mmio_write_32(MICROCONT_MUX_SEL, UNLOCK_CSR_ACCESS);
	ret = load_dq_cfg(dq_swap_cfg_size, dq_swap_cfg);
	if (ret != 0)
		return ret;

	mmio_write_32(MICROCONT_MUX_SEL, LOCK_CSR_ACCESS);

	init0 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_INIT0);
	init0 |= SKIP_DRAM_INIT_MASK;
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_INIT0, init0);

	pwrctl = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL, pwrctl |
		      SELFREF_SW_MASK);

	/* Setup AXI ports parity */
	ret = set_axi_parity();
	if (ret != NO_ERR)
		return ret;

	mmio_write_32(MICROCONT_MUX_SEL, UNLOCK_CSR_ACCESS);
	ret = load_register_cfg_16(phy_cfg_size, phy_cfg);
	if (ret != 0)
		return ret;

	mmio_write_32(MICROCONT_MUX_SEL, LOCK_CSR_ACCESS);

	/* Optimal PLL */
	set_optimal_pll();

	/* Reload saved CSRs */
	mmio_write_32(MICROCONT_MUX_SEL, UNLOCK_CSR_ACCESS);
	load_csr(csr_array);
	ret = load_register_cfg_16(pie_cfg_size, pie_cfg);
	if (ret != 0)
		return ret;

	mmio_write_32(MICROCONT_MUX_SEL, LOCK_CSR_ACCESS);

	return post_train_setup(0);
}
