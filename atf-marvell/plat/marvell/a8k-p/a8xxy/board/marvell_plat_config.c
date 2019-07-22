/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <armada_common.h>
#include <ap810_setup.h>
#include <mvebu_def.h>

/* This array describe how CPx connect to APx, via which MCI interface
 * For AP0: CP0 connected via MCI3
 *          CP1 connected via MCI2
 *          CP2 connected via MCI1
 *          CP3 connected via MCI0
 * For AP1: CP0 connected via MCI7
 *          CP1 connected via MCI6
 *          CP2 connected via MCI5
 *          CP3 connected via MCI4
 */
/* TODO - add mapping for AP2 and AP3 interconnection */
int ap0_mci_connect_cps[] = {MCI_3_TID, MCI_2_TID, MCI_1_TID, MCI_0_TID};
int ap1_mci_connect_cps[] = {MCI_7_TID, MCI_6_TID, MCI_5_TID, MCI_4_TID};

/* Return the MCI index that connect cp_id in ap_id */
int marvell_get_mci_map(int ap_id, int cp_id)
{
	if (ap_id == 1)
		return ap1_mci_connect_cps[cp_id];
	else
		return ap0_mci_connect_cps[cp_id];
}

/*****************************************************************************
 * GWIN Configuration
 *****************************************************************************
 */
/* Up to 2 interconnected APs maps */
struct addr_map_win gwin_memory_map2_ap0[] = {
	{0x4000000000,	0x4000000000,	0x1}, /* AP1 DRAM - 256GB */
	{0x9e00000000,	0x1d00000000,	0x1}, /* AP1 IO - 116GB */
};
struct addr_map_win gwin_memory_map2_ap1[] = {
	{0x0000000000,	0x4000000000,	0x0}, /* AP0 DRAM - 256GB */
	{0x8100000000,	0x1d00000000,	0x0}, /* AP0 IO - 116GB */
};
/* 3 to 4 interconnected APs maps */
struct addr_map_win gwin_memory_map4_ap0[] = {
	{0x2000000000,	0x2000000000,	0x1}, /* AP1 DRAM - 128GB */
	{0x4000000000,	0x2000000000,	0x2}, /* AP2 DRAM - 128GB */
	{0x6000000000,	0x2000000000,	0x3}, /* AP3 DRAM - 128GB */
	{0x9e00000000,	0x1d00000000,	0x1}, /* AP1 IO - 116GB */
	{0xbb00000000,	0x1d00000000,	0x2}, /* AP2 IO - 116GB */
	{0xd800000000,	0x1d00000000,	0x3}, /* AP3 IO - 116GB */
};
struct addr_map_win gwin_memory_map4_ap1[] = {
	{0x0000000000,	0x2000000000,	0x0}, /* AP0 DRAM - 128GB */
	{0x4000000000,	0x2000000000,	0x2}, /* AP2 DRAM - 128GB */
	{0x6000000000,	0x2000000000,	0x3}, /* AP3 DRAM - 128GB */
	{0x8100000000,	0x1d00000000,	0x0}, /* AP0 IO - 116GB */
	{0xbb00000000,	0x1d00000000,	0x2}, /* AP2 IO - 116GB */
	{0xd800000000,	0x1d00000000,	0x3}, /* AP3 IO - 116GB */
};
struct addr_map_win gwin_memory_map4_ap2[] = {
	{0x0000000000,	0x2000000000,	0x0}, /* AP0 DRAM - 128GB */
	{0x2000000000,	0x2000000000,	0x1}, /* AP1 DRAM - 128GB */
	{0x6000000000,	0x2000000000,	0x3}, /* AP3 DRAM - 128GB */
	{0x8100000000,	0x1d00000000,	0x0}, /* AP0 IO - 116GB */
	{0x9e00000000,	0x1d00000000,	0x1}, /* AP1 IO - 116GB */
	{0xd800000000,	0x1d00000000,	0x3}, /* AP3 IO - 116GB */
};
struct addr_map_win gwin_memory_map4_ap3[] = {
	{0x0000000000,	0x2000000000,	0x0}, /* AP0 DRAM - 128GB */
	{0x2000000000,	0x2000000000,	0x1}, /* AP1 DRAM - 128GB */
	{0x4000000000,	0x2000000000,	0x2}, /* AP2 DRAM - 128GB */
	{0x8100000000,	0x1d00000000,	0x0}, /* AP0 IO - 116GB */
	{0x9e00000000,	0x1d00000000,	0x1}, /* AP1 IO - 116GB */
	{0xbb00000000,	0x1d00000000,	0x2}, /* AP2 IO - 116GB */
};

