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

#ifndef LP_DDR_LP_H_
#define LP_DDR_LP_H_

#include "ddr_init.h"

#define DDRSS_BASE_ADDR                 0x40380000

#define DDR_GPR_OFFSET                  (0x4007c600UL)
#define DDR_CONFIG_0_REG                (DDR_GPR_OFFSET + 0x00)
#define DDR_RET_CONTROL_REG             (DDR_GPR_OFFSET + 0x1c)
#define DDR_RET_CONTROL_MASK            SHIFT_BIT(0)
#define DDR_CONFIG_0_MEM_RET            SHIFT_BIT(14)

#define DFI_FREQUENCY(f)                ((f) << 8)
#define SELFREF_STATE_SRPD              (0x2UL << 8)
#define SELFREF_STATE_MASK              (SHIFT_BIT(8) | SHIFT_BIT(9))
#define SELFREF_TYPE_NOT_AUTO_SR_CTRL   (0x2UL << 4)
#define OPERATING_MODE_SELF_REFRESH     (0x3)

extern const uint32_t csr_to_store[];
extern size_t csr_to_store_size;

/* Transition the DDR SubSystem from normal mode to retention mode. */
void ddrss_to_io_retention_mode(void);

/* Transition the DDR SubSystem from retention mode to normal mode. */
uint32_t ddrss_to_normal_mode(uintptr_t csr_array);

/* Store Configuration Status Registers. */
void store_csr(uintptr_t store_at);

/* Load Configuration Status Registers. */
void load_csr(uintptr_t load_from);

#endif /* LP_DDR_LP_H_ */
