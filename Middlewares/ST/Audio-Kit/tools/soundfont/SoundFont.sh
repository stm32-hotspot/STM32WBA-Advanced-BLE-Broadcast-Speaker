!/bin/bash
#for unix OS, it could be mandatory to do:
# dos2unix  ac_flash_firmware.sh and chmod +x ac_flash_firmware.sh

export PATH_LIST=("/c/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin" "$HOME/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin")
export CUBEPROG="STM32_Programmer_CLI"

function CheckPythonVersion
{
	# Get the Python version
	python_version=$(python --version 2>&1)

	# Check if Python 3 is in the version string
	if [[ $python_version == *"Python 3"* ]]; then
	echo "Python 3 is installed."
	else
	echo "Error: Python 3 is required." >&2
	exit 1
	fi
}

function CheckCubeProgPath
{
	export CUBE_PATH=""
	for i in "${PATH_LIST[@]}"
	do
		if [ -f "$i/STM32_Programmer_CLI.exe" ]; then # for windows
			CUBE_PATH="$i"
			break;
		fi
		echo "$i/STM32_Programmer_CLI"
		if [ -f "$i/STM32_Programmer_CLI" ]; then	# for linux
			CUBE_PATH="$i"
			break;
		fi

	done
	echo $CUBE_PATH
	PATH="$CUBE_PATH":"$PATH"
	check_tool "$CUBEPROG"     "STM32_Programmer_CLI is not found, please install STM32CubeProg from https://www.st.com/en/development-tools/stm32cubeprog.html or update the path CUBE_PATH=$CUBE_PATH"
}

function check_tool
{
export TOOL=$(type -P -a $1 | head -n 1 2>/dev/null)
	while [ "$TOOL" == "" ]; do
		>&2 echo -e $COLOR_RED "----------------------------------------"
		>&2 echo -e  $2
		>&2 echo -e  Fix the issue and try again
		>&2 echo -e "----------------------------------------"$COLOR_NC
		read
	done
}



export PATH=$CUBE_PATH:$PATH

export COLOR_RED='\e[1m\e[4m\e[31m'
export COLOR_NC='\033[0m'
export COLOR_BLUE='\033[0;34m'
export COLOR_YELLOW='\033[1;33m'
export COLOR_GREEN='\e[1m\033[1;32m'

function checkKey
{
	for i in "${listKey[@]}"
	do
		if [ "$i" == "$1" ] ; then
			echo "1"
			return
		fi
	done
	echo "0"

}

function waitType
{
	val_waitType=""
	IFS=':' read -ra listKey <<< "$1"
	string=""
	while [ $(checkKey "$string") == "0" ]

	do
		if [ "$string" == "quit" ] ; then
			exit
		fi

		if [ "$3" != "" ] ;
		then
			read -t $2 -p "$2  or wait $3 seconds > "  string
			if [ "$?" -gt "128" ] ;
			then
			 string=$1
			 echo -n $string
			fi
		else
			read   -p "$2"  string
			if [ "$string" == "" ]; then
				if [ "$4" != "" ]; then
					string=$4
				fi
			fi

		fi
	done
   export val_waitType=$string
}

PATH_TO_MIDDLEWARE="../../../../../../../"
CONVERT_SFF=1

export FOLDER_PERSIST=./.config
export FILE_PERSIST=$FOLDER_PERSIST/config.sh






