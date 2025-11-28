#!/bin/sh
#OVERWRITE="--basePath ../../src/"  # uncomment to generate files in the project 

#notice: This bash file as been configured for windows, for other OS , you must update ythepath and faust exec name

export PATH=/c/data/Tools/Faust/bin:$PATH
	cat ./readme.txt
echo faust -lang c -ct 1 -es 1 -mcd 16  -single -ftz 0 ./faust_example.faust >faust_example.c
faust -lang c -ct 1 -es 1 -mcd 16 -single -ftz 0 ./faust_example.faust >faust_example.c

echo python3 Faust2AC.py   generate_ac        $OVERWRITE --template generator  --name faust-distortion --group  myFaust faust_example.c
python3 Faust2AC.py   generate_ac        $OVERWRITE --template generator  --name faust-distortion --group  myFaust faust_example.c
read