/*
 * Copyright 2020-2021 NXP
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
#include <ddr/ddr_utils.h>
#include <ddr/ddrss.h>
#include <ssram_mailbox.h>

static uint32_t enable_axi_ports(void);
static uint32_t get_mail(uint32_t *mail);
static uint32_t ack_mail(void);

uint8_t polling_needed = 2;

/* Modify bitfield value with delta, given bitfield position and mask */
static inline bool update_bf(uint32_t *v, uint8_t pos, uint32_t mask,
			     int delta)
{
	if (mask >= (((*v >> pos) & mask) + delta)) {
		*v = (*v & ~(mask << pos)) | ((((*v >> pos)
			& mask) + delta) << pos);
		return true;
	} else {
		return false;
	}
}

/*
 * Set the ddr clock source, FIRC or DDR_PLL_PHI0.
 * @param clk_src - requested clock source
 * @return - true whether clock source has been changed, false otherwise
 */
bool sel_clk_src(uint32_t clk_src)
{
	uint32_t tmp32;

	/* Check if the clock source is already set to clk_src*/
	tmp32 = mmio_read_32(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSS);
	if (((tmp32 & 0x3fffffff) >> 24) == clk_src)
		return false;

	/* To wait till clock switching is completed */
	do {
		tmp32 = mmio_read_32(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSS);
	} while (((tmp32 >> 16) & 0x1) != 0x0);

	/* Set DDR_CLK source on src_clk */
	tmp32 = mmio_read_32(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSC);
	mmio_write_32(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSC,
		      (0xc0ffffff & tmp32) | (clk_src << 24));

	 /* Request clock switch */
	tmp32 = mmio_read_32(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSC);
	mmio_write_32(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSC,
		      (0x1 << 2) | tmp32);

	/* To wait till clock switching is completed */
	do {
		tmp32 = mmio_read_32(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSS);
	} while (((tmp32 >> 16) & 0x1) != 0x0);

	/* To wait till Switch after request is succeeded */
	do {
		tmp32 = mmio_read_32(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSS);
	} while (((tmp32 >> 17) & 0x1) != 0x1);

	/* Make sure correct clock source is selected */
	do {
		tmp32 = mmio_read_32(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSS);
	} while (((tmp32 & 0x3fffffff) >> 24) != clk_src);

	return true;
}

/* Sets default AXI parity. */
uint32_t set_axi_parity(void)
{
	uint32_t tmp32;
	bool switched_to_firc;

	/* Enable Parity For All AXI Interfaces */
	mmio_write_32(DDR_SS_REG, mmio_read_32(DDR_SS_REG) | 0x1ff0);

	/* Set AXI_PARITY_TYPE to 0x1ff;   0-even, 1-odd */
	mmio_write_32(DDR_SS_REG, mmio_read_32(DDR_SS_REG) | 0x1ff0000);

	/* Set DFI1_ENABLED to 0x1 */
	mmio_write_32(DDR_SS_REG, mmio_read_32(DDR_SS_REG) | 0x1);

	/*
	 * Set ddr clock source on FIRC_CLK.
	 * If it's already set on FIRC_CLK, it returns false.
	 */
	switched_to_firc = sel_clk_src(FIRC_CLK_SRC);

	/* De-assert Reset To Controller and AXI Ports */
	tmp32 = mmio_read_32(MC_RGM_PRST_0);
	mmio_write_32(MC_RGM_PRST_0, ~(0x1 << 3) & tmp32);

	/* Check if the initial clock source was not on FIRC */
	if (switched_to_firc)
		sel_clk_src(DDR_PHI0_PLL);

	/* Enable HIF, CAM Queueing */
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DBG1, 0x0);

	/* Disable auto-refresh: RFSHCTL3.dis_auto_refresh = 1 */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3,
		      (1 | tmp32));

	/* Disable power down: PWRCTL.powerdown_en = 0 */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL,
		      ((~0x00000002) & tmp32));

	/* Disable self-refresh: PWRCTL.selfref_en = 0 */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL,
		      ((~0x00000001) & tmp32));

	/*
	 * Disable assertion of dfi_dram_clk_disable:
	 * PWRTL.en_dfi_dram_clk_disable = 0
	 */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL,
		      ((~0x00000008) & tmp32));

	/* Enable Quasi-Dynamic Programming */
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL,
		      DDRC_SWCTL_SWDONE_ENABLE);

	/* Confirm Register Programming Done Ack is Cleared */
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) == 1);

	/* DFI_INIT_COMPLETE_EN set to 0 */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC, (~0x1) & tmp32);

	/* Set SWCTL.sw_done to 1 */
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL,
		      DDRC_SWCTL_SWDONE_DONE);

	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) == 0);

	return NO_ERR;
}

