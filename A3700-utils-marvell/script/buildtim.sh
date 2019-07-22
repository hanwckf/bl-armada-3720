#!/bin/bash
#
# Create TIM/NTIM descriptor
#
# $1 - trusted/non trusted (1/0)
# $2 - Boot device (SPINOR/SPINAND/EMMCNORM/EMMCALT/SATA/UART)
# $3 - Path to image text files
# $4 - WTP path
# $5 - Clocks preset
# $6 - DDR topology map
# $7 - Partition number
# $8 - Enable debug message (1/0)
# $9 - Output TIM/NTIM file name
# $10 - Output TIMN file name (valid for trusted mode only)
# $11 - TIMN CSK sign key file name (valid for trusted mode only)
# $12 - Encrypt the image (valid for trusted mode only)
#

DATE=`date +%d%m%Y`
BOOT_DEV=$2
IMGPATH=$3
WTPPATH=$4
PRESET=$5
DDRTOPOLOGY=$6
BOOTPART=$7
DEBUG=$8
OUTFILE=$9

CLOCKSPATH=$WTPPATH/tim/ddr
SYSINITPATH=$WTPPATH/wtmi/sys_init
SYSINITLOADADDR=`sed -n '/. = 0x1fff[0-9|a-f]*/p' $SYSINITPATH/sys_init.ld | awk '{print $3}' | cut -d ';' -f 1`

# All file names extention
FILEEXT="txt"
# Image definition files name prefix
IMGPREF="img"
# Number of images excepting TIM
IMGSNUM=`ls -l $IMGPATH/$IMGPREF*.$FILEEXT | wc -l`
# Reserved area definition for TIM/NTIM - file name prefix
RSRVDPREF="rsrvd"
RSRVDLEN=`wc -l < $IMGPATH/$RSRVDPREF.$FILEEXT`

# DDR static - auto-generated file according
# to the specified DDR topology map profile
DDRFILE=$CLOCKSPATH/ddr_static.$FILEEXT


# DLL tuning - same for all DDR frequencies
# Located in the same folder as DDR init file
DLLTUNFILE=$CLOCKSPATH/dll_tune.$FILEEXT
GETDDRPARAMS=$WTPPATH/script/getddrparams.sh

# TIM/NTIM image definition file name prefix
TIMPREF="tim"
# CSK keys file name prefix
CSKPREF="csk"
# KAK key file name prefix (TIM signature)
KAKPREF="kak"

# Below values used only by TIMN
TIMNOUTFILE=${10}
SIGNFILE=${11}
ENCRYPT=${12}
# TIMN image definition file name prefix
TIMNPREF="timn"
# Reserved area definition for TIMN - file name prefix
RSRVD2PREF="rsrvdtimn"
# General Purpose Patch files -
# can be different for trusted/non-trusted boot modes
GPP1PREF="gpp1"
GPP2PREF="gpp2"
GPP1FILE="$IMGPATH/$GPP1PREF.$FILEEXT"
GPP2FILE="$IMGPATH/$GPP2PREF.$FILEEXT"

# TIM version
TIMDIR=$IMGPATH/..
VERPREF="version"
VERFILE="$TIMDIR/$VERPREF.$FILEEXT"
SCRIPTPATH=$(dirname "$0")
TIMGETVER=$SCRIPTPATH/gettimver.sh

usage () {
	echo ""
	echo "$0 - script for creating TIM/NTIM/TIMN image descriptors"
	echo ""
	echo "$0 <trusted> <boot_device> <files_path> <clocks_path> <clocks_preset> <ddr_topology>"\
		" <part_num> <debug> <output> [timN_out] [timN_key]"
	echo " <trusted>     - trusted/non trusted (supported values 0 and 1)"
	echo " <boot_device> - Boot device (Supported values SPINOR/SPINAND/EMMCNORM/EMMCALT/SATA/UART)"
	echo " <files_path>  - Path to image and keys descriptors text files"
	echo " <clocks_path> - Path to clocks and DDR initialization files"
	echo " <clocks_preset> - Name of clocks preset to use - see \"build.txt\" for details"
	echo " <ddr_topology> - Index of DDR topology map"
	echo " <part_num>    - partition number of the boot device"
	echo " <debug>       - debug flag to enable TIM DDR package's log"
	echo " <output>      - Output TIM/NTIM file name"
	echo " [timN_out]    - Output TIMN file name (required for trusted boot only)"
	echo " [timN_key]    - TIMN CSK sign key file name (required for trusted boot only)"
	echo " [E]           - Encrypt the binary image or not (supported values 0 and 1, trusted boot only)"
	echo ""
	exit 1
}

