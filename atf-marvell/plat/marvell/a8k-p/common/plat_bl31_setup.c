/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <ap810_setup.h>
#include <cache_llc.h>
#include <cp110_setup.h>
#include <debug.h>
#include <marvell_pm.h>
#include <marvell_plat_priv.h>
#include <plat_marvell.h>

#define CCU_ROUT_OPT_DIS(ap, stop)		(MVEBU_A2_BANKED_STOP_BASE(ap, stop) + 0x8)
#define CCU_SFWD_UL_AC_EN_OFFSET		9
#define CCU_SFWD_PT_AC_EN_OFFSET		1

#define SMMU_S_ACR(ap)				(MVEBU_SMMU_BASE(ap) + 0x10)
#define SMMU_S_ACR_PG_64K			(1 << 16)

#define MVEBU_CCU_GSPMU_CR(ap)			(MVEBU_CCU_LOCL_CNTL_BASE(ap) + 0x3F0)
#define GSPMU_CPU_CONTROL			(0x1 << 0)

#define CCU_HTC_CR(ap)				(MVEBU_CCU_BASE(ap) + 0x200)
#define CCU_SET_POC_OFFSET			5

#define GEVENT_CR_PORTx_EVENT_MASK(ap, port)	(MVEBU_AR_RFU_BASE(ap) + 0x500 + port * 0x4)

#define MVEBU_CCU_MASTERS_IN_RING		16

#define CCU_HTC_ACR_MEM_AFF_GRP_OFFSET		0
#define CCU_HTC_ACR_DVM_AFF_GRP_OFFSET		16
#define CCU_HTC_ACR_GLOBAL_STOP_OFFSET		(0x1 << 9)

#define CCU_HTC_ASET_WA				(0x1 << 5)

/* Generic Timer System Controller */
#define MVEBU_MSS_GTCR_REG(ap)			(MVEBU_REGS_BASE_AP(ap) + 0x581000)
#define MVEBU_MSS_GTCR_ENABLE_BIT		0x1
#define MVEBU_MSS_GTCVLR_REG(ap)		(MVEBU_MSS_GTCR_REG(ap) + 0x8)
#define MVEBU_MSS_GTCVHR_REG(ap)		(MVEBU_MSS_GTCR_REG(ap) + 0xc)

/* We can move this to Linux driver, need to check this :) */
#define XOR_0_STREAM_ID_REG	(0x410010)
#define XOR_1_STREAM_ID_REG	(0x430010)
#define XOR_2_STREAM_ID_REG	(0x450010)
#define XOR_3_STREAM_ID_REG	(0x470010)

/* Max stream IDs per AP */
#define MAX_SID_PER_AP		(0X200)
/* Unlike CP110 (supports up to 8 bit stream IDs),
 * AP810 supports up to 12 bit stream IDs.
 * This stream ID base allows us to separate the CPs
 * stream IDs values from the AP stream IDs values.
 */
#define AP0_STREAM_ID_BASE		(0x100)
#define APx_STREAM_ID_BASE(ap_id)	(AP0_STREAM_ID_BASE + \
					(ap_id * MAX_SID_PER_AP))

uintptr_t ap_stream_id_reg[] = {
	XOR_0_STREAM_ID_REG,
	XOR_1_STREAM_ID_REG,
	XOR_2_STREAM_ID_REG,
	XOR_3_STREAM_ID_REG,
	0
};

/* Used for Units of AP-810 (e.g. SDIO and etc) */
enum axi_attr {
	AXI_SDIO_ATTR = 0,
	AXI_DFX_ATTR,
	AXI_EIP197_ATTR,
	AXI_MAX_ATTR,
};

static _Bool pm_fw_running;

/* Initialize the CP110 in all APs */
static void cp110_die_init(void)
{
	int ap_id, cp_id;

	for (ap_id = 0; ap_id < ap_get_count(); ap_id++)
		for (cp_id = 0; cp_id < ap810_get_cp_per_ap_cnt(ap_id); cp_id++)
			cp110_init(MVEBU_CP_REGS_BASE(ap_id, cp_id),
				   STREAM_ID_BASE +
				   (cp_id * MAX_STREAM_ID_PER_CP));

}

/*
 * When the chip powers up, no affinity group is defined. During boot the affinity
 * group should be created. Memory affinity group describes which elements are
 * snooped when memory transaction enters the coherency fabric.  The snooped
 * element should be the one that includes a memory cache - the CPU clusters.
 */