/* Enables AXI port n. Programming Mode: Dynamic */
uint32_t enable_axi_ports(void)
{
	/* Port 0 Control Register*/
	mmio_write_32(DDRC_UMCTL2_MP_BASE_ADDR + OFFSET_DDRC_PCTRL_0,
		      0x00000001);
	/* Port 1 Control Register*/
	mmio_write_32(DDRC_UMCTL2_MP_BASE_ADDR + OFFSET_DDRC_PCTRL_1,
		      0x00000001);
	/* Port 2 Control Register*/
	mmio_write_32(DDRC_UMCTL2_MP_BASE_ADDR + OFFSET_DDRC_PCTRL_2,
		      0x00000001);

	return NO_ERR;
}

/*
 * Post PHY training setup - complementary settings that need to be
 * performed after running the firmware.
 */
uint32_t post_train_setup(void)
{
	uint32_t ret = NO_ERR;
	uint32_t tmp32;

	/*
	 * CalBusy.0 = 1, indicates the calibrator is actively calibrating.
	 * Wait Calibrating done.
	 */
	do {
		tmp32 = mmio_read_32(DDR_PHYA_MASTER0_CALBUSY);
	} while ((tmp32 & 0x1) != 0);

	store_csr(BL31SSRAM_CSR_BASE);

	/* Set SWCTL.sw_done to 0 */
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL,
		      DDRC_SWCTL_SWDONE_ENABLE);
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) != 0);

	/* Set DFIMISC.dfi_init_start to 1*/
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC,
		      (0x00000020 | tmp32));

	/* Set SWCTL.sw_done to 1 */
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL,
		      DDRC_SWCTL_SWDONE_DONE);
	/* Wait SWSTAT.sw_done_ack to 1*/
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) == 0);

	/* Wait DFISTAT.dfi_init_complete to 1 */
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFISTAT);
	} while ((tmp32 & 0x1) == 0);

	/* Set SWCTL.sw_done to 0 */
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL,
		      DDRC_SWCTL_SWDONE_ENABLE);
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK));

	/* Set dfi_init_start to 0 */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC,
		      (~0x00000020) & tmp32);

	/* Set dfi_complete_en to 1 */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC,
		      0x00000001 | tmp32);

	/* Set PWRCTL.selfref_sw to 0 */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL,
		      ((~0x00000020) & tmp32));

	/* Set SWCTL.sw_done to 1 */
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL,
		      DDRC_SWCTL_SWDONE_DONE);
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) == 0);

	/* Wait for DWC_ddr_umctl2 to move to normal operating mode */
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_STAT);
	} while ((tmp32 & 0x7) == 0);

	/* Enable auto-refresh: RFSHCTL3.dis_auto_refresh = 0 */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3,
		      (~0x00000001) & tmp32);

	/*
	 * If ECC feature is enabled (ECCCFG0[ecc_mode] > 0)
	 * initialize memory with the ecc scrubber
	 */
	if ((mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG0) & 0x7) > 0) {
		ret = init_memory_ecc_scrubber();
		if (ret != NO_ERR)
			return ret;
	}

	/* Enable power down: PWRCTL.powerdown_en = 1 */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL, 0x00000002 | tmp32);

	/* Enable self-refresh: PWRCTL.selfref_en = 1*/
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL, 0x00000001 | tmp32);

	/*
	 * Enable assertion of dfi_dram_clk_disable:
	 * PWRTL.en_dfi_dram_clk_disable = 1
	 */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL, 0x00000008 | tmp32);

	ret |= enable_derating_temp_errata();

	/*
	 * Each platform has a different number of AXI ports so this
	 * method should be implemented in hardware specific source
	 */
	ret |= enable_axi_ports();
	return ret;
}