static_ddr(){
	if [ -z "$1" ]; then
		echo "static_ddr(): NO output file!"
		exit 1
	fi

	DDROUTFILE=$1

	if [ ! -e "$DDRFILE" ]; then
		# DDR init is mandatory
		echo "Cannot find $DDRFILE file!"
		exit 1
	else
		echo "Extended Reserved Data:" >> $DDROUTFILE
		echo "Consumer ID:" >> $DDROUTFILE
		echo "CID: TBRI" >> $DDROUTFILE

		# GPPn packages are optional
		if [ -e "$GPP1FILE" ]; then
			echo "PID: GPP1" >> $DDROUTFILE
		fi
		if [ -e "$GPP2FILE" ]; then
			echo "PID: GPP2" >> $DDROUTFILE
		fi
		echo "PID: DDR3" >> $DDROUTFILE
		echo "End Consumer ID:" >> $DDROUTFILE

		# GPPn packages are optional
		if [ -e "$GPP1FILE" ]; then
			echo "GPP:" >> $DDROUTFILE
			echo "GPP_PID: GPP1" >> $DDROUTFILE
			echo "GPP Operations:" >> $DDROUTFILE
			echo "GPP_IGNORE_INST_TO: 0x0" >> $DDROUTFILE
			echo "End GPP Operations:" >> $DDROUTFILE
			echo "Instructions:" >> $DDROUTFILE
			if [ $DEBUG == "1" ]; then
				awk -F ";<DEBUG-INFO>" '{if ($0 !~ /DEBUG-INFO/) {print $0} else {print $2}}' \
					$GPP1FILE >> $DDROUTFILE.temp
			else
				awk '{if ($0 !~ /DEBUG-INFO/) {print $0}}' $GPP1FILE >> $DDROUTFILE.temp
			fi
			if [ "$BOOT_DEV" != "UART" ]; then
				awk -F ";<NON_UART>" '{if ($0 !~ /NON_UART/) {print $0} else {print $2}}' \
					$DDROUTFILE.temp >> $DDROUTFILE
			else
				awk '{if ($0 !~ /NON_UART/) {print $0}}' $DDROUTFILE.temp >> $DDROUTFILE
			fi
			rm $DDROUTFILE.temp

			# TIM version printing
			if [ "$BOOT_DEV" != "UART" ]; then
				# Read TIM version from "version.txt" file
				$TIMGETVER $VERFILE $DDROUTFILE
			fi
			echo "End Instructions:" >> $DDROUTFILE
			echo "End GPP:" >> $DDROUTFILE
		fi

		if [ -e "$GPP2FILE" ]; then
			echo "GPP:" >> $DDROUTFILE
			echo "GPP_PID: GPP2" >> $DDROUTFILE
			echo "GPP Operations:" >> $DDROUTFILE
			echo "GPP_IGNORE_INST_TO: 0x0" >> $DDROUTFILE
			echo "End GPP Operations:" >> $DDROUTFILE
			echo "Instructions:" >> $DDROUTFILE
			cat $GPP2FILE >> $DDROUTFILE
			echo "End Instructions:" >> $DDROUTFILE
			echo "End GPP:" >> $DDROUTFILE
		fi

		echo "DDR Initialization:" >> $DDROUTFILE
		echo "DDR_PID: DDR3" >> $DDROUTFILE
		echo "Operations:" >> $DDROUTFILE
		echo "DDR_INIT_ENABLE: 0x00000001" >> $DDROUTFILE
		echo "End Operations:" >> $DDROUTFILE
		echo "Instructions:" >> $DDROUTFILE
		cat $CLOCKSFILE >> $DDROUTFILE
		cat $DDRFILE >> $DDROUTFILE
		echo "End Instructions:" >> $DDROUTFILE
		echo "End DDR Initialization:" >> $DDROUTFILE

		echo "End Extended Reserved Data:" >> $DDROUTFILE
		echo "" >> $DDROUTFILE
	fi
}