static void ap810_dvm_affinity(int ap_id)
{
	uint32_t dvm_val;
	int i;

	debug_enter();
	/* Configure the snoop topology for local affinity */
	dvm_val = (CCU_HTC_ACR_CLUSTERx_OFFSET(0) | CCU_HTC_ACR_CLUSTERx_OFFSET(1) |
		CCU_HTC_ACR_CLUSTERx_OFFSET(2) | CCU_HTC_ACR_CLUSTERx_OFFSET(3) |
		CCU_HTC_ACR_GLOBAL_STOP_OFFSET);
	dvm_val |= dvm_val << CCU_HTC_ACR_DVM_AFF_GRP_OFFSET;
	for (i = 0; i < MVEBU_CCU_MASTERS_IN_RING; i++)
		mmio_write_32(CCU_HTC_ACR(ap_id, i), dvm_val);

	/* TODO: add errata for this WA */
	/*
	 * Workaround, bug in which DVM that inserts Sg fom global ring must not
	 * have affinity=0. We open the Sio of the SMMU (AURORA2-1615)
	 */
	mmio_write_32(CCU_HTC_ASET(ap_id), CCU_HTC_ACR_GLOBAL_STOP_OFFSET | CCU_HTC_ASET_WA);

	/* Configure the snop topology for global affinity */
	for (i = 0; i < MVEBU_CCU_MASTERS_IN_RING; i++)
		mmio_write_32(CCU_HTC_GACR(ap_id, i), AP810_MAX_AP_MASK);

	mmio_write_32(CCU_HTC_GASET(ap_id),
			AP810_MAX_AP_MASK >> (AP810_MAX_AP_NUM - ap_get_count()));

	debug_exit();
}

static void ap810_setup_smmu(int ap)
{
	uint32_t reg;

	debug_enter();

	/* Set the SMMU page size to 64 KB */
	reg = mmio_read_32(SMMU_S_ACR(ap));
	reg |= SMMU_S_ACR_PG_64K;
	mmio_write_32(SMMU_S_ACR(ap), reg);

	debug_exit();
}

static void ap810_sec_masters_access_en(int ap, uint32_t enable)
{
	debug_enter();
	INFO("place holder to implement %s\n", __func__);
	debug_exit();
}

static void ap810_axi_attr_init(int ap)
{
	uint32_t index, data;

	/* Initialize AXI attributes for AP810
	 * Go over the AXI attributes and set
	 * Ax-Cache and Ax-Domain
	 */
	debug_enter();
	for (index = 0; index < AXI_MAX_ATTR; index++) {
		switch (index) {
		/* DFX works with no coherent only -
		 * there's no option to configure the
		 * Ax-Cache and Ax-Domain
		 */
		case AXI_DFX_ATTR:
			continue;
		default:
			/* Set Ax-Cache as cacheable, no allocate, modifiable,
			 * bufferable the values are different because Read & Write
			 * definition is different in Ax-Cache
			 */
			data = mmio_read_32(MVEBU_AP_AXI_ATTR_REG(ap, index));
			data &= ~MVEBU_AXI_ATTR_ARCACHE_MASK;
			data |= (CACHE_ATTR_WRITE_ALLOC | CACHE_ATTR_CACHEABLE | CACHE_ATTR_BUFFERABLE)
				<< MVEBU_AXI_ATTR_ARCACHE_OFFSET;
			data &= ~MVEBU_AXI_ATTR_AWCACHE_MASK;
			data |= (CACHE_ATTR_READ_ALLOC | CACHE_ATTR_CACHEABLE | CACHE_ATTR_BUFFERABLE)
				<< MVEBU_AXI_ATTR_AWCACHE_OFFSET;
			/* Set Ax-Domain as Outer domain */
			data &= ~MVEBU_AXI_ATTR_ARDOMAIN_MASK;
			data |= DOMAIN_OUTER_SHAREABLE << MVEBU_AXI_ATTR_ARDOMAIN_OFFSET;
			data &= ~MVEBU_AXI_ATTR_AWDOMAIN_MASK;
			data |= DOMAIN_OUTER_SHAREABLE << MVEBU_AXI_ATTR_AWDOMAIN_OFFSET;
			mmio_write_32(MVEBU_AP_AXI_ATTR_REG(ap, index), data);
		}
	}

	debug_exit();
	return;
}

