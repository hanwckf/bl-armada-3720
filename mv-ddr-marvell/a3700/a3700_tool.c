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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "ddr3_init.h"
#include "mv_ddr_tim.h"
#include "mv_ddr_topology.h"

#define DEBUG(...)
#define INFO printf
#define ERROR printf

#define MAX_CFG_NAME_LEN	26
#define MAX_CFG_VALUE_LEN	51
#define MAX_CFG_LINE_LEN	(MAX_CFG_NAME_LEN + MAX_CFG_VALUE_LEN - 1)

enum ddr_type {
	DDR3,
	DDR4,
	DDR_TYPE_LAST
};

struct ddr_porting_cfg {
	struct mv_ddr_topology_map map;
	enum ddr_type type;
};

static struct ddr_porting_cfg cfg;

enum ddr_config_index {
	DDR_TYPE,
	SYS_DBG_LEVEL,
	DDR_ACTIVE_INTERFACE,
	DDR_CS_MASK,
	DDR_SPEEDBIN_INDEX,
	DDR_BUS_WIDTH_INDEX,
	DDR_MEM_SIZE_INDEX,
	DDR_MEM_FREQ_INDEX,
	DDR_BUS_ACT_MASK,
	DDR_CL,
	DDR_CWL
};

struct config_item {
	enum ddr_config_index index;
	char key[MAX_CFG_NAME_LEN];
	char value[MAX_CFG_VALUE_LEN];
};

enum output_operation_index {
	DDR_OP_WRITE = 1
};

struct output_item {
	u32 op;
	u32 para0;
	u32 para1;
	u32 para2;
};

#define MAX_TIM_ITEMS 300
#define MAX_TIM_ITEM_LEN 100
#define MAX_TIM_REG_LEN 11
#define MAX_TIM_VALUE_LEN 11

#ifdef CONFIG_DDR4
#define DDR_DFT_CL	12 /* DDR4-2400R with 800Mhz clk */
#define DDR_DFT_CWL	11
#else /* CONFIG_DDR3 */
#define DDR_DFT_CL	11 /* DDR3-1600K on 800Mhz clk */
#define DDR_DFT_CWL	8
#endif

static struct output_item outputlist[MAX_TIM_ITEMS];
static struct output_item replacelist[MAX_TIM_ITEMS];

static struct config_item cfg_list[] = {
	{DDR_TYPE,		"ddr_type",		{} },
	{SYS_DBG_LEVEL,		"sys_dbg_level",	{} },
	{DDR_ACTIVE_INTERFACE,	"ddr_active_interface",	{} },
	{DDR_CS_MASK,		"ddr_cs_mask",		{} },
	{DDR_SPEEDBIN_INDEX,	"ddr_speedbin_index",	{} },
	{DDR_BUS_WIDTH_INDEX,	"ddr_bus_width_index",	{} },
	{DDR_MEM_SIZE_INDEX,	"ddr_mem_size_index",	{} },
	{DDR_MEM_FREQ_INDEX,	"ddr_mem_freq_index",	{} },
	{DDR_BUS_ACT_MASK,	"ddr_bus_act_mask",	{} },
	{DDR_CL,		"ddr_cas_latency",	{} },
	{DDR_CWL,		"ddr_cas_write_latency",	{} }
};

static int strkv(char *src, char *key, char *value)
{
	char *p, *q;

	if (*src == '#' || strlen(src) == 0)
		return -2; /* use # for comments */

	p = strchr(src, '=');
	q = strchr(src, '\n') ? strchr(src, '\n') : strchr(src, '\0');
	if (p != NULL && q != NULL) {
		*q = '\0';
		strncpy(key, src, p - src);
		strcpy(value, p + 1);
		DEBUG("strkv item key=%s value=%s\n", key, value);
		return 0;
	}

	DEBUG("strkv item not found\n");

	return -1;
}