# Parameters check
case "$1" in
[0-1])
	TRUSTED="0x0000000"$1
	if [ "$TRUSTED" = "0x00000001" ]; then
		if [ "$TIMNOUTFILE" = "" ]; then
			echo "Empty TIMN output file name!"
			usage
		fi

		# Values required for trusted boot mode
		KEYSNUM=`ls -l $IMGPATH/$CSKPREF*.$FILEEXT | wc -l`
		RSRVD2LEN=`wc -l < $IMGPATH/$RSRVD2PREF.$FILEEXT`
	fi
	;;
*)
	echo "Only 0/1 is supported as first parameter (trusted/untrusted)!"
	usage
esac

case $BOOT_DEV in
SPINOR)
	FLASH="0x5350490A		; Select SPI'0A"
	;;
SPINAND)
	FLASH="0x5350491A		; Select SPI'1A"
	;;
EMMCNORM)
	FLASH="0x454D4D08		; Select MMC'08"
	;;
EMMCALT)
	FLASH="0x454D4D0B		; Select MMC'0B"
	;;
SATA)
	FLASH="0x53415432		; Select SAT'32"
	;;
UART)
	FLASH="0x55415223		; Select UAR'23"
	;;
*)
	echo "Unsupported boot device $2!"
	usage
esac

case "$DDRTOPOLOGY" in
CUST | [0-6])
	DDRTOPFILE=$CLOCKSPATH/DDR_TOPOLOGY_$DDRTOPOLOGY.$FILEEXT
	;;
*)
	echo "Unknown DDR topology map - $CLOCKSPATH/DDR_TOPOLOGY_$DDRTOPOLOGY.$FILEEXT!"
	usage
esac

CLOCKSFILE=$CLOCKSPATH/clocks_ddr.txt
# All DDR use the configuration for 800M
DDRTYPE=`$GETDDRPARAMS $DDRTOPFILE ddr_type`

$CLOCKSPATH/ddr_tool -i $DDRTOPFILE -o $CLOCKSPATH/ddr_static.txt

if [ $? -ne 0 ]; then
	echo "DDR_tool fails to run!"
	exit 1
fi

if [ ! -e $CLOCKSPATH/ddr_static.txt ]; then
	echo "Cannot find ddr_static.txt file!"
	exit 1
elif [ ! -s $CLOCKSPATH/ddr_static.txt ]; then
	echo "ddr_static.txt file is empty!"
	exit 1
fi

if [ $DDRTYPE = "0" ]; then
		cp -f $CLOCKSPATH/clocks-ddr3.txt $CLOCKSPATH/clocks_ddr.txt
elif [ $DDRTYPE = "1" ]; then
		cp -f $CLOCKSPATH/clocks-ddr4.txt $CLOCKSPATH/clocks_ddr.txt
fi

if [ ! -e "$DDRFILE" ]; then
	echo "Cannot find DDR init file!"
	usage
fi

if [ "$OUTFILE" = "" ]; then
	echo "Empty TIM/NTIM output file name!"
	usage
fi

# Build TIM/NTIM descriptor header

# Add TIM/NTIM to the images number, we count it too
let IMGSNUM=IMGSNUM+1
# Reserved section is in rows (one row per word), we need it in bytes
let RSRVDLEN=RSRVDLEN*4

echo "Version:                        0x00030600" > $OUTFILE
echo "Trusted:                        $TRUSTED" >> $OUTFILE
echo "Issue Date:                     0x$DATE		; today's date" >> $OUTFILE
echo "OEM UniqueID:                   0x4D52564C		; MRVL" >> $OUTFILE
echo "Stepping:                       0x00000000" >> $OUTFILE
echo "Processor Type:                 <undefined>		; ARMADA3700" >> $OUTFILE
echo "Boot Flash Signature:           $FLASH" >> $OUTFILE
if [ "$TRUSTED" = "0x00000000" ]; then
	echo "Number of Images:               $IMGSNUM" >> $OUTFILE
else
	# Trusted TIM has only TIMH image
	echo "Number of Images:                1" >> $OUTFILE
	echo "Number of Keys:                  $KEYSNUM		; DSIG KAK key is not counted" >> $OUTFILE
fi
echo "Size of Reserved in bytes:      $RSRVDLEN" >> $OUTFILE
echo ""  >> $OUTFILE

# Images or CSK keys block

