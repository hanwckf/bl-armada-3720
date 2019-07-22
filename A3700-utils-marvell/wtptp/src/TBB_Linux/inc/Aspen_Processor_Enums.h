/*******************************************************************************
Copyright (C) 2017 Marvell International Ltd.

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the three
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, either version 2 of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************************
Marvell GNU General Public License FreeRTOS Exception

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the Lesser
General Public License Version 2.1 plus the following FreeRTOS exception.
An independent module is a module which is not derived from or based on
FreeRTOS.
Clause 1:
Linking FreeRTOS statically or dynamically with other modules is making a
combined work based on FreeRTOS. Thus, the terms and conditions of the GNU
General Public License cover the whole combination.
As a special exception, the copyright holder of FreeRTOS gives you permission
to link FreeRTOS with independent modules that communicate with FreeRTOS solely
through the FreeRTOS API interface, regardless of the license terms of these
independent modules, and to copy and distribute the resulting combined work
under terms of your choice, provided that:
1. Every copy of the combined work is accompanied by a written statement that
details to the recipient the version of FreeRTOS used and an offer by yourself
to provide the FreeRTOS source code (including any modifications you may have
made) should the recipient request it.
2. The combined work is not itself an RTOS, scheduler, kernel or related
product.
3. The independent modules add significant and primary functionality to
FreeRTOS and do not merely extend the existing functionality already present in
FreeRTOS.
Clause 2:
FreeRTOS may not be used for any competitive or comparative purpose, including
the publication of any form of run time or compile time metric, without the
express permission of Real Time Engineers Ltd. (this is the norm within the
industry and is intended to ensure information accuracy).

********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.

	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.

	* Neither the name of Marvell nor the names of its contributors may be
	  used to endorse or promote products derived from this software without
	  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/


#ifndef __ASPEN_PROCESSOR_ENUMS_H__
#define __ASPEN_PROCESSOR_ENUMS_H__

typedef enum ASPEN_CLOCK_ID_E {
	ASPEN_CLOCK_ID_MCU, ASPEN_CLOCK_ID_I2C,

	ASPEN_CLOCK_ID_E_MAX	// MUST BE FINAL ENUM IN ASPEN_CLOCK_ID_T
} ASPEN_CLOCK_ID_T;

typedef enum ASPEN_FREQ_ID_E {
	ASPEN_FREQ_ID_MCU,

	ASPEN_FREQ_ID_E_MAX	// MUST BE FINAL ENUM IN ASPEN_FREQ_ID_T
} ASPEN_FREQ_ID_T;

typedef enum ASPEN_OPMODE_E {
	ASPEN_OPMODE_CONSUMER_ID, // will be BootROM or BootLoader. If omitted, processed by both.
	ASPEN_OPMODE0_ID,             // 156,  PLL1 based
	ASPEN_OPMODE1_ID,             // 400,  PLL1 based
	ASPEN_OPMODE2_ID,			// 624,  PLL2 based - requires higher voltage
	ASPEN_OPMODE3_ID,			// 800,  PLL2 based - requires higher voltage
	ASPEN_OPMODE4_ID,			// 1066, PLL2 based - requires higher voltage
	ASPEN_OPMODE5_ID,			// 1200, PLL2 based - requires higher voltage
	ASPEN_OPMODE6_ID,			// 1600, PLL2 based - requires higher voltage

	ASPEN_OPMODE2_1_ID,           // 624, PLL1 based
	ASPEN_OPMODE2_2_ID,           // 624, PLL1 based
	ASPEN_OPMODE2_3_ID,           // 624, PLL1 based

	ASPEN_OPMODE3_1_ID,           // 800, PLL2 based
	ASPEN_OPMODE3_2_ID,           // 800, PLL2 based
	ASPEN_OPMODE3_3_ID,           // 800, PLL2 based

	// new modes
	ASPEN_OPMODE0_3_ID,			// 156,  PLL2 based, tight voltage tolerance
	ASPEN_OPMODE2_4_ID,			// 624,  PLL2 based
	ASPEN_OPMODE4_1_ID,			// 1066, PLL2 based

	ASPEN_OPMODE_MAX
} ASPEN_OPMODE_T;

typedef enum ASPEN_OPDIV_E {
	ASPEN_OPDIV_CONSUMER_ID,
	ASPEN_OPDIV_PLLSEL,
	ASPEN_OPDIV_AXI_PLLSEL,
	ASPEN_OPDIV_PLL2_REFDIV,
	ASPEN_OPDIV_PLL2_FBDIV,
	ASPEN_OPDIV_PCLK_DIV,
	ASPEN_OPDIV_DCLK_DIV,
	ASPEN_OPDIV_XPCLK_DIV,
	ASPEN_OPDIV_BACLK_DIV,
	ASPEN_OPDIV_ACLK_DIV,
	ASPEN_OPDIV_ACLK2_DIV,
	ASPEN_OPDIV_PLL2_REG1,
	ASPEN_OPDIV_MV,
	ASPEN_OPDIV_GO,

	ASPEN_OPDIV_MAX
} ASPEN_OPDIV_T;

typedef enum ASPEN_MCU_REGID_E {
	ASPEN_SDRREVREG_ID,			// revision
	ASPEN_SDRADCREG_ID,			// address decode
	ASPEN_SDRCFGREG0_ID,		// sdram config reg 0
	ASPEN_SDRCFGREG1_ID,		// sdram config reg 1
	ASPEN_SDRTMGREG1_ID,		// sdram timing reg 1
	ASPEN_SDRTMGREG2_ID,		// sdram timing reg 2
	ASPEN_SDRTMGREG3_ID,		// sdram timing reg 3
	ASPEN_SDRTMGREG4_ID,		// sdram timing reg 4
	ASPEN_SDRTMGREG5_ID,		// sdram timing reg 5
	ASPEN_SDRCTLREG1_ID,		// sdram control reg 1
	ASPEN_SDRCTLREG2_ID,		// sdram control reg 2
	ASPEN_SDRCTLREG3_ID,		// sdram control reg 3
	ASPEN_SDRCTLREG4_ID,		// sdram control reg 4
	ASPEN_SDRCTLREG5_ID,		// sdram control reg 5
	ASPEN_SDRCTLREG6_ID,		// sdram control reg 6
	ASPEN_SDRCTLREG7_ID,		// sdram control reg 7
	ASPEN_SDRCTLREG13_ID,		// sdram control reg 13
	ASPEN_SDRCTLREG14_ID,		// sdram control reg 14
	ASPEN_SDRERRREG_ID,			// sdram error status
	ASPEN_ADRMAPREG0_ID,		// address map cs0
	ASPEN_ADRMAPREG1_ID,		// address map cs1
	ASPEN_USRCMDREG0_ID,		// user initiated command registers
	ASPEN_SDRSTAREG_ID,			// sdram status register
	ASPEN_PHYCTLREG3_ID,		// phy control reg 3
	ASPEN_PHYCTLREG7_ID,		// phy control reg 7
	ASPEN_PHYCTLREG8_ID,		// phy control reg 8
	ASPEN_PHYCTLREG9_ID,		// phy control reg 9
	ASPEN_PHYCTLREG10_ID,		// phy control reg 10
	ASPEN_PHYCTLREG11_ID,		// phy control reg 11
	ASPEN_PHYCTLREG12_ID,		// phy control reg 12
	ASPEN_PHYCTLREG13_ID,		// phy control reg 13
	ASPEN_PHYCTLREG14_ID,		// phy control reg 14
	ASPEN_DLLCTLREG1_ID,		// dll control reg 1
	ASPEN_TSTMODREG0_ID,		// test mode reg 0
	ASPEN_TSTMODREG1_ID,		// test mode reg 1
	ASPEN_MCBCTLREG1_ID,		// mcb control reg 1
	ASPEN_MCBCTLREG2_ID,		// mcb control reg 2
	ASPEN_MCBCTLREG3_ID,		// mcb control reg 3
	ASPEN_MCBCTLREG4_ID,		// mcb control reg 4
	ASPEN_PRFCTLREG0_ID,		// perf control reg 0
	ASPEN_PRFCTLREG1_ID,		// perf control reg 1
	ASPEN_PRFSTAREG_ID,			// perf status reg
	ASPEN_PRFSELREG_ID,			// perf select reg
	ASPEN_PRFCNTREG_ID,			// perf count reg
	ASPEN_SDRTMGREG6_ID,		// sdram timing reg 6
	ASPEN_PHYCTLREGTST_ID,		// phy control reg test mode
	ASPEN_PRF_COUNTER_ID,		// performance counter reg
	ASPEN_OPDELAY_ID,			// operation delay reg
	ASPEN_OPREAD_ID,			// operation read reg

	ASPEN_MCU_REGID_E_MAX		// MUST BE FINAL ENUM IN ASPEN_MCU_REGID_T
} ASPEN_MCU_REGID_T;

#endif

