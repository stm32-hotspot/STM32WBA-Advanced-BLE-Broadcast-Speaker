#!/bin/bash
# "../copyDelivery.sh" "${FW_NAME}.hex" 

export  BASE_DELIVERY=../../../BinCubeIde

if [ ! -d "$BASE_DELIVERY" ]
then
		mkdir "$BASE_DELIVERY"
fi
basedir=`pwd`
fileOut=$(dirname $(readlink -f "$basedir"))
fileOut=$(basename $fileOut)
echo cp -f "$1" "$BASE_DELIVERY/$2-$fileOut.hex"
cp -f "$1" "$BASE_DELIVERY/$2-$fileOut.hex"