if [ "$TRUSTED" = "0x00000000" ]; then

	# First image is always TIMH

	TIMHFILE="$IMGPATH/$TIMPREF.$FILEEXT"
	if [ ! -e "$TIMHFILE" ]; then
		echo "Cannot find $TIMHFILE file!"
		exit 1
	else
		cat $TIMHFILE >> $OUTFILE
		echo "" >> $OUTFILE
	fi

	# Untrusted (NTIM) lists images before reserved area
	i=1
	while [ "$i" -lt "$IMGSNUM" ]; do
		IMAGE="$IMGPATH/$IMGPREF-$i.$FILEEXT"
		if [ ! -e "$IMAGE" ]; then
			echo "Cannot find $IMAGE file!"
			exit 1
		fi
		cat $IMAGE >> $OUTFILE
		echo "" >> $OUTFILE
		let i=i+1
	done
else
	# First image is always TIMH

	TIMHFILE="$IMGPATH/$TIMPREF.$FILEEXT"
	if [ ! -e "$TIMHFILE" ]; then
		echo "Cannot find $TIMHFILE file!"
		exit 1
	else
		#trusted uart mode, should load TIMH to 0x20002000
		if [ "$BOOT_DEV" = "UART" ]; then
			while IFS='' read -r line; do
				if [[ "$line" == *"Load Address:"* ]]; then
					echo "Load Address:                   0x20002000		; TIM ISRAM addr" >> $OUTFILE
				else
					echo "$line" >> $OUTFILE
				fi
			done < $TIMHFILE
		else
			cat $TIMHFILE >> $OUTFILE
		fi
		echo "" >> $OUTFILE
	fi

	# Trusted (TIM) has CSK keys block before reserved area
	if [ "$KEYSNUM" != "16" ]; then
		echo "The number of csk files is less than 16!"
		exit 1
	fi

	i=0
	while [ "$i" -le "$[$KEYSNUM - 1]" ]; do
		IMAGE="$IMGPATH/$CSKPREF-$i.$FILEEXT"
		if [ ! -e "$IMAGE" ]; then
			echo "Cannot find $IMAGE file!"
			exit 1
		fi
		cat $IMAGE >> $OUTFILE
		echo "" >> $OUTFILE
		let i=i+1
	done
fi

# Reserved area

RSRVDFILE="$IMGPATH/$RSRVDPREF.$FILEEXT"
if [ ! -e "$RSRVDFILE" ]; then
	echo "Cannot find $RSRVDFILE file!"
	exit 1
else
	echo "Reserved Data:" >> $OUTFILE
	sed 's|EXEC_ADDR|'$SYSINITLOADADDR'|1' $RSRVDFILE >> $OUTFILE
	echo "" >> $OUTFILE
fi

# Extended reserved area - GPP actions and DDR init
# For non-trusted mode move DDR init to TIMH
if [[ "$TRUSTED" = "0x00000000" ]]; then
	static_ddr $OUTFILE
fi

# Set correct partition number in the output
mv $OUTFILE $OUTFILE.temp
while IFS='' read -r line; do
	if [[ "$line" == *"Partition Number:"* ]]; then
		echo "Partition Number:               $BOOTPART" >> $OUTFILE
	else
		echo "$line" >> $OUTFILE
	fi
done < $OUTFILE.temp
rm $OUTFILE.temp