int marvell_get_gwin_memory_map(int ap, struct addr_map_win **win, uint32_t *size)
{
	int ap_count = ap_get_count();

	if (ap_count < 3) {
		switch (ap) {
		case 0:
			*size = ARRAY_SIZE(gwin_memory_map2_ap0);
			*win = gwin_memory_map2_ap0;
			return 0;
		case 1:
			*size = ARRAY_SIZE(gwin_memory_map2_ap1);
			*win = gwin_memory_map2_ap1;
			return 0;
		default:
			break;
		}
	} else {
		switch (ap) {
		case 0:
			*size = ARRAY_SIZE(gwin_memory_map4_ap0);
			*win = gwin_memory_map4_ap0;
			return 0;
		case 1:
			*size = ARRAY_SIZE(gwin_memory_map4_ap1);
			*win = gwin_memory_map4_ap1;
			return 0;
		case 2:
			*size = ARRAY_SIZE(gwin_memory_map4_ap2);
			*win = gwin_memory_map4_ap2;
			return 0;
		case 3:
			*size = ARRAY_SIZE(gwin_memory_map4_ap3);
			*win = gwin_memory_map4_ap3;
			return 0;
		default:
			break;
		}
	}

	*size = 0;
	*win = NULL;
	return -1;
}

/*****************************************************************************
 * CCU Configuration
 *****************************************************************************
 */
/*
 * 0x0000_0000_0000 - 0x0080_ffff_ffff - DRAM:
 *					 partitioned differently for 2 or 4 APs
 * 0x0081_0000_0000 - 0x009d_ffff_ffff - AP0 IO
 * 0x009e_0000_0000 - 0x00ba_ffff_ffff - AP1 IO
 * 0x00bb_0000_0000 - 0x00d7_ffff_ffff - AP2 IO
 * 0x00d8_0000_0000 - 0x00f4_ffff_ffff - AP3 IO
 * 0x00f5_0000_0000 - 0x00ff_ffff_ffff - Unalocated
 * There are 5 windows total in CCU address decoding unit.
 * - Window 0 is occupied by the internal configuration space entry
 */
/* 1 to 2 APs map */
struct addr_map_win ccu_memory_map2_ap0[] = {
	{0x0000000000,	0x4000000000,	RAR_TID}, /* 256GB DRAM + 1GB for remapping */
	{0x8100000000,	0x1d00000000,	IO_0_TID}, /* 116GB local IO */
};
struct addr_map_win ccu_memory_map2_ap1[] = {
	{0x4000000000,	0x4000000000,	RAR_TID}, /* 256GB DRAM + 1GB for remapping */
	{0x9e00000000,	0x1d00000000,	IO_0_TID}, /* 116GB local IO */
};
/* 3 to 4 APs map */
struct addr_map_win ccu_memory_map_ap0[] = {
	{0x0000000000,	0x2000000000,	RAR_TID}, /* 128GB DRAM + 1GB for remapping */
	{0x8100000000,	0x1d00000000,	IO_0_TID}, /* 116GB local IO */
};
struct addr_map_win ccu_memory_map_ap1[] = {
	{0x2000000000,	0x2000000000,	RAR_TID}, /* 128GB DRAM + 1GB for remapping */
	{0x9e00000000,	0x1d00000000,	IO_0_TID}, /* 116GB local IO */
};
struct addr_map_win ccu_memory_map_ap2[] = {
	{0x4000000000,	0x2000000000,	RAR_TID}, /* 128GB DRAM + 1GB for remapping */
	{0xbb00000000,	0x1d00000000,	IO_0_TID}, /* 116GB local IO */
};
struct addr_map_win ccu_memory_map_ap3[] = {
	{0x6000000000,	0x2000000000,	RAR_TID}, /* 128GB DRAM + 1GB for remapping */
	{0xd800000000,	0x1d00000000,	IO_0_TID}, /* 116GB local IO */
};