static void ap810_init_aurora2(int ap_id)
{
	unsigned int reg;
	int stop;

	debug_enter();

	/* Open access to another AP configuration */
	ap810_setup_banked_rgf(ap_id);

	/*
	 * For now - it's relevant for A0 only, they may change the
	 * default configuration for B0
	 */
	if (ap810_rev_id_get(ap_id) == MVEBU_AP810_REV_ID_A0) {
		/* Enable store-and-forward buffer & up-link */
		for (stop = 0; stop < AP810_S_END; stop++) {
			reg = mmio_read_32(CCU_ROUT_OPT_DIS(ap_id, stop));
			reg &= ~((1 << CCU_SFWD_UL_AC_EN_OFFSET) | (1 << CCU_SFWD_PT_AC_EN_OFFSET));
			reg |= ((1 << CCU_SFWD_UL_AC_EN_OFFSET) | (1 << CCU_SFWD_PT_AC_EN_OFFSET));
			mmio_write_32(CCU_ROUT_OPT_DIS(ap_id, stop), reg);
		}
	}

	/* Enable CPU control over SPMU registers */
	reg = mmio_read_32(MVEBU_CCU_GSPMU_CR(ap_id));
	reg |= GSPMU_CPU_CONTROL;
	mmio_write_32(MVEBU_CCU_GSPMU_CR(ap_id), reg);

#if LLC_ENABLE
	/* Enable LLC in exclusive mode */
	llc_enable(ap_id, 1);
#endif /* LLC_ENABLE */

	/* Set point of coherency to DDR. This is
	 * required by units which have SW cache coherency
	 */
	reg = mmio_read_32(CCU_HTC_CR(ap_id));
	reg |= (0x1 << CCU_SET_POC_OFFSET);
	mmio_write_32(CCU_HTC_CR(ap_id), reg);

	ap810_dvm_affinity(ap_id);

	debug_exit();
}

static void ap810_stream_id_init(int ap_id)
{
	uintptr_t base = MVEBU_REGS_BASE_AP(ap_id);
	int i = 0;
	uint32_t stream_id = APx_STREAM_ID_BASE(ap_id);


	debug_enter();

	/* Initialize Func stream ID & intrrupt stream ID */
	while (ap_stream_id_reg[i]) {
		mmio_write_32(base + ap_stream_id_reg[i++], stream_id << 16 | stream_id);
		stream_id++;
	}

	debug_exit();
}

/* Setup events that controls the propagation
 * of CPU event between dies.
 */
static void ap810_setup_events(int ap_id)
{
	debug_enter();
	INFO("Event propegation setup for AP%d\n", ap_id);
	/* The index of the register represents the destination port.
	 * The bit number represents the source to be masked.
	 * All sources which are unmasked will be ORed and sent to the
	 * destination port.
	 *
	 * For Quad AP the connectios is:
	 *  AP0: port 0 -> AP3, port 1 -> NC, port 2 -> AP2, port 3 -> AP1
	 *  AP1: port 0 -> AP3, port 1 -> AP0, port 2 -> AP2, port 3 -> NC
	 *  AP2: port 0 -> AP1, port 1 -> NC, port 2 -> AP0, port 3 -> AP3
	 *  AP3: port 0 -> AP2, port 1 -> AP2, port 2 -> AP0, port 3 -> NC
	 *
	 * For Dual AP the connection is:
	 *  AP0: port 2 -> AP1
	 *  AP1: port 0 -> AP0
	 */
	switch (ap_id) {
	case 0:
		if (ap_get_count() == 2) {
			/* Port 2 - unmask local GEvent */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 2), 0x2f);
			/* Port 4 (Local) - unmask Port 2 */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 4), 0x3b);
		} else {
			/* Port 0 - unmask local GEvent */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 0), 0x2f);
			/* Port 2 - unmask local GEvent */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 2), 0x2f);
			/* Port 3 - unmask local GEvent */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 3), 0x2f);
			/* Port 4 (Local) - unmask Port 0/2/3 */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 4), 0x32);
		}
		break;
	case 1:
		if (ap_get_count() == 2) {
			/* Port 0 - unmask local GEvent */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 0), 0x2f);
			/* Port 4 (Local) - unmask Port 0 */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 4), 0x3e);
		} else {
			/* Port 0 - unmask local GEvent */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 0), 0x2f);
			/* Port 1 - unmask local GEvent */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 1), 0x2f);
			/* Port 2 - unmask local GEvent  */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 2), 0x2f);
			/* Port 4 (Local) - unmask Port 0/1/2 */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 4), 0x38);
		}
		break;
	case 2:
			/* Port 0 - unmask local GEvent */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 0), 0x2f);
			/* Port 2 - unmask local GEvent */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 2), 0x2f);
			/* Port 3 - unmask local GEvent */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 3), 0x2f);
			/* Port 4 (Local) - unmask Port 0/2/3 */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 4), 0x32);
		break;
	case 3:
			/* Port 0 - unmask local GEvent */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 0), 0x2f);
			/* Port 1 - unmask local GEvent */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 1), 0x2f);
			/* Port 2 - unmask local GEvent */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 2), 0x2f);
			/* Port 4 (Local) - unmask Port 0/1/2 */
			mmio_write_32(GEVENT_CR_PORTx_EVENT_MASK(ap_id, 4), 0x38);
		break;
	}
	debug_exit();
}

