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

#ifndef __CLOCK_H_
#define __CLOCK_H_

/* Clocks */
#define MVEBU_NORTH_CLOCK_REGS_BASE	(APPLICATION_CPU_REGS_BASE + 0x13000)
#define MVEBU_SOUTH_CLOCK_REGS_BASE	(APPLICATION_CPU_REGS_BASE + 0x18000)
#define MVEBU_TESTPIN_NORTH_REG_BASE	(MVEBU_NORTH_CLOCK_REGS_BASE + 0x800)

/****************/
/* North Bridge */
/****************/
#define MVEBU_NORTH_BRG_PLL_BASE		(MVEBU_NORTH_CLOCK_REGS_BASE + 0x200)
#define MVEBU_NORTH_BRG_TBG_CFG			(MVEBU_NORTH_BRG_PLL_BASE + 0x0)
#define MVEBU_NORTH_BRG_TBG_CTRL0		(MVEBU_NORTH_BRG_PLL_BASE + 0x4)
#define MVEBU_NORTH_BRG_TBG_CTRL1		(MVEBU_NORTH_BRG_PLL_BASE + 0x8)
#define MVEBU_NORTH_BRG_TBG_CTRL2		(MVEBU_NORTH_BRG_PLL_BASE + 0xC)
#define MVEBU_NORTH_BRG_TBG_CTRL3		(MVEBU_NORTH_BRG_PLL_BASE + 0x10)
#define MVEBU_NORTH_BRG_TBG_CTRL4		(MVEBU_NORTH_BRG_PLL_BASE + 0x14)
#define MVEBU_NORTH_BRG_TBG_CTRL5		(MVEBU_NORTH_BRG_PLL_BASE + 0x18)
#define MVEBU_NORTH_BRG_TBG_CTRL6		(MVEBU_NORTH_BRG_PLL_BASE + 0x1C)
#define MVEBU_NORTH_BRG_TBG_CTRL7		(MVEBU_NORTH_BRG_PLL_BASE + 0x20)
#define MVEBU_NORTH_BRG_TBG_CTRL8		(MVEBU_NORTH_BRG_PLL_BASE + 0x30)

#define MVEBU_NORTH_CLOCK_TBG_SELECT_REG	(MVEBU_NORTH_CLOCK_REGS_BASE + 0x0)
#define MVEBU_NORTH_CLOCK_DIVIDER_SELECT0_REG	(MVEBU_NORTH_CLOCK_REGS_BASE + 0x4)
#define MVEBU_NORTH_CLOCK_DIVIDER_SELECT1_REG	(MVEBU_NORTH_CLOCK_REGS_BASE + 0x8)
#define MVEBU_NORTH_CLOCK_DIVIDER_SELECT2_REG	(MVEBU_NORTH_CLOCK_REGS_BASE + 0xC)
#define MVEBU_NORTH_CLOCK_SELECT_REG		(MVEBU_NORTH_CLOCK_REGS_BASE + 0x10)
#define NB_CLK_DIV0_MASK_ALL			(BIT31 | (0x3 << 26) | BIT19 | (0x3F << 6))
#define NB_CLK_DIV1_MASK_ALL			((0x3 << 30) | (0x7 << 21))
#define NB_CLK_DIV2_MASK_ALL			(BIT31 | (0xF << 3))
#define NB_CLK_TBG_SEL_MASK_ALL			(0x3F << 26)

/* north bridge clock enable register */
#define MVEBU_NORTH_CLOCK_ENABLE_REG		(MVEBU_NORTH_CLOCK_REGS_BASE + 0x14)
#define NB_ALL_CLK_ENABLE			(0x0)

