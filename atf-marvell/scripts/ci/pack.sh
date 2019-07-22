#!/bin/bash
# SPDX-License-Identifier:           GPL-2.0
# https://spdx.org/licenses
# Copyright (C) 2018 Marvell International Ltd.
#
###############################################################################
## This is the pack script for atf                                           ##
## This script is called by CI automated builds                              ##
###############################################################################
## WARNING: Do NOT MODIFY the CI wrapper code segments.                      ##
## You can only modify the config and compile commands                       ##
###############################################################################
## Prerequisites:       DESTDIR is the path to the destination directory
## Usage:               pack BUILD_NAME

## =v=v=v=v=v=v=v=v=v=v=v CI WRAPPER - Do not Modify! v=v=v=v=v=v=v=v=v=v=v= ##
set -exuo pipefail
shopt -s extglob

build_name=$1
echo "running pack.sh ${build_name}"
## =^=^=^=^=^=^=^=^=^=^=^=^  End of CI WRAPPER code -=^=^=^=^=^=^=^=^=^=^=^= ##

case $build_name in
	*_a70x0_pcap )   platform="a70x0_pcac"; ;;
	*_a70x0* )       platform="a70x0"; ;;
	*_a7020_amc )    platform="a70x0_amc"; ;;
	*_a80x0_mcbin* ) platform="a80x0_mcbin"; ;;
	*_a80x0* ) 	 platform="a80x0"; ;;
	*_a3900_z1* )    platform="a3900_z1"; ;;
	*_a3900_z2* )    platform="a3900_z2"; ;;
	*_a3900* )       platform="a3900"; ;;
	*_a37xx* )       platform="a3700"; ;;
	* )	echo "Error: Could not configure platform."
	             "Unsupported build ${build_name}"; exit -1; ;;
esac

mkdir -p $DESTDIR/dev_images
module_file="build/${platform}/release/flash-image.bin"
cp $module_file $DESTDIR/

#echo 'atf.pack: copying .bin .elf .txt files'
find . -name *.bin | xargs cp -u --target-directory=$DESTDIR/dev_images/
find . -name *.elf | xargs cp -u --target-directory=$DESTDIR/dev_images/

if [[ $platform == "a3700" ]]; then
	#echo 'atf.pack: copying uart-images'
	find . -name uart-images.tgz | \
		xargs cp --target-directory=$DESTDIR/dev_images/
fi