uint32_t marvell_get_ccu_gcr_target(int ap)
{
	return GLOBAL_TID;
}

int marvell_get_ccu_memory_map(int ap, struct addr_map_win **win, uint32_t *size)
{
	int ap_count = ap_get_count();

	if (ap_count < 3) {
		switch (ap) {
		case 0:
			*size = ARRAY_SIZE(ccu_memory_map2_ap0);
			*win = ccu_memory_map2_ap0;
			return 0;
		case 1:
			*size = ARRAY_SIZE(ccu_memory_map2_ap1);
			*win = ccu_memory_map2_ap1;
			return 0;
		default:
			break;
		}
	} else {
		switch (ap) {
		case 0:
			*size = ARRAY_SIZE(ccu_memory_map_ap0);
			*win = ccu_memory_map_ap0;
			return 0;
		case 1:
			*size = ARRAY_SIZE(ccu_memory_map_ap1);
			*win = ccu_memory_map_ap1;
			return 0;
		case 2:
			*size = ARRAY_SIZE(ccu_memory_map_ap2);
			*win = ccu_memory_map_ap2;
			return 0;
		case 3:
			*size = ARRAY_SIZE(ccu_memory_map_ap3);
			*win = ccu_memory_map_ap3;
			return 0;
		default:
			break;
		}
	}

	*size = 0;
	*win = NULL;
	return -1;
}

/*****************************************************************************
 * IO WIN Configuration
 *****************************************************************************
 */
/* Up to 2 interconnected APs maps */
struct addr_map_win io_win_memory_map2_ap0[] = {
	{0x8100000000,	0x700000000,	MCI_3_TID}, /* AP0-CP0 */
	{0x8800000000,	0x700000000,	MCI_2_TID}, /* AP0-CP1 */
	{0x8f00000000,	0x700000000,	MCI_1_TID}, /* AP0-CP2 */
	{0x9600000000,	0x700000000,	MCI_0_TID}, /* AP0-CP3 */
	{0x9d00000000,	0x010000000,	SPI_TID},   /* AP0-SPI direct */
};
struct addr_map_win io_win_memory_map2_ap1[] = {
	{0x9e00000000,	0x700000000,	MCI_7_TID}, /* AP1-CP0 */
	{0xa500000000,	0x700000000,	MCI_6_TID}, /* AP1-CP1 */
	{0xac00000000,	0x700000000,	MCI_5_TID}, /* AP1-CP2 */
	{0xb300000000,	0x700000000,	MCI_4_TID}, /* AP1-CP3 */
	{0xba00000000,	0x010000000,	SPI_TID},   /* AP1-SPI direct */
};
/* TODO: 3 to 4 interconnected APs maps */

uint32_t marvell_get_io_win_gcr_target(int ap_index)
{
	return STM_TID;
}

int marvell_get_io_win_memory_map(int ap, struct addr_map_win **win,
				  uint32_t *size)
{
	int ap_count = ap_get_count();

	if (ap_count < 3) {
		switch (ap) {
		case 0:
			*size = ARRAY_SIZE(io_win_memory_map2_ap0);
			*win = io_win_memory_map2_ap0;
			return 0;
		case 1:
			*size = ARRAY_SIZE(io_win_memory_map2_ap1);
			*win = io_win_memory_map2_ap1;
			return 0;
		default:
			break;
		}
	}

	/* TODO: add support for 4 interconnected APs */

	*size = 0;
	*win = NULL;
	return -1;
}

/*****************************************************************************
 * IOB Configuration
 *****************************************************************************
 */
/* Internal registers access is already defined by the IOB Window 0
 * The below arrays describes the rest of IOB peripherals mapping
 */