/****************/
/*South Bridge */
/****************/
/* south bridge clock enable register */
#define MVEBU_SOUTH_CLOCK_TBG_SELECT_REG	(MVEBU_SOUTH_CLOCK_REGS_BASE + 0x0)
#define MVEBU_SOUTH_CLOCK_DIVIDER_SELECT0_REG	(MVEBU_SOUTH_CLOCK_REGS_BASE + 0x4)
#define MVEBU_SOUTH_CLOCK_DIVIDER_SELECT1_REG	(MVEBU_SOUTH_CLOCK_REGS_BASE + 0x8)
#define MVEBU_SOUTH_CLOCK_DIVIDER_SELECT2_REG	(MVEBU_SOUTH_CLOCK_REGS_BASE + 0xC)
#define MVEBU_SOUTH_CLOCK_SELECT_REG		(MVEBU_SOUTH_CLOCK_REGS_BASE + 0x10)
#define SB_CLK_TBG_SEL_MASK_ALL			((0x3FF << 22) | (0x3 << 12) | 0x3F)
#define SB_CLK_DIV0_MASK_ALL			((0x1F << 27) | 0x7)
#define SB_CLK_DIV1_MASK_ALL			((0xFF << 24) | (0x7 << 15) | 0x3F)
#define SB_CLK_DIV2_MASK_ALL			((0xFFFFF << 12) | 0x3F)

/* south bridge clock enable register */
#define MVEBU_SOUTH_CLOCK_ENABLE_REG		(MVEBU_SOUTH_CLOCK_REGS_BASE + 0x14)
/* (GBE0 and GBE1 have reversed logic) */
#define SB_ALL_CLK_ENABLE			(BIT19 | BIT20)

/*
 * North Bridge Clocks
 */
/* North Bridge clock line selectors (0x13000) */
struct nb_clock_sel {
	enum clock_line mmc_pclk_sel;		/* [1:0] */
	enum clock_line sata_host_pclk_sel;	/* [3:2] */
	enum clock_line sec_at_pclk_sel;	/* [5:4] */
	enum clock_line sec_dap_pclk_sel;	/* [7:6] */
	enum clock_line tsecm_pclk_sel;		/* [9:8] */
	enum clock_line setm_tmx_pclk_sel;	/* [11:10] */
	enum clock_line sqf_pclk_sel;		/* [13:12] */
	enum clock_line pwm_pclk_sel;		/* [15:14] */
	enum clock_line ddr_fclk_pclk_sel;	/* [17:16] */
	enum clock_line trace_pclk_sel;		/* [19:18] */
	enum clock_line counter_pclk_sel;	/* [21:20] */
	enum clock_line a53_cpu_pclk_sel;	/* [23:22] */
	enum clock_line eip97_pclk_sel;		/* [25:24] */
};

/* North Bridge clock divider 0 (0x13004) */
struct nb_clock_div_0 {
	u8 pwm_clk_prscl2;	/* [2:0] */
	u8 pwm_clk_prscl1;	/* [5:3] */
	u8 ddr_fclk_clk_prscl2;	/* [14:12] */
	u8 ddr_fclk_clk_prscl1;	/* [17:15] */
	u8 ddr_phy_mck_div_sel;	/* [18] */
	u8 trace_div_sel;	/* [22:20] */
	u8 counter_clk_prscl;	/* [25:23] */
	u8 a53_cpu_clk_prscl;	/* [30:28] */
};

/* North Bridge clock divider 1 (0x13008) */
struct nb_clock_div_1 {
	u8 sec_at_clk_prscl2;	/* [2:0] */
	u8 sec_at_clk_prscl1;	/* [5:3] */
	u8 sec_dap_clk_prscl2;	/* [8:6] */
	u8 sec_dap_clk_prscl1;	/* [11:9] */
	u8 tsecm_clk_prscl2;	/* [14:12] */
	u8 tsecm_clk_prscl1;	/* [17:15] */
	u8 setm_tmx_clk_prscl;	/* [20:18] */
	u8 sqf_clk_prscl2;	/* [26:24] */
	u8 sqf_clk_prscl1;	/* [29:27] */
};

/* North Bridge clock divider 2 (0x1300C) */
struct nb_clock_div_2 {
	u8 wc_ahb_div_sel;	/* [2:0] */
	u8 sata_h_clk_prscl2;	/* [9:7] */
	u8 sata_h_clk_prscl1;	/* [12:10] */
	u8 mmc_clk_prscl2;	/* [15:13] */
	u8 mmc_clk_prscl1;	/* [18:16] */
	u8 eip97_clk_prscl2;	/* [21:19] */
	u8 eip97_clk_prscl1;	/* [24:22] */
	u8 atb_clk_div_sel;	/* [26:25] */
	u8 cpu_cnt_clk_div_sel;	/* [28:27] */
	u8 plkdbg_clk_div_sel;	/* [30:29] */
};

