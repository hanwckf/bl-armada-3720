#!/bin/bash
#
# Get DDR topology parameters
#
# $1 - DDR topology file
# $2 - parameter name
#

usage () {
	echo ""
	echo "$0 - script for getting parameter in the specific DDR topology file"
	echo ""
	echo "$0 <topology_file> <parameter_name>"
	echo "  <topology_file>  - full path and file name of DDR topology file"
	echo "  <parameter_name> - DDR parameter name"
	echo ""
	echo "supported parameters:"
	echo "  ddr_type"
	echo "  ddr_speedbin_index"
	echo "  ddr_bus_width_index"
	echo "  ddr_cs_mask"
	echo "  ddr_mem_size_index"
	exit 1
}

value=`cat $1|grep "$2"|awk -F '=' '{print $2}'`

case $2 in
ddr_type|ddr_speedbin_index)
	;;
ddr_bus_width_index)
	bw_index=$value
	if [ $bw_index == "1" ]; then
		bw=8
	elif [ $bw_index == "2" ]; then
		bw=16
	else
		echo "unsupported ddr bus width $bw_index!"
		exit 1
	fi
	value=$bw
	;;
ddr_cs_mask)
	cs_mask=$value
	# cs_mask is a bit map of active DDR CS
	# support up to DDR 2CS
	#   b'01 = 1CS
	#   b'11 = 2CS
	if [ $cs_mask == "1" ]; then
		cs_num=1
	elif [ $cs_mask == "3" ]; then
		cs_num=2
	else
		echo "unsupported cs mask $cs_mask!"
		exit 1
	fi
	value=$cs_num
	;;
ddr_mem_size_index)
	mem_size_index=$value
	mem_size=64
	# memory size = 64(MiB) x index
	while [ $mem_size_index -gt 0 ]
	do
		let mem_size=mem_size*2
		let mem_size_index=mem_size_index-1
	done
	value=$mem_size
	;;
*)
	echo "unsupported DDR parameter $2!"
	usage
	exit 1
esac

# print the value of DDR parameter
echo $value
exit 0