/*=============== AP0 *===============*/
struct addr_map_win iob_memory_map_ap0_cp0[] = {
	/* PEX0_X4 window */
	/* AP0 CP0 IO
	 * 0x81_0000_0000 - 0x81_ffff_ffff - internal CFG (4GB)
	 * 0x82_0000_0000 - 0x83_ffff_ffff - PEX0 (8GB)
	 * 0x84_0000_0000 - 0x85_ffff_ffff - PEX1 (8GB)
	 * 0x86_0000_0000 - 0x87_ffff_ffff - PEX2 (8GB)
	 */
	{0x8200000000,	0x200000000,	PEX0_TID},
	{0x8400000000,	0x200000000,	PEX1_TID},
	{0x8600000000,	0x200000000,	PEX2_TID},
};
struct addr_map_win iob_memory_map_ap0_cp1[] = {
	/* PEX0_X4 window */
	/* AP0 CP1 IO
	 * 0x88_0000_0000 - 0x88_ffff_ffff - internal CFG (4GB)
	 * 0x89_0000_0000 - 0x8a_ffff_ffff - PEX0 (8GB)
	 * 0x8b_0000_0000 - 0x8c_ffff_ffff - PEX1 (8GB)
	 * 0x8d_0000_0000 - 0x8e_ffff_ffff - PEX2 (8GB)
	 */
	{0x8900000000,	0x200000000,	PEX0_TID},
	{0x8b00000000,	0x200000000,	PEX1_TID},
	{0x8d00000000,	0x200000000,	PEX2_TID},
};
struct addr_map_win iob_memory_map_ap0_cp2[] = {
	/* PEX0_X4 window */
	/* AP0 CP2 IO
	 * 0x8f_0000_0000 - 0x8f_ffff_ffff - internal CFG (4GB)
	 * 0x90_0000_0000 - 0x91_ffff_ffff - PEX0 (8GB)
	 * 0x92_0000_0000 - 0x93_ffff_ffff - PEX1 (8GB)
	 * 0x94_0000_0000 - 0x95_ffff_ffff - PEX2 (8GB)
	 */
	{0x9000000000,	0x200000000,	PEX0_TID},
	{0x9200000000,	0x200000000,	PEX1_TID},
	{0x9400000000,	0x200000000,	PEX2_TID},
};
struct addr_map_win iob_memory_map_ap0_cp3[] = {
	/* PEX0_X4 window */
	/* AP0 CP3 IO
	 * 0x96_0000_0000 - 0x96_ffff_ffff - internal CFG (4GB)
	 * 0x97_0000_0000 - 0x98_ffff_ffff - PEX0 (8GB)
	 * 0x99_0000_0000 - 0x9a_ffff_ffff - PEX1 (8GB)
	 * 0x9b_0000_0000 - 0x9c_ffff_ffff - PEX2 (8GB)
	 */
	{0x9700000000,	0x200000000,	PEX0_TID},
	{0x9900000000,	0x200000000,	PEX1_TID},
	{0x9b00000000,	0x200000000,	PEX2_TID},
};
/*=============== AP1 *===============*/
struct addr_map_win iob_memory_map_ap1_cp0[] = {
	/* PEX0_X4 window */
	/* AP1 CP0 IO
	 * 0x9e_0000_0000 - 0x9e_ffff_ffff - internal CFG (4GB)
	 * 0x9f_0000_0000 - 0xa0_ffff_ffff - PEX0 (8GB)
	 * 0xa1_0000_0000 - 0xa2_ffff_ffff - PEX1 (8GB)
	 * 0xa3_0000_0000 - 0xa4_ffff_ffff - PEX2 (8GB)
	 */
	{0x9f00000000,	0x200000000,	PEX0_TID},
	{0xa100000000,	0x200000000,	PEX1_TID},
	{0xa300000000,	0x200000000,	PEX2_TID},
};
struct addr_map_win iob_memory_map_ap1_cp1[] = {
	/* PEX0_X4 window */
	/* AP1 CP1 IO
	 * 0xa5_0000_0000 - 0xa5_ffff_ffff - internal CFG (4GB)
	 * 0xa6_0000_0000 - 0xa7_ffff_ffff - PEX0 (8GB)
	 * 0xa8_0000_0000 - 0xa9_ffff_ffff - PEX1 (8GB)
	 * 0xaa_0000_0000 - 0xab_ffff_ffff - PEX2 (8GB)
	 */
	{0xa600000000,	0x200000000,	PEX0_TID},
	{0xa800000000,	0x200000000,	PEX1_TID},
	{0xaa00000000,	0x200000000,	PEX2_TID},
};
struct addr_map_win iob_memory_map_ap1_cp2[] = {
	/* PEX0_X4 window */
	/* AP1 CP2 IO
	 * 0xac_0000_0000 - 0xac_ffff_ffff - internal CFG (4GB)
	 * 0xad_0000_0000 - 0xae_ffff_ffff - PEX0 (8GB)
	 * 0xaf_0000_0000 - 0xb0_ffff_ffff - PEX1 (8GB)
	 * 0xb1_0000_0000 - 0xb2_ffff_ffff - PEX2 (8GB)
	 */
	{0xad00000000,	0x200000000,	PEX0_TID},
	{0xaf00000000,	0x200000000,	PEX1_TID},
	{0xb100000000,	0x200000000,	PEX2_TID},
};
struct addr_map_win iob_memory_map_ap1_cp3[] = {
	/* PEX0_X4 window */
	/* AP1 CP3 IO
	 * 0xb3_0000_0000 - 0xb3_ffff_ffff - internal CFG (4GB)
	 * 0xb4_0000_0000 - 0xb5_ffff_ffff - PEX0 (8GB)
	 * 0xb6_0000_0000 - 0xb7_ffff_ffff - PEX1 (8GB)
	 * 0xb8_0000_0000 - 0xb9_ffff_ffff - PEX2 (8GB)
	 */
	{0xb400000000,	0x200000000,	PEX0_TID},
	{0xb600000000,	0x200000000,	PEX1_TID},
	{0xb800000000,	0x200000000,	PEX2_TID},
};
/*=============== AP2 *===============*/
struct addr_map_win iob_memory_map_ap2_cp0[] = {
	/* PEX0_X4 window */
	/* AP2 CP0 IO
	 * 0xbb_0000_0000 - 0xbb_ffff_ffff - internal CFG (4GB)
	 * 0xbc_0000_0000 - 0xbd_ffff_ffff - PEX0 (8GB)
	 * 0xbe_0000_0000 - 0xbf_ffff_ffff - PEX1 (8GB)
	 * 0xc0_0000_0000 - 0xc1_ffff_ffff - PEX2 (8GB)
	 */
	{0xbc00000000,	0x200000000,	PEX0_TID},
	{0xbe00000000,	0x200000000,	PEX1_TID},
	{0xc000000000,	0x200000000,	PEX2_TID},
};
struct addr_map_win iob_memory_map_ap2_cp1[] = {
	/* PEX0_X4 window */
	/* AP2 CP1 IO
	 * 0xc2_0000_0000 - 0xc2_ffff_ffff - internal CFG (4GB)
	 * 0xc3_0000_0000 - 0xc4_ffff_ffff - PEX0 (8GB)
	 * 0xc5_0000_0000 - 0xc6_ffff_ffff - PEX1 (8GB)
	 * 0xc7_0000_0000 - 0xc8_ffff_ffff - PEX2 (8GB)
	 */
	{0xc300000000,	0x200000000,	PEX0_TID},
	{0xc500000000,	0x200000000,	PEX1_TID},
	{0xc700000000,	0x200000000,	PEX2_TID},
};
struct addr_map_win iob_memory_map_ap2_cp2[] = {
	/* PEX0_X4 window */
	/* AP2 CP2 IO
	 * 0xc9_0000_0000 - 0xc9_ffff_ffff - internal CFG (4GB)
	 * 0xca_0000_0000 - 0xcb_ffff_ffff - PEX0 (8GB)
	 * 0xcc_0000_0000 - 0xcd_ffff_ffff - PEX1 (8GB)
	 * 0xce_0000_0000 - 0xcf_ffff_ffff - PEX2 (8GB)
	 */
	{0xca00000000,	0x200000000,	PEX0_TID},
	{0xcc00000000,	0x200000000,	PEX1_TID},
	{0xce00000000,	0x200000000,	PEX2_TID},
};
struct addr_map_win iob_memory_map_ap2_cp3[] = {
	/* PEX0_X4 window */
	/* AP2 CP3 IO
	 * 0xd0_0000_0000 - 0xd0_ffff_ffff - internal CFG (4GB)
	 * 0xd1_0000_0000 - 0xd2_ffff_ffff - PEX0 (8GB)
	 * 0xd3_0000_0000 - 0xd4_ffff_ffff - PEX1 (8GB)
	 * 0xd5_0000_0000 - 0xd6_ffff_ffff - PEX2 (8GB)
	 */
	{0xd100000000,	0x200000000,	PEX0_TID},
	{0xd200000000,	0x200000000,	PEX1_TID},
	{0xd500000000,	0x200000000,	PEX2_TID},
};
/*=============== AP3 *===============*/
struct addr_map_win iob_memory_map_ap3_cp0[] = {
	/* PEX0_X4 window */
	/* AP3 CP0 IO
	 * 0xd8_0000_0000 - 0xd8_ffff_ffff - internal CFG (4GB)
	 * 0xd9_0000_0000 - 0xda_ffff_ffff - PEX0 (8GB)
	 * 0xdb_0000_0000 - 0xdc_ffff_ffff - PEX1 (8GB)
	 * 0xdd_0000_0000 - 0xde_ffff_ffff - PEX2 (8GB)
	 */
	{0xd900000000,	0x200000000,	PEX0_TID},
	{0xdb00000000,	0x200000000,	PEX1_TID},
	{0xdd00000000,	0x200000000,	PEX2_TID},
};
struct addr_map_win iob_memory_map_ap3_cp1[] = {
	/* PEX0_X4 window */
	/* AP3 CP1 IO
	 * 0xdf_0000_0000 - 0xdf_ffff_ffff - internal CFG (4GB)
	 * 0xe0_0000_0000 - 0xe1_ffff_ffff - PEX0 (8GB)
	 * 0xe2_0000_0000 - 0xe3_ffff_ffff - PEX1 (8GB)
	 * 0xe4_0000_0000 - 0xe5_ffff_ffff - PEX2 (8GB)
	 */
	{0xe000000000,	0x200000000,	PEX0_TID},
	{0xe200000000,	0x200000000,	PEX1_TID},
	{0xe400000000,	0x200000000,	PEX2_TID},
};
struct addr_map_win iob_memory_map_ap3_cp2[] = {
	/* PEX0_X4 window */
	/* AP3 CP2 IO
	 * 0xe6_0000_0000 - 0xe6_ffff_ffff - internal CFG (4GB)
	 * 0xe7_0000_0000 - 0xe8_ffff_ffff - PEX0 (8GB)
	 * 0xe9_0000_0000 - 0xea_ffff_ffff - PEX1 (8GB)
	 * 0xeb_0000_0000 - 0xec_ffff_ffff - PEX2 (8GB)
	 */
	{0xe700000000,	0x200000000,	PEX0_TID},
	{0xe900000000,	0x200000000,	PEX1_TID},
	{0xeb00000000,	0x200000000,	PEX2_TID},
};
struct addr_map_win iob_memory_map_ap3_cp3[] = {
	/* PEX0_X4 window */
	/* AP3 CP3 IO
	 * 0xed_0000_0000 - 0xed_ffff_ffff - internal CFG (4GB)
	 * 0xee_0000_0000 - 0xef_ffff_ffff - PEX0 (8GB)
	 * 0xf0_0000_0000 - 0xf1_ffff_ffff - PEX1 (8GB)
	 * 0xf2_0000_0000 - 0xf3_ffff_ffff - PEX2 (8GB)
	 */
	{0xee00000000,	0x200000000,	PEX0_TID},
	{0xf000000000,	0x200000000,	PEX1_TID},
	{0xf200000000,	0x200000000,	PEX2_TID},
};

