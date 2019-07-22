#!/bin/bash
# Copyright (C) 2018 Marvell International Ltd.
#
# SPDX-License-Identifier:           GPL-2.0
# https://spdx.org/licenses
###############################################################################
## This is the compile script for atf                                        ##
## This script is called by CI automated builds                              ##
## It may also be used interactively by users to compile the same way as CI  ##
###############################################################################
## WARNING: Do NOT MODIFY the CI wrapper code segments.                      ##
## You can only modify the config and compile commands                       ##
###############################################################################


## =v=v=v=v=v=v=v=v=v=v=v CI WRAPPER - Do not Modify! v=v=v=v=v=v=v=v=v=v=v= ##
set -euo pipefail
shopt -s extglob
##==================================== USAGE ================================##
function usage {
	echo """
Usage: compile [--no_configure] [--echo_only] BUILD_NAME
 or:   compile --list
 or:   compile --help

Compiles atf similar to the given CI build

 -N, --no_configure   Skip configuration steps (make distclean)
 -e, --echo_only      Print out the compilation sequence but do not execute it
 -l, --list           List all supported BUILD_NAME values and exit
 -h, --help           Display this help and exit

Prerequisites:
  CROSS_COMPILE       path to cross compiler
  BL33                path to u-boot or uefi binary
  MV_DDR_PATH         path to ddr source
  SCP_BL2             path to FreeRTOS binary (optional)
  WTMI_IMG            path to FreeRTOS binary for armada-3700 (optional)
  WTP                 path to a3700 utils source (optional)


"""
	exit 0
}
##============================ PARSE ARGUMENTS ==============================##
TEMP=`getopt -a -o Nelh --long no_configure,echo_only,list,help \
             -n 'compile' -- "$@"`

if [ $? != 0 ] ; then
	echo "Error: Failed parsing command options" >&2
	exit 1
fi
eval set -- "$TEMP"

no_configure=
echo_only=
list=

while true; do
	case "$1" in
		-N | --no_configure ) no_configure=true; shift ;;
		-e | --echo_only )    echo_only=true; shift ;;
		-l | --list ) 	      list=true; shift ;;
		-h | --help )         usage; ;;
		-- ) shift; break ;;
		* ) break ;;
	esac
done

if [[ $list ]] ; then
	DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
	echo "Supported build names:"
	grep -v '^#' "$DIR/supported_builds.txt"
	echo
	echo
	exit 0
fi

[[ $# -ne 1 ]] && usage
build_name=$1

grep ^$build_name$ ./scripts/ci/supported_builds.txt >&/dev/null ||
	( echo "Error: Unsupported build ${build_name}"; exit -1 )
echo "running compile.sh ${build_name}"
## =^=^=^=^=^=^=^=^=^=^=^=^  End of CI WRAPPER code -=^=^=^=^=^=^=^=^=^=^=^= ##


########################### U-BOOT CONFIGURATION ##############################
case $build_name in
	*_a70x0_pcap )   platform="a70x0_pcac"; ;;
	*_a70x0* )       platform="a70x0"; ;;
	*_a7020_amc )    platform="a70x0_amc"; ;;
	*_a80x0_mcbin* ) platform="a80x0_mcbin"; ;;
	*_a80x0* )       platform="a80x0"; ;;
	*_a3900_z1* )    platform="a3900_z1"; ;;
	*_a3900_z2* )    platform="a3900_z2"; ;;
	*_a3900* )       platform="a3900"; ;;
	*_a37xx* )       platform="a3700"; ;;
	* ) echo "Error: Could not configure platform."
		"Unsupported build ${build_name}"; exit -1; ;;
esac

