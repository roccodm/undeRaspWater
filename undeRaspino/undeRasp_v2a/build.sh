#!/bin/sh

SCRIPT="$0"
FILES="undeRasp_v2a.ino"
BOARD="arduino:avr:nano:cpu=atmega328"
PORT="/dev/ttyUSB0"

quit() {
	echo $1
	exit 1;
}

if [ -z "$ARD" ]; then
	ARD="`which arduino`"
	echo "Using default arduino $ARD"
fi
if [ -z "$ARD" ]; then
	quit "Unabled to find arduino bin, specify with 'ARD=/path/to/arduino $SCRIPT'"
fi
if [ ! -f "$ARD" ]; then
	quit "Invalid arduino bin '$ARD': no such file"
fi

echo "======================"
echo "Verifing..."
echo "======================"

$ARD --verify $FILES --board $BOARD --port $PORT || quit "Verification failed"

echo "======================"
echo "Verified, uploading..."
echo "======================"

$ARD --upload $FILES --board $BOARD --port $PORT || quit "Upload failed"

echo "======================"
echo "Uploaded"
echo "======================"