/* Wait until firmware finishes execution and return training result */
uint32_t wait_firmware_execution(void)
{
	uint32_t mail = 0;

	while (mail == 0) {
		/* Obtain message from PHY (major message) */
		uint32_t ret = get_mail(&mail);

		if (ret != NO_ERR)
			return ret;

		/* 0x07 means OK, 0xFF means failure */
		if (mail == 0x07)
			return NO_ERR;
		if (mail == 0xff)
			/* Training stage failed */
			return TRAINING_FAILED;

		/* No error. Keep querying for mails */
		mail = 0;
	}
	return TIMEOUT_ERR;
}

/* Acknowledge received message */
uint32_t ack_mail(void)
{
	uint32_t timeout = DEFAULT_TIMEOUT;
	/* ACK message */
	mmio_write_32(DDR_PHYA_APBONLY_DCTWRITEPROT, 0);

	/* Wait firmware to respond to ACK (UctWriteProtShadow to be set) */
	while (--timeout && !(mmio_read_32(DDR_PHYA_APBONLY_UCTSHSADOWREGS) &
			      UCT_WRITE_PROT_SHADOW_MASK))
		;

	if (!timeout)
		return TIMEOUT_ERR;

	mmio_write_32(DDR_PHYA_APBONLY_DCTWRITEPROT, 1);

	return NO_ERR;
}

/* Read available message from DDR PHY microcontroller */
uint32_t get_mail(uint32_t *mail)
{
	uint32_t timeout = DEFAULT_TIMEOUT;

	while (--timeout && (mmio_read_32(DDR_PHYA_APBONLY_UCTSHSADOWREGS) &
			     UCT_WRITE_PROT_SHADOW_MASK))
		;

	if (!timeout)
		return TIMEOUT_ERR;

	*mail = mmio_read_32(DDR_PHYA_APBONLY_UCTWRITEONLYSHADOW);

	/* ACK */
	return ack_mail();
}

/* Initialize memory with the ecc scrubber */
uint32_t init_memory_ecc_scrubber(void)
{
	uint8_t region_lock;
	uint32_t tmp32, pattern = 0x12345678;

	/* Save previous ecc region parity locked state. */
	region_lock = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1)
				   & (0x1 << 4);

	/* Enable ecc region lock. */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1,
		      (0x1 << 4) | tmp32);

	/* Set SBRCTL.scrub_mode = 1. */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL, (0x1 << 2) | tmp32);

	/* Set SBRCTL.scrub_during_lowpower = 1. */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL, (0x1 << 1) | tmp32);

	/* Set SBRCTL.scrub_interval = 0. */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL,
		      ~(0x1fff << 8) & tmp32);

	/* Set the desired pattern through SBRWDATA0 register. */
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRWDATA0, pattern);

	/* Enable the SBR by programming SBRCTL.scrub_en = 1. */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL, 0x1 | tmp32);

	/*
	 * Poll until SBRSTAT.scrub_done = 1
	 * (all scrub writes commands have been sent).
	 */
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRSTAT);
	} while ((tmp32 & 0x2) == 0);

	/*
	 * Poll until SBRSTAT.scrub_busy = 0
	 * (all scrub writes data have been sent).
	 */
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRSTAT);
	} while (tmp32 & 0x1);

	/* Disable SBR by programming SBRCTL.scrub_en = 0. */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL, ~(0x1) & tmp32);

	/* Enter normal scrub operation (Reads): SBRCTL.scrub_mode = 0. */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL,
		      ~(0x1 << 2) & tmp32);

	/* Set SBRCTL.scrub_interval = 1. */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	tmp32 = ~(0x1fff << 8) & tmp32;
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL, (0x1 << 8) | tmp32);

	/* Enable the SBR by programming SBRCTL.scrub_en = 1. */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL, 0x1 | tmp32);

	/* Restore locked state of ecc region. */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1);
	tmp32 = (tmp32 & ~(0x1 << 4)) | (region_lock << 4);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1, tmp32);

	return NO_ERR;
}