static int ddr_add_replace_item(u32 op, u32 para0, u32 para1, u32 para2)
{
	static u32 index;
	u32 i;

	/* check for same reg settings in replace list */
	for (i = 0; i < index; i++) {
		if (replacelist[i].op == op && replacelist[i].para0 == para0) {
			DEBUG("ddr_add_replace_item found index=%d para0=%x para1=%x\n",
			      i, replacelist[i].para0, para1);
			return 0;
		}
	}

	if (index < MAX_TIM_ITEMS) {
		replacelist[index].op = op;
		replacelist[index].para0 = para0;
		replacelist[index].para1 = para1;
		replacelist[index].para2 = para2;
		DEBUG("ddr_add_replace_item index=%d para0=%x para1=%x\n", index, replacelist[index].para0, para1);
		index++;
	}

	return 0;
}

static int ddr_add_replace_write(char *item)
{
	char reg[MAX_TIM_REG_LEN] = "";
	char value[MAX_TIM_VALUE_LEN] = "";

	if (strkv(item, reg, value) == 0)
		ddr_add_replace_item(DDR_OP_WRITE, strtol(reg, NULL, 16), strtol(value, NULL, 16), 0);
	else
		return -1;

	return 0;
}

static int ddr_add_output_item(u32 op, u32 para0, u32 para1, u32 para2)
{
	static u32 index;

	if (index < MAX_TIM_ITEMS) {
		outputlist[index].op = op;
		outputlist[index].para0 = para0;
		outputlist[index].para1 = para1;
		outputlist[index].para2 = para2;
		DEBUG("index=%d para0=%x para1=%x\n", index, outputlist[index].para0, para1);
		index++;
	}

	return 0;
}

int ddr_reg_write(u32 reg, u32 value)
{
	return ddr_add_output_item(DDR_OP_WRITE, reg, value, 0);
}

static int ddr_gen_output_item(char *buf, struct output_item *item)
{
	int i;

	if (item == NULL)
		return -1;

	switch (item->op) {
	case DDR_OP_WRITE:
		for (i = 0; replacelist[i].op && i < MAX_TIM_ITEMS; i++) {
			if (replacelist[i].para0 == item->para0) {
				sprintf(buf, "WRITE: 0x%08X 0x%08X\n", replacelist[i].para0, replacelist[i].para1);
				return 0;
			}
		}
		sprintf(buf, "WRITE: 0x%08X 0x%08X\n", item->para0, item->para1);
		break;
	default:
		ERROR("unsupported operation requested\n");
		return -1;
	}

	return 0;
}

static int ddr_gen_pre_init(FILE *fp)
{
	struct if_params *iface_params = &(cfg.map.interface_params[0]);

	if (cfg.type == DDR3) {
		if (iface_params->as_bus_params[0].cs_bitmask == 1)
			fputs(ddr3_1cs_tim_pre, fp);
		else
			fputs(ddr3_2cs_tim_pre, fp);
	} else if (cfg.type == DDR4) {
		if (iface_params->as_bus_params[0].cs_bitmask == 1)
			fputs(ddr4_1cs_tim_pre, fp);
		else
			fputs(ddr4_2cs_tim_pre, fp);
	} else {
		ERROR("incorrect ddr type found\n");
		return -1;
	}

	return 0;
}

static int ddr_gen_post_init(FILE *fp)
{
	struct if_params *iface_params = &(cfg.map.interface_params[0]);

	if (cfg.type == DDR3) {
		if (iface_params->as_bus_params[0].cs_bitmask == 1)
			fputs(ddr3_1cs_tim_post, fp);
		else
			fputs(ddr3_2cs_tim_post, fp);
	} else if (cfg.type == DDR4) {
		if (iface_params->as_bus_params[0].cs_bitmask == 1)
			fputs(ddr4_1cs_tim_post, fp);
		else
			fputs(ddr4_2cs_tim_post, fp);
	} else {
		ERROR("incorrect ddr type found\n");
		return -1;
	}

	return 0;
}

