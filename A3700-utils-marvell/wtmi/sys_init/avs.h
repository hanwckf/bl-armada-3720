/*
* ***************************************************************************
* Copyright (C) 2017 Marvell International Ltd.
* ***************************************************************************
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* Neither the name of Marvell nor the names of its contributors may be used
* to endorse or promote products derived from this software without specific
* prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
***************************************************************************
*/

#ifndef __AVS_H_
#define __AVS_H_

/* AVS Register */
#define MVEBU_AVS_REGS_BASE		(APPLICATION_CPU_REGS_BASE + 0x11500)
#define MVEBU_AVS_CONTROL0		(MVEBU_AVS_REGS_BASE + 0x0)
#define AVS_SOFT_RST_BIT		BIT31
#define AVS_ENABLE_BIT			BIT30
#define AVS_PAUSE_BIT			BIT29
#define SEL_VSENSE0_BIT			BIT28
#define HIGH_VDD_LIMIT_OFF		(16)
#define LOW_VDD_LIMIT_OFF		(22)
#define MVEBU_AVS_CONTROL1		(MVEBU_AVS_REGS_BASE + 0x4)
#define MVEBU_AVS_CONTROL2		(MVEBU_AVS_REGS_BASE + 0x8)
#define MVEBU_AVS_CONTROL3		(MVEBU_AVS_REGS_BASE + 0xC)
#define MVEBU_AVS_CONTROL4		(MVEBU_AVS_REGS_BASE + 0x10)
#define MVEBU_AVS_CONTROL5		(MVEBU_AVS_REGS_BASE + 0x14)
#define MVEBU_AVS_COLLECT_CTRL		(MVEBU_AVS_REGS_BASE + 0x18)

/* NB OTP Register */
#define MVEBU_NORTH_OTP_REGS_BASE	(APPLICATION_CPU_REGS_BASE + 0x12600)
#define MVEBU_NORTH_OTP_CTRL		(MVEBU_NORTH_OTP_REGS_BASE + 0x0)
#define OTP_MODE_BIT			(BIT15)
#define OTP_RPTR_RST_BIT		(BIT14)
#define OTP_POR_B_BIT			(BIT13)
#define OTP_SCLK_BIT			(BIT8)
#define OTP_PRDT_BIT			(BIT3)
#define OTP_CSB_BIT			(BIT2)
#define OTP_WRPROT_BIT			(BIT1)
#define OTP_PGM_BIT			(BIT0)
#define MVEBU_NORTH_OTP_RD_PORT		(MVEBU_NORTH_OTP_REGS_BASE + 0x4)
#define MVEBU_NORTH_OTP_RD_POINTER	(MVEBU_NORTH_OTP_REGS_BASE + 0x8)
#define OTP_PTR_INC_BIT			(BIT8)

enum otp_data_id {
	OTP_DATA_SVC_REV_ID = 0,
	OTP_DATA_SVC_SPEED_ID,
	OTP_DATA_SVC_RESERVED_ID,
	OTP_DATA_MAX,
};

enum otp_svc_speed {
	OTP_SVC_SPEED_600 = 600,
	OTP_SVC_SPEED_800 = 800,
	OTP_SVC_SPEED_1000 = 1000,
	OTP_SVC_SPEED_1200 = 1200,
};

enum otp_svc_speed_off {
	OTP_SVC_SPEED_600_OFF = 18,
	OTP_SVC_SPEED_800_OFF = 12,
	OTP_SVC_SPEED_1000_OFF = 6,
	OTP_SVC_SPEED_1200_OFF = 0,
};

enum avs_default_value {
	VAS_800M_DEFAULT_VALUE = 0x1F, /* 1100mV */
	VAS_1000M_DEFAULT_VALUE = 0x23, /* 1150mV */
	VAS_1200M_DEFAULT_VALUE = 0x27, /* 1200mV */
};

#define SVC_REVISION_2		(2)
#define OTP_SVC_REV_OFFSET	(8)
#define OTP_SVC_REV_MASK	(0xF)

#define AVS_VDD_BASE		(0xD) /* Base voltage is 0.898V(0xd) */
#define AVS_VDD_MASK		(0x3F)

int init_avs(u32 speed);

#endif /* __AVS_H_ */
