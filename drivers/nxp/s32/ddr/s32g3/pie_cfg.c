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

struct regconf_16 pie_cfg[] = {
	{0x40380c04, 0x00000010},
	{0x40380c0c, 0x00000400},
	{0x40380c24, 0x0000010e},
	{0x40380c3c, 0x00000000},
	{0x40380c54, 0x00000000},
	{0x40380c6c, 0x00000008},
	{0x40380dc0, 0x0000000b},
	{0x40380dc8, 0x00000480},
	{0x40380dd0, 0x00000109},
	{0x40380dd8, 0x00000008},
	{0x40380de0, 0x00000448},
	{0x40380de8, 0x00000139},
	{0x40380df0, 0x00000008},
	{0x40380df8, 0x00000478},
	{0x40380e00, 0x00000109},
	{0x40380e08, 0x00000000},
	{0x40380e10, 0x000000e8},
	{0x40380e18, 0x00000109},
	{0x40380e20, 0x00000002},
	{0x40380e28, 0x00000010},
	{0x40380e30, 0x00000139},
	{0x40380e38, 0x0000000b},
	{0x40380e40, 0x000007c0},
	{0x40380e48, 0x00000139},
	{0x40380e50, 0x00000044},
	{0x40380e58, 0x00000633},
	{0x40380e60, 0x00000159},
	{0x40380e68, 0x0000014f},
	{0x40380e70, 0x00000630},
	{0x40380e78, 0x00000159},
	{0x40380e80, 0x00000047},
	{0x40380e88, 0x00000633},
	{0x40380e90, 0x00000149},
	{0x40380e98, 0x0000004f},
	{0x40380ea0, 0x00000633},
	{0x40380ea8, 0x00000179},
	{0x40380eb0, 0x00000008},
	{0x40380eb8, 0x000000e0},
	{0x40380ec0, 0x00000109},
	{0x40380ec8, 0x00000000},
	{0x40380ed0, 0x000007c8},
	{0x40380ed8, 0x00000109},
	{0x40380ee0, 0x00000000},
	{0x40380ee8, 0x00000001},
	{0x40380ef0, 0x00000008},
	{0x40380ef8, 0x00000000},
	{0x40380f00, 0x0000045a},
	{0x40380f08, 0x00000009},
	{0x40380f10, 0x00000000},
	{0x40380f18, 0x00000448},
	{0x40380f20, 0x00000109},
	{0x40380f28, 0x00000040},
	{0x40380f30, 0x00000633},
	{0x40380f38, 0x00000179},
	{0x40380f40, 0x00000001},
	{0x40380f48, 0x00000618},
	{0x40380f50, 0x00000109},
	{0x40380f58, 0x000040c0},
	{0x40380f60, 0x00000633},
	{0x40380f68, 0x00000149},
	{0x40380f70, 0x00000008},
	{0x40380f78, 0x00000004},
	{0x40380f80, 0x00000048},
	{0x40380f88, 0x00004040},
	{0x40380f90, 0x00000633},
	{0x40380f98, 0x00000149},
	{0x40380fa0, 0x00000000},
	{0x40380fa8, 0x00000004},
	{0x40380fb0, 0x00000048},
	{0x40380fb8, 0x00000040},
	{0x40380fc0, 0x00000633},
	{0x40380fc8, 0x00000149},
	{0x40380fd0, 0x00000010},
	{0x40380fd8, 0x00000004},
	{0x40380fe0, 0x00000018},
	{0x40380fe8, 0x00000000},
	{0x40380ff0, 0x00000004},
	{0x40380ff8, 0x00000078},
	{0x40381000, 0x00000549},
	{0x40381008, 0x00000633},
	{0x40381010, 0x00000159},
	{0x40381018, 0x00000d49},
	{0x40381020, 0x00000633},
	{0x40381028, 0x00000159},
	{0x40381030, 0x0000094a},
	{0x40381038, 0x00000633},
	{0x40381040, 0x00000159},
	{0x40381048, 0x00000441},
	{0x40381050, 0x00000633},
	{0x40381058, 0x00000149},
	{0x40381060, 0x00000042},
	{0x40381068, 0x00000633},
	{0x40381070, 0x00000149},
	{0x40381078, 0x00000001},
	{0x40381080, 0x00000633},
	{0x40381088, 0x00000149},
	{0x40381090, 0x00000000},
	{0x40381098, 0x000000e0},
	{0x403810a0, 0x00000109},
	{0x403810a8, 0x0000000a},
	{0x403810b0, 0x00000010},
	{0x403810b8, 0x00000109},
	{0x403810c0, 0x00000009},
	{0x403810c8, 0x000003c0},
	{0x403810d0, 0x00000149},
	{0x403810d8, 0x00000009},
	{0x403810e0, 0x000003c0},
	{0x403810e8, 0x00000159},
	{0x403810f0, 0x00000018},
	{0x403810f8, 0x00000010},
	{0x40381100, 0x00000109},
	{0x40381108, 0x00000000},
	{0x40381110, 0x000003c0},
	{0x40381118, 0x00000109},
	{0x40381120, 0x00000018},
	{0x40381128, 0x00000004},
	{0x40381130, 0x00000048},
	{0x40381134, 0x00000018},
	{0x40381138, 0x00000004},
	{0x4038113c, 0x00000058},
	{0x40381140, 0x0000000b},
	{0x40381144, 0x00000010},
	{0x40381148, 0x00000109},
	{0x4038114c, 0x00000001},
	{0x40381150, 0x00000010},
	{0x40381154, 0x00000109},
	{0x40381158, 0x00000005},
	{0x4038115c, 0x000007c0},
	{0x40381160, 0x00000109},
	{0x40380000, 0x00000811},
	{0x40380080, 0x00000880},
	{0x40380100, 0x00000000},
	{0x40380180, 0x00000000},
	{0x40380004, 0x00004008},
	{0x40380084, 0x00000083},
	{0x40380104, 0x0000004f},
	{0x40380184, 0x00000000},
	{0x40380008, 0x00004040},
	{0x40380088, 0x00000083},
	{0x40380108, 0x00000051},
	{0x40380188, 0x00000000},
	{0x4038000c, 0x00000811},
	{0x4038008c, 0x00000880},
	{0x4038010c, 0x00000000},
	{0x4038018c, 0x00000000},
	{0x40380010, 0x00000720},
	{0x40380090, 0x0000000f},
	{0x40380110, 0x00001740},
	{0x40380190, 0x00000000},
	{0x40380014, 0x00000016},
	{0x40380094, 0x00000083},
	{0x40380114, 0x0000004b},
	{0x40380194, 0x00000000},
	{0x40380018, 0x00000716},
	{0x40380098, 0x0000000f},
	{0x40380118, 0x00002001},
	{0x40380198, 0x00000000},
	{0x4038001c, 0x00000716},
	{0x4038009c, 0x0000000f},
	{0x4038011c, 0x00002800},
	{0x4038019c, 0x00000000},
	{0x40380020, 0x00000716},
	{0x403800a0, 0x0000000f},
	{0x40380120, 0x00000f00},
	{0x403801a0, 0x00000000},
	{0x40380024, 0x00000720},
	{0x403800a4, 0x0000000f},
	{0x40380124, 0x00001400},
	{0x403801a4, 0x00000000},
	{0x40380028, 0x00000e08},
	{0x403800a8, 0x00000c15},
	{0x40380128, 0x00000000},
	{0x403801a8, 0x00000000},
	{0x4038002c, 0x00000625},
	{0x403800ac, 0x00000015},
	{0x4038012c, 0x00000000},
	{0x403801ac, 0x00000000},
	{0x40380030, 0x00004028},
	{0x403800b0, 0x00000080},
	{0x40380130, 0x00000000},
	{0x403801b0, 0x00000000},
	{0x40380034, 0x00000e08},
	{0x403800b4, 0x00000c1a},
	{0x40380134, 0x00000000},
	{0x403801b4, 0x00000000},
	{0x40380038, 0x00000625},
	{0x403800b8, 0x0000001a},
	{0x40380138, 0x00000000},
	{0x403801b8, 0x00000000},
	{0x4038003c, 0x00004040},
	{0x403800bc, 0x00000080},
	{0x4038013c, 0x00000000},
	{0x403801bc, 0x00000000},
	{0x40380040, 0x00002604},
	{0x403800c0, 0x00000015},
	{0x40380140, 0x00000000},
	{0x403801c0, 0x00000000},
	{0x40380044, 0x00000708},
	{0x403800c4, 0x00000005},
	{0x40380144, 0x00000000},
	{0x403801c4, 0x00002002},
	{0x40380048, 0x00000008},
	{0x403800c8, 0x00000080},
	{0x40380148, 0x00000000},
	{0x403801c8, 0x00000000},
	{0x4038004c, 0x00002604},
	{0x403800cc, 0x0000001a},
	{0x4038014c, 0x00000000},
	{0x403801cc, 0x00000000},
	{0x40380050, 0x00000708},
	{0x403800d0, 0x0000000a},
	{0x40380150, 0x00000000},
	{0x403801d0, 0x00002002},
	{0x40380054, 0x00004040},
	{0x403800d4, 0x00000080},
	{0x40380154, 0x00000000},
	{0x403801d4, 0x00000000},
	{0x40380058, 0x0000060a},
	{0x403800d8, 0x00000015},
	{0x40380158, 0x00001200},
	{0x403801d8, 0x00000000},
	{0x4038005c, 0x0000061a},
	{0x403800dc, 0x00000015},
	{0x4038015c, 0x00001300},
	{0x403801dc, 0x00000000},
	{0x40380060, 0x0000060a},
	{0x403800e0, 0x0000001a},
	{0x40380160, 0x00001200},
	{0x403801e0, 0x00000000},
	{0x40380064, 0x00000642},
	{0x403800e4, 0x0000001a},
	{0x40380164, 0x00001300},
	{0x403801e4, 0x00000000},
	{0x40380068, 0x00004808},
	{0x403800e8, 0x00000880},
	{0x40380168, 0x00000000},
	{0x403801e8, 0x00000000},
	{0x40381164, 0x00000000},
	{0x40381168, 0x00000790},
	{0x4038116c, 0x0000011a},
	{0x40381170, 0x00000008},
	{0x40381174, 0x000007aa},
	{0x40381178, 0x0000002a},
	{0x4038117c, 0x00000010},
	{0x40381180, 0x000007b2},
	{0x40381184, 0x0000002a},
	{0x40381188, 0x00000000},
	{0x4038118c, 0x000007c8},
	{0x40381190, 0x00000109},
	{0x40381194, 0x00000010},
	{0x40381198, 0x00000010},
	{0x4038119c, 0x00000109},
	{0x403811a0, 0x00000010},
	{0x403811a4, 0x000002a8},
	{0x403811a8, 0x00000129},
	{0x403811ac, 0x00000008},
	{0x403811b0, 0x00000370},
	{0x403811b4, 0x00000129},
	{0x403811b8, 0x0000000a},
	{0x403811bc, 0x000003c8},
	{0x403811c0, 0x000001a9},
	{0x403811c4, 0x0000000c},
	{0x403811c8, 0x00000408},
	{0x403811cc, 0x00000199},
	{0x403811d0, 0x00000014},
	{0x403811d4, 0x00000790},
	{0x403811d8, 0x0000011a},
	{0x403811dc, 0x00000008},
	{0x403811e0, 0x00000004},
	{0x403811e4, 0x00000018},
	{0x403811e8, 0x0000000e},
	{0x403811ec, 0x00000408},
	{0x403811f0, 0x00000199},
	{0x403811f4, 0x00000008},
	{0x403811f8, 0x00008568},
	{0x403811fc, 0x00000108},
	{0x40381200, 0x00000018},
	{0x40381204, 0x00000790},
	{0x40381208, 0x0000016a},
	{0x4038120c, 0x00000008},
	{0x40381210, 0x000001d8},
	{0x40381214, 0x00000169},
	{0x40381218, 0x00000010},
	{0x4038121c, 0x00008558},
	{0x40381220, 0x00000168},
	{0x40381224, 0x00000070},
	{0x40381228, 0x00000788},
	{0x4038122c, 0x0000016a},
	{0x40381230, 0x00001ff8},
	{0x40381234, 0x000085a8},
	{0x40381238, 0x000001e8},
	{0x4038123c, 0x00000050},
	{0x40381240, 0x00000798},
	{0x40381244, 0x0000016a},
	{0x40381248, 0x00000060},
	{0x4038124c, 0x000007a0},
	{0x40381250, 0x0000016a},
	{0x40381254, 0x00000008},
	{0x40381258, 0x00008310},
	{0x4038125c, 0x00000168},
	{0x40381260, 0x00000008},
	{0x40381264, 0x0000a310},
	{0x40381268, 0x00000168},
	{0x4038126c, 0x0000000a},
	{0x40381270, 0x00000408},
	{0x40381274, 0x00000169},
	{0x40381278, 0x0000006e},
	{0x4038127c, 0x00000000},
	{0x40381280, 0x00000068},
	{0x40381284, 0x00000000},
	{0x40381288, 0x00000408},
	{0x4038128c, 0x00000169},
	{0x40381290, 0x00000000},
	{0x40381294, 0x00008310},
	{0x40381298, 0x00000168},
	{0x4038129c, 0x00000000},
	{0x403812a0, 0x0000a310},
	{0x403812a4, 0x00000168},
	{0x403812a8, 0x00001ff8},
	{0x403812ac, 0x000085a8},
	{0x403812b0, 0x000001e8},
	{0x403812b4, 0x00000068},
	{0x403812b8, 0x00000798},
	{0x403812bc, 0x0000016a},
	{0x403812c0, 0x00000078},
	{0x403812c4, 0x000007a0},
	{0x403812c8, 0x0000016a},
	{0x403812cc, 0x00000068},
	{0x403812d4, 0x00000790},
	{0x40380c08, 0x0000016a},
	{0x40380c20, 0x00000008},
	{0x40380c38, 0x00008b10},
	{0x40380c50, 0x00000168},
	{0x40380c68, 0x00000008},
	{0x40380c80, 0x0000ab10},
	{0x40380c98, 0x00000168},
	{0x40380cb0, 0x0000000a},
	{0x40380cc8, 0x00000408},
	{0x40380cd0, 0x00000169},
	{0x40380cd8, 0x00000058},
	{0x40380ce0, 0x00000000},
	{0x40380ce8, 0x00000068},
	{0x40380cf0, 0x00000000},
	{0x40380cf8, 0x00000408},
	{0x40380d00, 0x00000169},
	{0x40380d08, 0x00000000},
	{0x40380d10, 0x00008b10},
	{0x40380d18, 0x00000168},
	{0x40380d20, 0x00000001},
	{0x40380d24, 0x0000ab10},
	{0x40380d28, 0x00000168},
	{0x40380d2c, 0x00000000},
	{0x40380d34, 0x000001d8},
	{0x40380d3c, 0x00000169},
	{0x40380d44, 0x00000080},
	{0x40380d4c, 0x00000790},
	{0x40380d54, 0x0000016a},
	{0x40380d5c, 0x00000018},
	{0x40380d64, 0x000007aa},
	{0x40380d6c, 0x0000006a},
	{0x40380d74, 0x0000000a},
	{0x40380d7c, 0x00000000},
	{0x40380d84, 0x000001e9},
	{0x40380d8c, 0x00000008},
	{0x40380d94, 0x00008080},
	{0x40380d9c, 0x00000108},
	{0x40380da4, 0x0000000f},
	{0x40380dac, 0x00000408},
	{0x40380db4, 0x00000169},
	{0x40380dbc, 0x0000000c},
	{0x40380dc4, 0x00000000},
	{0x40380dcc, 0x00000068},
	{0x40380dd4, 0x00000009},
	{0x40380ddc, 0x00000000},
	{0x40380de4, 0x000001a9},
	{0x40380dec, 0x00000000},
	{0x40380df4, 0x00000408},
	{0x40380dfc, 0x00000169},
	{0x40380e04, 0x00000000},
	{0x40380e0c, 0x00008080},
	{0x40380e14, 0x00000108},
	{0x40380e1c, 0x00000008},
	{0x40380e24, 0x000007aa},
	{0x40380e2c, 0x0000006a},
	{0x40380e34, 0x00000000},
	{0x40380e3c, 0x00008568},
	{0x40380e44, 0x00000108},
	{0x40380e4c, 0x000000b7},
	{0x40380e54, 0x00000790},
	{0x40380e5c, 0x0000016a},
	{0x40380e64, 0x0000001f},
	{0x40380e6c, 0x00000000},
	{0x40380e74, 0x00000068},
	{0x40380e7c, 0x00000008},
	{0x40380e84, 0x00008558},
	{0x40380e8c, 0x00000168},
	{0x40380e94, 0x0000000f},
	{0x40380e9c, 0x00000408},
	{0x40380ea4, 0x00000169},
	{0x40380eac, 0x0000000d},
	{0x40380eb4, 0x00000000},
	{0x40380ebc, 0x00000068},
	{0x40380ec4, 0x00000000},
	{0x40380ecc, 0x00000408},
	{0x40380ed4, 0x00000169},
	{0x40380edc, 0x00000000},
	{0x40380ee4, 0x00008558},
	{0x40380eec, 0x00000168},
	{0x40380ef4, 0x00000008},
	{0x40380efc, 0x000003c8},
	{0x40380f04, 0x000001a9},
	{0x40380f0c, 0x00000003},
	{0x40380f14, 0x00000370},
	{0x40380f1c, 0x00000129},
	{0x40380f24, 0x00000020},
	{0x40380f2c, 0x000002aa},
	{0x40380f34, 0x00000009},
	{0x40380f3c, 0x00000008},
	{0x40380f44, 0x000000e8},
	{0x40380f4c, 0x00000109},
	{0x40380f54, 0x00000000},
	{0x40380f5c, 0x00008140},
	{0x40380f64, 0x0000010c},
	{0x40380f6c, 0x00000010},
	{0x40380f74, 0x00008138},
	{0x40380f7c, 0x00000104},
	{0x40380f84, 0x00000008},
	{0x40380f8c, 0x00000448},
	{0x40380f94, 0x00000109},
	{0x40380f9c, 0x0000000f},
	{0x40380fa4, 0x000007c0},
	{0x40380fac, 0x00000109},
	{0x40380fb4, 0x00000000},
	{0x40380fbc, 0x000000e8},
	{0x40380fc4, 0x00000109},
	{0x40380fcc, 0x00000047},
	{0x40380fd4, 0x00000630},
	{0x40380fdc, 0x00000109},
	{0x40380fe4, 0x00000008},
	{0x40380fec, 0x00000618},
	{0x40380ff4, 0x00000109},
	{0x40380ffc, 0x00000008},
	{0x40381004, 0x000000e0},
	{0x4038100c, 0x00000109},
	{0x40381014, 0x00000000},
	{0x4038101c, 0x000007c8},
	{0x40381024, 0x00000109},
	{0x4038102c, 0x00000008},
	{0x40381034, 0x00008140},
	{0x4038103c, 0x0000010c},
	{0x40381044, 0x00000000},
	{0x4038104c, 0x00000478},
	{0x40381054, 0x00000109},
	{0x4038105c, 0x00000000},
	{0x40381064, 0x00000001},
	{0x4038106c, 0x00000008},
	{0x40381074, 0x00000008},
	{0x4038107c, 0x00000004},
	{0x40381084, 0x00000000},
	{0x40380c84, 0x00000008},
	{0x40380c9c, 0x000007c8},
	{0x40380cb4, 0x00000109},
	{0x40380ccc, 0x00000000},
	{0x40380cd4, 0x00000400},
	{0x40380cdc, 0x00000106},
	{0x40380424, 0x00000400},
	{0x40380d30, 0x00000000},
	{0x40380d70, 0x00000029},
	{0x40380da8, 0x00000068},
	{0x40380314, 0x00000000},
	{0x40380318, 0x00000101},
	{0x4038031c, 0x00000105},
	{0x40380320, 0x00000107},
	{0x40380324, 0x0000010f},
	{0x40380328, 0x00000202},
	{0x4038032c, 0x0000020a},
	{0x40380330, 0x0000020b},
	{0x403814fc, 0x00000002},
	{0x40381330, 0x00000064},
	{0x40381340, 0x000000c8},
	{0x40381350, 0x000007d0},
	{0x40381360, 0x0000002c},
	{0x40380ce4, 0x00000000},
	{0x40380cec, 0x00000173},
	{0x40380cf4, 0x00000060},
	{0x40380cfc, 0x00006110},
	{0x40380d04, 0x00002152},
	{0x40380d0c, 0x0000dfbd},
	{0x40380d14, 0x00002060},
	{0x40380d1c, 0x00006152},
	{0x40381374, 0x0000005a},
	{0x40381388, 0x00000003},
	{0x40380200, 0x000000e0},
	{0x40380210, 0x00000012},
	{0x40380220, 0x000000e0},
	{0x40380230, 0x00000012},
	{0x40380240, 0x000000e0},
	{0x40380250, 0x00000012},
	{0x403803c8, 0x0000000f},
	{0x4039444c, 0x00000001},
	{0x40394450, 0x00000001},
	{0x40394454, 0x00000180},
	{0x40394460, 0x00000001},
	{0x40394434, 0x00006209},
	{0x40394884, 0x00000001},
	{0x403948d0, 0x00000001},
	{0x403948d4, 0x00000001},
	{0x403948d8, 0x00000001},
	{0x403948dc, 0x00000001},
	{0x403948e0, 0x00000001},
	{0x403948e4, 0x00000001},
	{0x403948e8, 0x00000001},
	{0x403948ec, 0x00000001},
	{0x4039644c, 0x00000001},
	{0x40396450, 0x00000001},
	{0x40396454, 0x00000180},
	{0x40396460, 0x00000001},
	{0x40396434, 0x00006209},
	{0x40396884, 0x00000001},
	{0x403968d0, 0x00000001},
	{0x403968d4, 0x00000001},
	{0x403968d8, 0x00000001},
	{0x403968dc, 0x00000001},
	{0x403968e0, 0x00000001},
	{0x403968e4, 0x00000001},
	{0x403968e8, 0x00000001},
	{0x403968ec, 0x00000001},
	{0x4039844c, 0x00000001},
	{0x40398450, 0x00000001},
	{0x40398454, 0x00000180},
	{0x40398460, 0x00000001},
	{0x40398434, 0x00006209},
	{0x40398884, 0x00000001},
	{0x403988d0, 0x00000001},
	{0x403988d4, 0x00000001},
	{0x403988d8, 0x00000001},
	{0x403988dc, 0x00000001},
	{0x403988e0, 0x00000001},
	{0x403988e4, 0x00000001},
	{0x403988e8, 0x00000001},
	{0x403988ec, 0x00000001},
	{0x4039a44c, 0x00000001},
	{0x4039a450, 0x00000001},
	{0x4039a454, 0x00000180},
	{0x4039a460, 0x00000001},
	{0x4039a434, 0x00006209},
	{0x4039a884, 0x00000001},
	{0x4039a8d0, 0x00000001},
	{0x4039a8d4, 0x00000001},
	{0x4039a8d8, 0x00000001},
	{0x4039a8dc, 0x00000001},
	{0x4039a8e0, 0x00000001},
	{0x4039a8e4, 0x00000001},
	{0x4039a8e8, 0x00000001},
	{0x4039a8ec, 0x00000001},
	{0x40381644, 0x00000001},
	{0x40381640, 0x00000019},
	{0x40380bec, 0x00000002},
};

size_t pie_cfg_size = ARRAY_SIZE(pie_cfg);
