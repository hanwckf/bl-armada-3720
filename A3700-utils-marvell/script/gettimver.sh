#!/bin/bash
#
# Get TIM version
#
# $1 - the input txt file which includes TIM version string
# $2 - the output TIM file name
#

VERFILE=$1
OUTFILE=$2

usage () {
	echo ""
	echo "$0 - script for generating the script to get TIM version"
	echo ""
	echo "$0 <ver_txt> <output>"
	echo " <ver_txt>     - the input txt file which includes TIM version string"
	echo " <output>      - the output TIM file name"
	echo ""
	exit 1
}

if [ "$VERFILE" = "" ]; then
	echo "Empty input TIM version file name!"
	usage
fi

if [ "$OUTFILE" = "" ]; then
	echo "Empty output TIM file name!"
	usage
fi

# Get version string and its length
VERSTR=$(cat $VERFILE)
VERLEN=${#VERSTR}

echo "WRITE: 0xC0012004 0x54              ; Print 'T'" >> $OUTFILE
echo "WAIT_FOR_BIT_SET: 0xC001200C 0x20 1 ; Wait for TX ready" >> $OUTFILE
echo "WRITE: 0xC0012004 0x49              ; Print 'I'" >> $OUTFILE
echo "WAIT_FOR_BIT_SET: 0xC001200C 0x20 1 ; Wait for TX ready" >> $OUTFILE
echo "WRITE: 0xC0012004 0x4D              ; Print 'M'" >> $OUTFILE
echo "WAIT_FOR_BIT_SET: 0xC001200C 0x20 1 ; Wait for TX ready" >> $OUTFILE
echo "WRITE: 0xC0012004 0x2D              ; Print '-'" >> $OUTFILE
echo "WAIT_FOR_BIT_SET: 0xC001200C 0x20 1 ; Wait for TX ready" >> $OUTFILE

# Print out TIM version number
for (( i=0; i<$VERLEN; i++ ))
do
	CHARACTER=${VERSTR:$i:1}
	ASCIIVAL=$(printf "%d" \'$CHARACTER)
	echo "WRITE: 0xC0012004" $ASCIIVAL  "; Print tim version number">> $OUTFILE
	echo "WAIT_FOR_BIT_SET: 0xC001200C 0x20 1 ; Wait for TX ready" >> $OUTFILE
done
echo "WRITE: 0xC0012004 0x0D                      ; Print CR" >> $OUTFILE
echo "WAIT_FOR_BIT_SET: 0xC001200C 0x20 1 ; Wait for TX ready" >> $OUTFILE
echo "WRITE: 0xC0012004 0x0A                      ; Print LF" >> $OUTFILE
echo "WAIT_FOR_BIT_SET: 0xC001200C 0x20 1 ; Wait for TX ready" >> $OUTFILE

exit 0