struct addr_map_win *iob_map[PLAT_MARVELL_NORTHB_COUNT][PLAT_MARVELL_SOUTHB_COUNT] = {
	/* AP0 */
	{ iob_memory_map_ap0_cp0, iob_memory_map_ap0_cp1,
	  iob_memory_map_ap0_cp2, iob_memory_map_ap0_cp3 },
	/* AP1 */
	{ iob_memory_map_ap1_cp0, iob_memory_map_ap1_cp1,
	  iob_memory_map_ap1_cp2, iob_memory_map_ap1_cp3 },
	/* AP2 */
	{ iob_memory_map_ap2_cp0, iob_memory_map_ap2_cp1,
	  iob_memory_map_ap2_cp2, iob_memory_map_ap2_cp3 },
	/* AP3 */
	{ iob_memory_map_ap3_cp0, iob_memory_map_ap3_cp1,
	  iob_memory_map_ap3_cp2, iob_memory_map_ap3_cp3 }
};

uint32_t iob_map_size[PLAT_MARVELL_NORTHB_COUNT][PLAT_MARVELL_SOUTHB_COUNT] = {
	/* AP0 */
	{ ARRAY_SIZE(iob_memory_map_ap0_cp0),
	  ARRAY_SIZE(iob_memory_map_ap0_cp1),
	  ARRAY_SIZE(iob_memory_map_ap0_cp2),
	  ARRAY_SIZE(iob_memory_map_ap0_cp3) },
	/* AP1 */
	{ ARRAY_SIZE(iob_memory_map_ap1_cp0),
	  ARRAY_SIZE(iob_memory_map_ap1_cp1),
	  ARRAY_SIZE(iob_memory_map_ap1_cp2),
	  ARRAY_SIZE(iob_memory_map_ap1_cp3) },
	/* AP2 */
	{ ARRAY_SIZE(iob_memory_map_ap2_cp0),
	  ARRAY_SIZE(iob_memory_map_ap2_cp1),
	  ARRAY_SIZE(iob_memory_map_ap2_cp2),
	  ARRAY_SIZE(iob_memory_map_ap2_cp3) },
	/* AP3 */
	{ ARRAY_SIZE(iob_memory_map_ap3_cp0),
	  ARRAY_SIZE(iob_memory_map_ap3_cp1),
	  ARRAY_SIZE(iob_memory_map_ap3_cp2),
	  ARRAY_SIZE(iob_memory_map_ap3_cp3) }
};