static int ddr_gen_output(FILE *fp)
{
	char line[MAX_TIM_ITEM_LEN];
	u32 i;

	if (ddr_gen_pre_init(fp))
		return -1;

	for (i = 0; i < MAX_TIM_ITEMS && outputlist[i].op > 0; i++) {
		if (!ddr_gen_output_item(line, &outputlist[i]))
			fputs(line, fp);
		else {
			ERROR("unsupported output item found\n");
			return -1;
		}
	}

	if (ddr_gen_post_init(fp))
		return -1;

	return 0;
}

static void usage(void)
{
	static const char *ddr_tool_usage =
	"Armada3700 DDR registers list generation tool v0.1\n"
	"Usage: a3700_tool -i CFG_FILE [-o OUTPUT_FILE] [OPTION]\n"
	"Read DDR configuration from CFG_FILE and print out DDR registers list or write to OUTPUT_FILE\n"
	"\n"
	"\t-r\treplace the reg value in list\n"
	"\t-h\tdisplay this help\n"
	"\n"
	"Example:\n"
	"\t./a3700_tool -i cfg_ddr4_1cs.txt -o tim_ddr4_1cs.txt -r 0xC0000380=0x0007A120\n";
	fprintf(stderr, "%s\n", ddr_tool_usage);
}

static int ddr_cfg_read(FILE *fp, struct config_item *cfg_list, int num)
{
	char line[MAX_CFG_LINE_LEN] = "";
	char key[MAX_CFG_NAME_LEN] = "";
	char value[MAX_CFG_VALUE_LEN] = "";
	int i;

	while (fgets(line, MAX_CFG_LINE_LEN, fp)) {
		DEBUG("line %s\n", line);
		if (strkv(line, key, value) == 0) {
			for (i = 0; i < num; i++) {
				if (strcmp(key, cfg_list[i].key) == 0)
					strcpy(cfg_list[i].value, value);
			}
			memset(key, 0, MAX_CFG_LINE_LEN);
		}
	}

	return 0;
}

static int ddr_cfg_parse(struct config_item *clist, struct ddr_porting_cfg *cfg)
{
	struct if_params *iface_params = &(cfg->map.interface_params[0]);
	u8 cs_bitmask;
	u32 i;

	cfg->type = strtol(clist[DDR_TYPE].value, (char **)NULL, 0);
	cfg->map.debug_level = strtol(clist[SYS_DBG_LEVEL].value, (char **)NULL, 0);
	cfg->map.if_act_mask = 1;

	cs_bitmask = strtol(clist[DDR_CS_MASK].value, (char **)NULL, 0);
	for (i = 0; i < MAX_BUS_NUM; i++)
		iface_params->as_bus_params[i].cs_bitmask = cs_bitmask;

	iface_params->speed_bin_index = strtol(clist[DDR_SPEEDBIN_INDEX].value, (char **)NULL, 0);
	iface_params->bus_width = strtol(clist[DDR_BUS_WIDTH_INDEX].value, (char **)NULL, 0);
	iface_params->memory_size = strtol(clist[DDR_MEM_SIZE_INDEX].value, (char **)NULL, 0);
	iface_params->memory_freq = MV_DDR_FREQ_800;

	cfg->map.bus_act_mask = BUS_MASK_16BIT;

	/* Parse CL/CWL settings from cfg items */
	if (clist[DDR_CL].value[0] && clist[DDR_CWL].value[0]) {
		iface_params->cas_l = strtol(clist[DDR_CL].value, (char **)NULL, 0);
		iface_params->cas_wl = strtol(clist[DDR_CWL].value, (char **)NULL, 0);
	}
	/* Set to default value if items not found */
	if (!(iface_params->cas_l && iface_params->cas_wl)) {
		iface_params->cas_l = DDR_DFT_CL;
		iface_params->cas_wl = DDR_DFT_CWL;
	}

	if ((cfg->type != DDR3 && cfg->type != DDR4) ||
		(iface_params->as_bus_params[0].cs_bitmask != MV_DDR_CS_BITMASK_1CS
			&& iface_params->as_bus_params[0].cs_bitmask != MV_DDR_CS_BITMASK_2CS) ||
		(iface_params->bus_width != MV_DDR_DEV_WIDTH_8BIT
			&& iface_params->bus_width != MV_DDR_DEV_WIDTH_16BIT) ||
		iface_params->memory_size >= MV_DDR_DIE_CAP_LAST) {
		ERROR("incorrect input ddr configuration found\n");
		return -1;
	}

#ifdef CONFIG_DDR4
	if (cfg->type != DDR4) {
		ERROR("only DDR4 supported, check CFG_FILE\n");
		return -1;
	}
#else /* CONFIG_DDR3 */
	if (cfg->type != DDR3) {
		ERROR("only DDR3 supported, check CFG_FILE\n");
		return -1;
	}
#endif

	INFO("Board info:\n");
	INFO("\tBus width:\t16bit\n");
	INFO("\tCS num:\t\t%s\n", iface_params->as_bus_params[0].cs_bitmask == MV_DDR_CS_BITMASK_2CS ?
		"2CS" :	(iface_params->as_bus_params[0].cs_bitmask == MV_DDR_CS_BITMASK_1CS ?
			"1CS" : "unsupported cs number"));
	INFO("\tChip:\t\t%s ", cfg->type == DDR3 ? "DDR3" : "DDR4");
	INFO("%s ", iface_params->bus_width == MV_DDR_DEV_WIDTH_16BIT ? "16bit" :
		(iface_params->bus_width == MV_DDR_DEV_WIDTH_8BIT ? "8bit" : "unsupported bus width"));
	INFO("%dMBytes\n", 64<<(iface_params->memory_size));

	return 0;
}