# Untrusted NTIM does not require more operations, continue with trusted stuff
if [ "$TRUSTED" = "0x00000001" ]; then
	# Trusted TIM needs a KAK key for header signature
	KAKFILE="$IMGPATH/$KAKPREF.$FILEEXT"
	if [ ! -e "$KAKFILE" ]; then
		echo "Cannot find $KAKFILE file!"
		exit 1
	else
		cat $KAKFILE >> $OUTFILE
		echo "" >> $OUTFILE
	fi
	# No more operations for TIM file

	# Now the TIMN file has to be created
	# Reserved section is in rows (one row per word), we need it in bytes
	let RSRVD2LEN=RSRVD2LEN*4

	echo "Version:                        0x00030600" > $TIMNOUTFILE
	echo "Trusted:                        $TRUSTED" >> $TIMNOUTFILE
	echo "Issue Date:                     0x$DATE		; today's date" >> $TIMNOUTFILE
	echo "OEM UniqueID:                   0x4D52564C		; MRVL" >> $TIMNOUTFILE
	echo "Stepping:                       0x00000000" >> $TIMNOUTFILE
	echo "Processor Type:                 <undefined>		; ARMADA3700" >> $TIMNOUTFILE
	echo "Boot Flash Signature:           $FLASH" >> $TIMNOUTFILE
	echo "Number of Images:               $IMGSNUM" >> $TIMNOUTFILE
	echo "Number of Keys:                 0			; DSIG key is not counted" >> $TIMNOUTFILE
	echo "Size of Reserved in bytes:      $RSRVD2LEN" >> $TIMNOUTFILE
	echo "" >> $TIMNOUTFILE

	# TIMN header

	TIMNHFILE="$IMGPATH/$TIMNPREF.$FILEEXT"
	if [ ! -e "$TIMNHFILE" ]; then
		echo "Cannot find $TIMNFILE file!"
		exit 1
	else
		#trusted uart mode, should load TIMN to 0x20006000
		if [ "$BOOT_DEV" = "UART" ]; then
			while IFS='' read -r line; do
				if [[ "$line" == *"Load Address:"* ]]; then
					echo "Load Address:                   0x20006000		; TIMN ISRAM addr" >> $TIMNOUTFILE
				else
					echo "$line" >> $TIMNOUTFILE
				fi
			done < $TIMNHFILE
		else
			cat $TIMNHFILE >> $TIMNOUTFILE
		fi
		echo "" >> $TIMNOUTFILE
	fi

	# TIMN images

	i=1
	while [ "$i" -lt "$IMGSNUM" ]; do
		IMAGE="$IMGPATH/$IMGPREF-$i.$FILEEXT"
		if [ ! -e "$IMAGE" ]; then
			echo "Cannot find $IMAGE file!"
			exit 1
		fi
		if [[ "$ENCRYPT" = "1" && "$BOOT_DEV" != "UART" ]]; then
			# If the boot image has to be encrypted, change the algorithm ID and size
			while IFS='' read -r line; do
				if [[ "$line" == *"Encrypt Algorithm ID:"* ]]; then
					echo -n "Encrypt Algorithm ID:           0x0001E005"  >> $TIMNOUTFILE
					echo "		; AES_TB_CTS_CBC256" >> $TIMNOUTFILE
				elif [[ "$line" == *"Encrypt Size:"* ]]; then
					echo "Encrypt Size:                   0xFFFFFFFF" >> $TIMNOUTFILE
				else
					echo "$line" >> $TIMNOUTFILE
				fi
			done < $IMAGE
		else
			cat $IMAGE >> $TIMNOUTFILE
		fi
		echo "" >> $TIMNOUTFILE
		let i=i+1
	done

	# Replace partition number in the output for EMMC
	if [[ "$BOOT_DEV" = "EMMCNORM" || "$BOOT_DEV" = "EMMCALT" ]]; then
		mv $TIMNOUTFILE $TIMNOUTFILE.temp
		while IFS='' read -r line; do
			if [[ "$line" == *"Partition Number:"* ]]; then
				echo "Partition Number:               $BOOTPART" >> $TIMNOUTFILE
			else
				echo "$line" >> $TIMNOUTFILE
			fi
		done < $TIMNOUTFILE.temp
		rm $TIMNOUTFILE.temp
	fi

	# Second reserved area

	RSRVD2FILE="$IMGPATH/$RSRVD2PREF.$FILEEXT"
	if [ ! -e "$RSRVD2FILE" ]; then
		echo "Cannot find $RSRVD2FILE file!"
		exit 1
	else
		echo "Reserved Data:" >> $TIMNOUTFILE
		sed 's|EXEC_ADDR|'$SYSINITLOADADDR'|1' $RSRVD2FILE >> $TIMNOUTFILE
		echo "" >> $TIMNOUTFILE
	fi

	# For trusted mode move DDR init to TIMN for reducing
	# the TIMH size (limited to 12KB)
	static_ddr $TIMNOUTFILE

	# Last TIMN component is the CSK key for signature creation

	if [ ! -e "$SIGNFILE" ]; then
		echo "Cannot find $SIGNFILE file!"
		exit 1
	else
		cat $SIGNFILE >> $TIMNOUTFILE
		echo "" >> $TIMNOUTFILE
		exit 0
	fi
fi

exit 0
