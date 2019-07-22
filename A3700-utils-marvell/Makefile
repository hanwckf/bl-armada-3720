ROOT_PATH		:= $(shell pwd)
LOCAL_VERSION_FILE	:= $(ROOT_PATH)/localversion
LOCAL_VERSION_STRING	:= $(shell cat $(LOCAL_VERSION_FILE))
TIM_DDR_PATH 		:= $(ROOT_PATH)/tim/ddr
SCRIPT_PATH		:= $(ROOT_PATH)/script

DDR_TOPOLOGY		?= 0
DDR_TOPOLOGY_FILE 	:= $(TIM_DDR_PATH)/DDR_TOPOLOGY_$(DDR_TOPOLOGY).txt
GET_DDR_PARAMS=$(SCRIPT_PATH)/getddrparams.sh
CONFIG_DDR_TYPE=$(shell ($(GET_DDR_PARAMS) $(DDR_TOPOLOGY_FILE) ddr_type))

ifeq ($(CONFIG_DDR_TYPE), 1)
	DDR_TYPE = DDR4
else ifeq ($(CONFIG_DDR_TYPE), 0)
	DDR_TYPE = DDR3
endif

all: mv_ddr WTMI

mv_ddr:
	${Q}${MAKE} PLATFORM=a3700 --no-print-directory -C ${MV_DDR_PATH} DDR_TYPE=$(DDR_TYPE) clean
	make -C ${MV_DDR_PATH} PLATFORM=a3700 DDR_TYPE=$(DDR_TYPE)
	@cp -f ${MV_DDR_PATH}/a3700_tool $(TIM_DDR_PATH)/ddr_tool

WTMI:
	${MAKE} -C wtmi LOCAL_VERSION_STRING=$(LOCAL_VERSION_STRING)

clean:
	${Q}${MAKE} -C wtmi clean
	@rm -f tim/ddr/ddr_static.txt tim/ddr/clocks_ddr.txt
	${Q}${MAKE} PLATFORM=a3700 --no-print-directory -C ${MV_DDR_PATH} DDR_TYPE=$(DDR_TYPE) clean