static void ap810_generic_timer_init(void)
{
	uint32_t reg, i;

	debug_enter();

	/* This code initializes the timer of the SoC,
	 * in case of single AP, the bootrom initializes the timer.
	 * In this code, we re-initialize the timer in the all APs.
	 * TODO: move this code to be timer calibration algorithm
	 */
	for (i = 0; i < ap_get_count(); i++) {
		/* Disable timer */
		reg = mmio_read_32(MVEBU_MSS_GTCR_REG(i));
		reg &= ~MVEBU_MSS_GTCR_ENABLE_BIT;
		mmio_write_32(MVEBU_MSS_GTCR_REG(i), reg);
		/* Set Zero to value high register */
		mmio_write_32(MVEBU_MSS_GTCVLR_REG(i), 0x0);
		/* Set Zero to value low register */
		mmio_write_32(MVEBU_MSS_GTCVHR_REG(i), 0x0);
	}

	if (ap_get_count() == 2) {
		/* Enable timer */
		mmio_write_32(MVEBU_MSS_GTCR_REG(0), MVEBU_MSS_GTCR_ENABLE_BIT);
		mmio_write_32(MVEBU_MSS_GTCR_REG(1), MVEBU_MSS_GTCR_ENABLE_BIT);

#if LOG_LEVEL >= LOG_LEVEL_INFO
		unsigned int ap0, ap1;
		ap0 = mmio_read_32(MVEBU_MSS_GTCVLR_REG(0));
		ap1 = mmio_read_32(MVEBU_MSS_GTCVLR_REG(1));
		INFO("Read time AP0 = %x - AP1 = %x\n", ap0, ap1);
#endif
	} else if (ap_get_count() == 4) {
		mmio_write_32(MVEBU_MSS_GTCR_REG(0), MVEBU_MSS_GTCR_ENABLE_BIT);
		mmio_write_32(MVEBU_MSS_GTCR_REG(1), MVEBU_MSS_GTCR_ENABLE_BIT);
		mmio_write_32(MVEBU_MSS_GTCR_REG(2), MVEBU_MSS_GTCR_ENABLE_BIT);
		mmio_write_32(MVEBU_MSS_GTCR_REG(3), MVEBU_MSS_GTCR_ENABLE_BIT);

#if LOG_LEVEL >= LOG_LEVEL_INFO
		unsigned int ap0, ap1, ap2, ap3;
		ap0 = mmio_read_32(MVEBU_MSS_GTCVLR_REG(0));
		ap1 = mmio_read_32(MVEBU_MSS_GTCVLR_REG(1));
		ap2 = mmio_read_32(MVEBU_MSS_GTCVLR_REG(2));
		ap3 = mmio_read_32(MVEBU_MSS_GTCVLR_REG(3));
		INFO("Read time AP0 = %x - AP1 = %x - AP2 = %x - AP3 = %x\n", ap0, ap1, ap2, ap3);
#endif
	} else {
		mmio_write_32(MVEBU_MSS_GTCR_REG(0), MVEBU_MSS_GTCR_ENABLE_BIT);
	}

	debug_exit();
}

static void ap810_bl31_init(void)
{
	int ap_id;

	debug_enter();

	for (ap_id = 0; ap_id < ap_get_count(); ap_id++) {
		INFO("Initialize AP-%d\n", ap_id);
		/* Setup Aurora2. */
		ap810_init_aurora2(ap_id);
		/* configure the SMMU */
		ap810_setup_smmu(ap_id);
		/* Open AP incoming access for all masters */
		ap810_sec_masters_access_en(ap_id, 1);
		/* configure axi for AP */
		ap810_axi_attr_init(ap_id);
		/* Setup events */
		ap810_setup_events(ap_id);
		/* Setup stream-id */
		ap810_stream_id_init(ap_id);
	}

	ap810_generic_timer_init();

	debug_exit();
}

_Bool is_pm_fw_running(void)
{
	return pm_fw_running;
}

/* This function overruns the same function in marvell_bl31_setup.c */
void bl31_plat_arch_setup(void)
{
	uintptr_t *mailbox = (void *)PLAT_MARVELL_MAILBOX_BASE;

	/* initialize the timer for mdelay/udelay functionality */
	plat_delay_timer_init();

	/* Configure ap810 */
	ap810_bl31_init();

	/* Configure the connected CP110 (if any) */
	cp110_die_init();

	/* In marvell_bl31_plat_arch_setup, el3 mmu is configured.
	 * el3 mmu configuration MUST be called after ap810_init, if not,
	 * this will cause an hang in init_io_win
	 * (after setting the IO windows GCR values).
	 */
	if (mailbox[MBOX_IDX_MAGIC] != MVEBU_MAILBOX_MAGIC_NUM ||
	    mailbox[MBOX_IDX_SUSPEND_MAGIC] != MVEBU_MAILBOX_SUSPEND_STATE)
		marvell_bl31_plat_arch_setup();
}