/* Read lpddr4 mode register with given index */
uint32_t read_lpddr4_mr(uint8_t mr_index)
{
	uint32_t tmp32;

	/* Set MRR_DDR_SEL_REG to 0x1 to enable LPDDR4 mode */
	tmp32 = mmio_read_32(PERF_BASE_ADDR + OFFSET_MRR_0_DATA_REG_ADDR);
	mmio_write_32(PERF_BASE_ADDR + OFFSET_MRR_0_DATA_REG_ADDR,
		      (tmp32 | 0x1));

	/*
	 * Ensure no MR transaction is in progress:
	 * mr_wr_busy signal must be low
	 */
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRSTAT);
	} while ((tmp32 & 0x1) != 0);

	/* Set MR_TYPE = 0x1 (Read) and MR_RANK = 0x1 (Rank 0) */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);
	tmp32 |= 0x1;
	tmp32 = (tmp32 & ~(0xf << 4)) | (0x1 << 4);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0, tmp32);

	/* Configure MR address: MRCTRL1[8:15] */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL1);
	tmp32 = (tmp32 & ~(0xff << 8)) | (mr_index << 8);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL1,
		      tmp32);

	__asm__("DSB SY");

	/* Initiate MR transaction: MR_WR = 0x1 */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0,
		      tmp32 | (0x1u << 31));

	/* Wait until MR transaction completed */
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRSTAT);
	} while ((tmp32 & 0x1) != 0);

	return mmio_read_32(PERF_BASE_ADDR + OFFSET_MRR_1_DATA_REG_ADDR);
}

/* Write data in lpddr4 mode register with given index */
uint32_t write_lpddr4_mr(uint8_t mr_index, uint8_t mr_data)
{
	uint32_t tmp32;

	/* Set MRR_DDR_SEL_REG to 0x1 to enable LPDDR4 mode */
	tmp32 = mmio_read_32(PERF_BASE_ADDR + OFFSET_MRR_0_DATA_REG_ADDR);
	mmio_write_32(PERF_BASE_ADDR + OFFSET_MRR_0_DATA_REG_ADDR,
		      tmp32 | 0x1);

	/*
	 * Ensure no MR transaction is in progress:
	 * mr_wr_busy signal must be low
	 */
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRSTAT);
	} while ((tmp32 & 0x1) != 0);

	/* Set MR_TYPE = 0x0 (Write) and MR_RANK = 0x1 (Rank 0) */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);
	tmp32 &= ~(0x1);
	tmp32 = (tmp32 & ~(0xf << 4)) | (0x1 << 4);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0, tmp32);

	/* Configure MR address: MRCTRL1[8:15] and MR data: MRCTRL1[0:7]*/
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL1);
	tmp32 = (tmp32 & (0xffffu << 16)) | (mr_index << 8) | mr_data;
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL1, tmp32);

	__asm__("DSB SY");

	/* Initiate MR transaction: MR_WR = 0x1 */
	tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);
	mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0,
		      tmp32 | (0x1u << 31));

	/* Wait until MR transaction completed */
	do {
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_MRSTAT);
	} while ((tmp32 & 0x1) != 0);

	return NO_ERR;
}

/* Read Temperature Update Flag from lpddr4 MR4 register. */
uint8_t read_tuf(void)
{
	uint32_t mr4_val;
	uint8_t mr4_die_1, mr4_die_2;

	mr4_val = read_lpddr4_mr(MR4);
	mr4_die_1 = mr4_val & 0x7;
	mr4_die_2 = (mr4_val >> 16) & 0x7;

	return mr4_die_1 > mr4_die_2 ? mr4_die_1 : mr4_die_2;
}

/*
 * Enable ERR050543 errata workaround.
 * If the system is hot or cold prior to enabling derating, Temperature Update
 * Flag might not be set in MR4 register, causing incorrect refresh period and
 * derated timing parameters (tRCD, tRAS, rRP, tRRD being used.
 * Software workaround requires reading MR register and adjusting timing
 * parameters, if necessary.
 */
