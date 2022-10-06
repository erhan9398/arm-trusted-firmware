/*
 * Copyright 2021-2022 NXP
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

#include "ddr_init.h"

struct regconf_16 phy_cfg[] = {
	{0x4038040c, 0x00000001},
	{0x40380bdc, 0x00000001},
	{0x403945dc, 0x000001ff},
	{0x403945e0, 0x000001ff},
	{0x403965dc, 0x000001ff},
	{0x403965e0, 0x000001ff},
	{0x403985dc, 0x000001ff},
	{0x403985e0, 0x000001ff},
	{0x4039a5dc, 0x000001ff},
	{0x4039a5e0, 0x000001ff},
	{0x403803e8, 0x000001ff},
	{0x403823e8, 0x000001ff},
	{0x403843e8, 0x000001ff},
	{0x403863e8, 0x000001ff},
	{0x403883e8, 0x000001ff},
	{0x4038a3e8, 0x000001ff},
	{0x4038c3e8, 0x000001ff},
	{0x4038e3e8, 0x000001ff},
	{0x403903e8, 0x000001ff},
	{0x403923e8, 0x000001ff},
	{0x403816dc, 0x00000019},
	{0x40381494, 0x00000002},
	{0x40380c58, 0x00000000},
	{0x40381438, 0x000001e3},
	{0x403814fc, 0x00000002},
	{0x40381618, 0x00000212},
	{0x40381608, 0x00000061},
	{0x40381530, 0x00000003},
	{0x403945a0, 0x00000600},
	{0x403945a4, 0x00000600},
	{0x403965a0, 0x00000600},
	{0x403965a4, 0x00000600},
	{0x403985a0, 0x00000600},
	{0x403985a4, 0x00000600},
	{0x4039a5a0, 0x00000600},
	{0x4039a5a4, 0x00000600},
	{0x4039454c, 0x00000e3f},
	{0x40394550, 0x00000e3f},
	{0x4039654c, 0x00000e3f},
	{0x40396550, 0x00000e3f},
	{0x4039854c, 0x00000e3f},
	{0x40398550, 0x00000e3f},
	{0x4039a54c, 0x00000e3f},
	{0x4039a550, 0x00000e3f},
	{0x403803e0, 0x0000007f},
	{0x403823e0, 0x0000007f},
	{0x403843e0, 0x0000007f},
	{0x403863e0, 0x0000007f},
	{0x403883e0, 0x0000007f},
	{0x4038a3e0, 0x0000007f},
	{0x4038c3e0, 0x0000007f},
	{0x4038e3e0, 0x0000007f},
	{0x403903e0, 0x0000007f},
	{0x403923e0, 0x0000007f},
	{0x403813c8, 0x00000003},
	{0x403815ec, 0x00000004},
	{0x40381528, 0x00000011},
	{0x40381668, 0x00000002},
	{0x40381314, 0x00000320},
	{0x40381640, 0x00000009},
	{0x40381690, 0x00000134},
	{0x4039451c, 0x00000e20},
	{0x40394520, 0x00000e20},
	{0x4039651c, 0x00000e20},
	{0x40396520, 0x00000e20},
	{0x4039851c, 0x00000e20},
	{0x40398520, 0x00000e20},
	{0x4039a51c, 0x00000e20},
	{0x4039a520, 0x00000e20},
	{0x403817c0, 0x00000001},
	{0x403813d0, 0x00000001},
	{0x40381798, 0x00000000},
	{0x4038179c, 0x00000000},
	{0x403817a0, 0x00004444},
	{0x403817a4, 0x00008888},
	{0x403817a8, 0x00005555},
	{0x403817ac, 0x00000000},
	{0x403817b0, 0x00000000},
	{0x403817b4, 0x0000f000},
	{0x4039456c, 0x00000500},
	{0x4039656c, 0x00000500},
	{0x4039856c, 0x00000500},
	{0x4039a56c, 0x00000500},
	{0x4038144c, 0x00000000},
	{0x40381480, 0x00000000},
	{0x40381478, 0x00000000},
};

size_t phy_cfg_size = ARRAY_SIZE(phy_cfg);