function select_loader
{
	if [[ $1 = *'STM32H573I-DK'* ]]; then
		export ST_REF_LOADER=MX25LM51245G_STM32H573I-DK-RevB.stldr
		Loader="$CUBE_PATH/ExternalLoader/$ST_REF_LOADER"
		LOADER_EXT="-el ${Loader@Q}"
		export STRUCT_OFFSET=0x90300000
  	export CONNECT_OPTION="-q -c port=swd reset=HWrst mode=UR"
	fi

	if [[ $1 = *'STM32H735G-DK'* ]]; then
		export ST_REF_LOADER=MX25LM51245G_STM32H735G-DK.stldr
		Loader="$CUBE_PATH/ExternalLoader/$ST_REF_LOADER"
		LOADER_EXT="-el ${Loader@Q}"
		export STRUCT_OFFSET=0x90300000
  	export CONNECT_OPTION="-q -c port=swd reset=HWrst mode=UR"
	fi

	if [[ $1 = *'STM32H747I-DISCO'* ]]; then
		export ST_REF_LOADER=MT25TL01G_STM32H747I-DISCO.stldr
		Loader="$CUBE_PATH/ExternalLoader/$ST_REF_LOADER"
		LOADER_EXT="-el ${Loader@Q}"
		#tmp remove verification due to a bug in STM32H747I-DISCO
		VERIFY=""
		export STRUCT_OFFSET=0x90300000
  	export CONNECT_OPTION="-q -c port=swd reset=HWrst mode=UR"
	fi

	if [[ $1 = *'STM32N6570-DK'* ]]; then
		export ST_REF_LOADER=MX66UW1G45G_STM32N6570-DK.stldr
		Loader="$CUBE_PATH/ExternalLoader/$ST_REF_LOADER"
		Loader=$(cygpath -w "$Loader")
		LOADER_EXT="-el \"$Loader\""
		export STRUCT_OFFSET=0x70340000
		export CONNECT_OPTION="-c port=swd mode=HOTPLUG ap=1  freq=200 -hardRst"
	fi

	if [[ $1 = *'STM32H7S78-DK'* ]]; then
		export ST_REF_LOADER=MX66UW1G45G_STM32H7S78-DK.stldr
		Loader="$CUBE_PATH/ExternalLoader/$ST_REF_LOADER"
		LOADER_EXT="-el ${Loader@Q}"
		export STRUCT_OFFSET=0x77220000
		export CONNECT_OPTION="-c port=SWD ap=1"
	fi

	if [ "$Loader" != "" ] ; then
		echo using $ST_REF_LOADER
		if [ ! -f "$Loader"  ] ; then
			echo -e $COLOR_RED"Error: External flash loader not found $ST_REF_LOADER"$COLOR_NC
		fi
	fi

}

function flash_hex
{
	echo "Flashing :"$(basename $1)
	echo $CUBEPROG  "$CONNECT_OPTION" -d $1  $LOADER_EXT   # >/dev/null
	eval $CUBEPROG  "$CONNECT_OPTION" -d "$1"  $LOADER_EXT   # >/dev/null
}

function buid_pkt
{

SelectBoard
select_loader "$board_Prefix"
python ./sf2ToSff.py sf2tosff "$1" "$1.sff"
python ./sf2ToSff.py package  "$1.pkt" --bins "$1.sff"
rm -f "$1.sff" >/dev/null
flashFile=X-CUBE-Audio-kit-${board_Prefix}-${1}-${STRUCT_OFFSET}.hex
python ./sf2ToSff.py bin2hex    --offset $STRUCT_OFFSET $1.pkt  "$flashFile"
echo
echo Flash  Output: $flashFile
echo Bin    Output: "$1.pkt"
echo
waitType "yes:no" "Do you want to flash the binary directly ? (yes/no)> " "" "yes"
if [ "$val_waitType" == "yes" ]; then
	echo select_loader "$board_Prefix"
	flash_hex "$flashFile"
fi
}

function SelectBoard
{
	export board_Prefix=""
	Board=(
	"STM32H573I-DK"
	"STM32H735G-DK"
	"STM32N6570-DK"
  "STM32H7S78-DK"
  "STM32H747I-DISCO"
	)
	select opt in "${Board[@]}"
	do
	   board_Prefix=$opt
	   return
	done
}


function SelectFromFiles
{
	n=0
	listFile=($(ls *.sf2)) 2>/dev/null
	for i in "${listFile[@]}"
	do
	   :
	   echo "$n) $i"
	   n=$((n+1))
	done
	quit=0
	while [ "$quit" == "0" ]
	do
		echo -n -e $COLOR_YELLOW"Select the file> "$COLOR_NC
		read   key
		if [ "$key" != "" ]; then
			if  [ "$key" -ge "0" ] && [ "$key" -lt  "$n" ] ; then
				quit=1
				export SELECTED="${listFile[$((key))]}"
			fi
		fi
	done
}




function menu()
{
S3='Generate ToolSet:'
echo "+-------------------------------------------------------------------------+"
echo "|  Sound Font tool                                                        |"
echo "|  Build a sound fonts                                                    |"
echo "|  Some files are avaiables for free at :                                 |"
echo "|  https://musical-artifacts.com/artifacts?formats=sf2                    |"
echo "|  https://archive.org/download/free-soundfonts-sf2-2019-04               |"
echo "|  https://sites.google.com/site/soundfonts4u/                            |"
echo "|  To play  perfectly the .sf2 must GM ( general midi) compliant          |"
echo "+-------------------------------------------------------------------------+"

while [ 1 ]
	do
		SelectFromFiles
		buid_pkt $SELECTED

	done
}
CheckPythonVersion
CheckCubeProgPath
menu