uint32_t enable_derating_temp_errata(void)
{
	uint32_t tmp32, bf_val;

	if (read_tuf() > TUF_THRESHOLD) {
		/* Disable timing parameter derating: DERATEEN.DERATE_EN = 0 */
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DERATEEN);
		mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DERATEEN,
			      tmp32 & ~DDRC_DERATEEN_MASK_DISABLE);

		/*
		 * Update average time interval between refreshes per rank:
		 * RFSHTMG.T_RFC_NOM_X1_X32 = RFSHTMG.T_RFC_NOM_X1_X32 / 4
		 */
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHTMG);
		bf_val = (tmp32 >> DDRC_RFSHTMG_VAL_SHIFT) & DDRC_RFSHTMG_VAL;
		bf_val = bf_val >> 2;
		tmp32 = (tmp32 & ~DDRC_RFSHTMG_MASK) |
			(bf_val << DDRC_RFSHTMG_VAL_SHIFT);
		mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHTMG, tmp32);

		/*
		 * Toggle RFSHCTL3.REFRESH_UPDATE_LEVEL to indicate that
		 * refresh registers have been updated
		 */
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);
		bf_val = (tmp32 >> DDRC_RFSHCTL3_UPDATE_SHIFT) &
			 DDRC_RFSHCTL3_AUTO_REFRESH_VAL;
		bf_val = bf_val ^ 1;
		tmp32 = (tmp32 & ~DDRC_RFSHCTL3_MASK) |
			(bf_val << DDRC_RFSHCTL3_UPDATE_SHIFT);
		mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3,
			      tmp32);

		/* Set SWCTL.sw_done to 0 */
		mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL,
			      DDRC_SWCTL_SWDONE_ENABLE);
		do {
			tmp32 = mmio_read_32(DDRC_BASE_ADDR +
					     OFFSET_DDRC_SWSTAT);
		} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) != 0);

		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4);
		/*
		 * Set minimum time from activate to read/write command to same
		 * bank: DRAMTMG4.T_RCD += 2
		 */
		if (!update_bf(&tmp32, DDRC_DRAMTMG4_TRCD_POS,
			       DDRC_DRAMTMG5_TRCD_MASK, 2))
			return BITFIELD_EXCEEDED;
		/*
		 * Set minimum time between activates from bank "a" to bank "b"
		 * DRAMTMG4.T_RRD += 2
		 */
		if (!update_bf(&tmp32, DDRC_DRAMTMG4_TRRD_POS,
			       DDRC_DRAMTMG5_TRRD_MASK, 2))
			return BITFIELD_EXCEEDED;
		mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4, tmp32);

		/*
		 * Set minimum time between activate and precharge to same bank
		 * DRAMTMG0.T_RAS_MIN += 2
		 */
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG0);
		if (!update_bf(&tmp32, DDRC_DRAMTMG0_TRAS_POS,
			       DDRC_DRAMTMG0_TRAS_MASK, 2))
			return BITFIELD_EXCEEDED;
		mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG0, tmp32);

		/*
		 * Set minimum time from single-bank precharge to activate of
		 * same bank: DRAMTMG4.T_RP += 2
		 */
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4);
		if (!update_bf(&tmp32, DDRC_DRAMTMG4_TRP_POS,
			       DDRC_DRAMTMG4_TRP_MASK, 2))
			return BITFIELD_EXCEEDED;
		mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4, tmp32);

		/*
		 * Set minimum time between activates to same bank:
		 * DRAMTMG1.T_RC += 3
		 */
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG1);
		if (!update_bf(&tmp32, DDRC_DRAMTMG1_TRC_POS,
			       DDRC_DRAMTMG1_TRC_MASK, 3))
			return BITFIELD_EXCEEDED;
		mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG1, tmp32);

		/* Set SWCTL.sw_done to 1 */
		mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL,
			      DDRC_SWCTL_SWDONE_DONE);
		do {
			tmp32 = mmio_read_32(DDRC_BASE_ADDR +
					     OFFSET_DDRC_SWSTAT);
		} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) == 0);

		polling_needed = 1;

	} else {
		/* Enable timing parameter derating: DERATEEN.DERATE_EN = 1 */
		tmp32 = mmio_read_32(DDRC_BASE_ADDR + OFFSET_DDRC_DERATEEN);
		mmio_write_32(DDRC_BASE_ADDR + OFFSET_DDRC_DERATEEN,
			      tmp32 | DDRC_DERATEEN_ENABLE);

		polling_needed = 0;
	}
	return NO_ERR;
}