int marvell_get_iob_memory_map(struct addr_map_win **win, uint32_t *size,
			       uintptr_t base)
{
	int  ap, cp;

	for (ap = 0; ap < ap_get_count(); ap++) {
		for (cp = 0; cp < ap810_get_cp_per_ap_cnt(ap); cp++) {
			if (MVEBU_CP_REGS_BASE(ap, cp) == base) {
				*win = iob_map[ap][cp];
				*size = iob_map_size[ap][cp];
				return 0;
			}
		}
	}

	*win = NULL;
	*size = 0;
	return -1;
}

/*****************************************************************************
 * AMB Configuration
 *****************************************************************************
 */
struct addr_map_win *amb_map[PLAT_MARVELL_NORTHB_COUNT]
			    [PLAT_MARVELL_SOUTHB_COUNT] = {
	/* AP0 */
	{ NULL, NULL, NULL, NULL },
	/* AP1 */
	{ NULL, NULL, NULL, NULL },
	/* AP2 */
	{ NULL, NULL, NULL, NULL },
	/* AP3 */
	{ NULL, NULL, NULL, NULL },
};

uint32_t amb_map_size[PLAT_MARVELL_NORTHB_COUNT][PLAT_MARVELL_SOUTHB_COUNT] = {
	/* AP0 */
	{ 0, 0, 0, 0 },
	/* AP1 */
	{ 0, 0, 0, 0 },
	/* AP2 */
	{ 0, 0, 0, 0 },
	/* AP3 */
	{ 0, 0, 0, 0 },
};

int marvell_get_amb_memory_map(struct addr_map_win **win, uint32_t *size,
			       uintptr_t base)
{
	int  ap, cp;

	for (ap = 0; ap < ap_get_count(); ap++) {
		for (cp = 0; cp < ap810_get_cp_per_ap_cnt(ap); cp++) {
			if (MVEBU_CP_REGS_BASE(ap, cp) == base) {
				*win = amb_map[ap][cp];
				*size = amb_map_size[ap][cp];
				return 0;
			}
		}
	}

	*win = NULL;
	*size = 0;
	return -1;
}
/*****************************************************************************
 * SoC PM configuration
 *****************************************************************************
 */
/* CP GPIO should be used and the GPIOs should be within same GPIO register */
struct power_off_method *pm_cfg = NULL;

void *plat_get_pm_cfg(void)
{
	/* Return the PM configurations */
	return &pm_cfg;
}