case $build_name in
	uefi* )       build_flags="USE_COHERENT_MEM=0 LOG_LEVEL=20 WORKAROUND_CVE_2018_3639=0"; ;;
	*_a70x0* )    build_flags="DEBUG=0 USE_COHERENT_MEM=0 LOG_LEVEL=20 WORKAROUND_CVE_2018_3639=0"; ;;
	*_a7020_amc ) build_flags="DEBUG=0 USE_COHERENT_MEM=0 LOG_LEVEL=20 WORKAROUND_CVE_2018_3639=0"; ;;
	*_a80x0_mcbin_cve_2018_3639) build_flags="DEBUG=0 USE_COHERENT_MEM=0 LOG_LEVEL=20"; ;;
	*_a80x0* )    build_flags="DEBUG=0 USE_COHERENT_MEM=0 LOG_LEVEL=20 WORKAROUND_CVE_2018_3639=0"; ;;
	*_a3900* )    build_flags="DEBUG=0 USE_COHERENT_MEM=0 LOG_LEVEL=20 WORKAROUND_CVE_2018_3639=0 MV_DDR4_BUILD=y"; ;;
	*_a37xx_ddr3_* ) build_flags="DDR_TYPE=DDR3 USE_COHERENT_MEM=0 LOG_LEVEL=20 DDR_TOPOLOGY=0"; ;;
	*_a37xx_ddr4_v1_* ) build_flags="DDR_TYPE=DDR4 USE_COHERENT_MEM=0 LOG_LEVEL=20 DDR_TOPOLOGY=1"; ;;
	*_a37xx_ddr4_v3_* ) build_flags="DDR_TYPE=DDR4 USE_COHERENT_MEM=0 LOG_LEVEL=20 DDR_TOPOLOGY=3"; ;;
	*_a37xx_espressobin_ddr3_1cs_* ) build_flags="DDR_TYPE=DDR3 USE_COHERENT_MEM=0 LOG_LEVEL=20 DDR_TOPOLOGY=0"; ;;
	*_a37xx_espressobin_ddr3_2cs_* ) build_flags="DDR_TYPE=DDR3 USE_COHERENT_MEM=0 LOG_LEVEL=20 DDR_TOPOLOGY=2"; ;;
	*_a37xx_espressobin_ddr4_v7_1G_emmc_* ) build_flags="DDR_TYPE=DDR4 USE_COHERENT_MEM=0 BOOTDEV=EMMCNORM PARTNUM=1 LOG_LEVEL=20 DDR_TOPOLOGY=5"; ;;
	*_a37xx_espressobin_ddr4_v7_2G_emmc_* ) build_flags="DDR_TYPE=DDR4 USE_COHERENT_MEM=0 BOOTDEV=EMMCNORM PARTNUM=1 LOG_LEVEL=20 DDR_TOPOLOGY=6"; ;;
	*_a37xx_espressobin_ddr4_v7_1G_* ) build_flags="DDR_TYPE=DDR4 USE_COHERENT_MEM=0 LOG_LEVEL=20 DDR_TOPOLOGY=5"; ;;
	*_a37xx_espressobin_ddr4_v7_2G_* ) build_flags="DDR_TYPE=DDR4 USE_COHERENT_MEM=0 LOG_LEVEL=20 DDR_TOPOLOGY=6"; ;;
	* ) echo "Error: Could not configure build_flags. Unsupported build ${build_name}"; exit -1; ;;
esac

pm_fw_export=""
if [[ $build_name =~ _a37xx_ ]]; then
	case $build_name in
		*a37xx_*_1000_800 )  build_flags="${build_flags} CLOCKSPRESET=CPU_1000_DDR_800"; ;;
		*a37xx_*_1200_750 )  build_flags="${build_flags} CLOCKSPRESET=CPU_1200_DDR_750"; ;;
		*a37xx_*_600_600 )   build_flags="${build_flags} CLOCKSPRESET=CPU_600_DDR_600"; ;;
		*a37xx_*_800_800 )   build_flags="${build_flags} CLOCKSPRESET=CPU_800_DDR_800"; ;;
		* ) echo "Error: Could not configure build_flags. Unsupported build ${build_name}"; exit -1; ;;
	esac
	[[ -z ${WTMI_IMG:-} ]] || pm_fw_export="WTMI_IMG=${WTMI_IMG}"
else
	[[ -z ${SCP_BL2:-} ]]  || pm_fw_export="SCP_BL2=${SCP_BL2}"
fi
###############################################################################


## =v=v=v=v=v=v=v=v=v=v=v CI WRAPPER - Do not Modify! v=v=v=v=v=v=v=v=v=v=v= ##
cmd="""
set -x
pwd"""
## =^=^=^=^=^=^=^=^=^=^=^=^  End of CI WRAPPER code -=^=^=^=^=^=^=^=^=^=^=^= ##


if [[ $build_name =~ _a3700_ ]]; then
##[ 3700
##################################### CONFIG ##################################
[[ $no_configure ]] || cmd=$cmd"""
make distclean"""

#################################### COMPILE ##################################
cmd=$cmd"""
make MV_DDR_PATH=$MV_DDR_PATH ${build_flags} PLAT=${platform} ${pm_fw_export} all fip"""
###############################################################################
##]

else
##[ All other SOCs
##################################### CONFIG ##################################
[[ $no_configure ]] || cmd=$cmd"""
${pm_fw_export}
make distclean"""

#################################### COMPILE ##################################
cmd=$cmd"""
make MV_DDR_PATH=$MV_DDR_PATH ${build_flags} PLAT=${platform} all fip"""
###############################################################################
##]
fi


## =v=v=v=v=v=v=v=v=v=v=v CI WRAPPER - Do not Modify! v=v=v=v=v=v=v=v=v=v=v= ##
if [[ $echo_only ]]; then
	echo "$cmd"
	exit 0
fi

eval "$cmd"
## =^=^=^=^=^=^=^=^=^=^=^=^  End of CI WRAPPER code -=^=^=^=^=^=^=^=^=^=^=^= ##