int main(int argc, char **argv)
{
	int ret, i;
	char *infile = NULL;
	char *outfile = NULL;
	FILE *ifp, *ofp = stdout;

	/* check for minimum number of arguments */
	if (argc < 3) {
		ERROR("too few arguments found\n");
		usage();
		return 0;
	}

	/* parse arguments */
	while ((ret = getopt(argc, argv, "i:o:r:h")) != -1) {
		switch (ret) {
		case 'i':
			infile = optarg;
			break;
		case 'o':
			outfile = optarg;
			break;
		case 'r':
			DEBUG("replace=%s\n", optarg);
			if (ddr_add_replace_write(optarg)) {
				ERROR("replace item format error\n");
				usage();
				return -1;
			}
			break;
		case 'h':
		default:
			usage();
			return 0;
		}
	}

	/* open input file for reading */
	if (infile != NULL) {
		ifp = fopen(infile, "r");
		if (ifp == NULL) {
			ERROR("can't open input file for reading\n");
			return -1;
		}
	} else {
		usage();
		return 0;
	}

	/* open output file for writing if requested */
	if (outfile != NULL) {
		ofp = fopen(outfile, "w");
		if (ofp == NULL) {
			ERROR("can't open output file for writing\n");
			return -1;
		}
	}

	/* read configuration from input file to cfg_list */
	ddr_cfg_read(ifp, cfg_list, sizeof(cfg_list) / sizeof(struct config_item));

	for (i = 0; i < sizeof(cfg_list) / sizeof(struct config_item); i++)
		DEBUG("%s = %s\n", cfg_list[i].key, cfg_list[i].value);

	/* parse cfg_list into ddr struct */
	if (ddr_cfg_parse(cfg_list, &cfg)) {
		usage();
		return 0;
	}

	/* get reg settings */
	ddr_controller_init(&cfg.map);

	/* print out or write to file reg settings */
	if (ddr_gen_output(ofp)) {
		usage();
		return 0;
	}

	fclose(ifp);
	fclose(ofp);

	return 0;
}