/* North Bridge clock configuration */
struct nb_clock_cfg {
	struct nb_clock_sel	clock_sel;
	struct nb_clock_div_0	div0;
	struct nb_clock_div_1	div1;
	struct nb_clock_div_2	div2;
};

/*
 * South Bridge Clocks
 */
/* South Bridge clock line selectors (0x18000) */
struct sb_clock_sel {
	enum clock_line gbe_50_pclk_sel;		/* [7:6] */
	enum clock_line gbe_core_pclk_sel;		/* [9:8] */
	enum clock_line gbe_125_pclk_sel;		/* [11:10] */
	enum clock_line sdio_pclk_sel;			/* [15:14] */
	enum clock_line usb32_usb2_sys_pclk_sel;	/* [17:16] */
	enum clock_line usb32_ss_sys_pclk_sel;		/* [19:18] */
	enum clock_line sb_axi_pclk_sel;		/* [21:20] */
};

/* South Bridge clock divider 0 (0x18004) */
struct sb_clock_div_0 {
	u8 sdio_clk_prscl2;		/* [5:3] */
	u8 sdio_clk_prscl1;		/* [8:6] */
	u8 usb32_usb2_sys_clk_prscl2;	/* [11:9] */
	u8 usb32_usb2_sys_clk_prscl1;	/* [14:12] */
	u8 usb32_ss_sys_clk_prscl2;	/* [17:15] */
	u8 usb32_ss_sys_clk_prscl1;	/* [20:18] */
	u8 sb_axi_clk_prscl2;		/* [23:21] */
	u8 sb_axi_clk_prscl1;		/* [24:24] */
};

/* South Bridge clock divider 1 (0x18008) */
struct sb_clock_div_1 {
	u8 gbe_125_clk_prscl2;		/* [8:6] */
	u8 gbe_125_clk_prscl1;		/* [11:9] */
	u8 gbe_bm_core_clk_div2_sel;	/* [12] */
	u8 gbe1_core_clk_div2_sel;	/* [13] */
	u8 gbe0_core_clk_div2_sel;	/* [14] */
	u8 gbe_core_clk_prscl2;		/* [20:18] */
	u8 gbe_core_clk_prscl1;		/* [23:21] */
};

/* South Bridge clock divider 2 (0x1800C) */
struct sb_clock_div_2 {
	u8 gbe_50_clk_prscl2;		/* [8:6] */
	u8 gbe_50_clk_prscl1;		/* [11:9] */
};

/* South Bridge clock configuration */
struct sb_clock_cfg {
	struct sb_clock_sel	clock_sel;
	struct sb_clock_div_0	div0;
	struct sb_clock_div_1	div1;
	struct sb_clock_div_2	div2;
};

struct tbg_cfg {
	u32	kvco_mhz;
	u32	se_vcodiv;
	u32	diff_vcodiv;
};

/* System-wide clock configuration */
struct clock_cfg {
	u32			cpu_freq_mhz;
	u32			ddr_freq_mhz;
	struct tbg_cfg		tbg_a;
	struct tbg_cfg		tbg_b;
	struct nb_clock_cfg	nb_clk_cfg;
	struct sb_clock_cfg	sb_clk_cfg;
};

enum ssc_mode {
	CENTER_SPREAD = 0,
	DOWN_SPREAD
};

enum ssc_offset_mode {
	freq_down = 0,
	freq_up
};

struct ssc_cfg {
	enum ssc_mode		mode;
	u32			mod_freq; /* hz */
	u32			amp_freq; /* hz */
	u32			offset;
	enum ssc_offset_mode	offset_mode;
};

enum clk_preset {
	CLK_PRESET_CPU600_DDR600  = 0,
	CLK_PRESET_CPU800_DDR800,
	CLK_PRESET_CPU1000_DDR800,
	CLK_PRESET_CPU1200_DDR750,
	CLK_PRESET_MAX,
};

int set_clock_preset(enum clk_preset idx);
int get_cpu_clock(void);
int get_ddr_clock(void);
int setup_clock_tree(void);

#endif /* __CLOCK_H_ */
