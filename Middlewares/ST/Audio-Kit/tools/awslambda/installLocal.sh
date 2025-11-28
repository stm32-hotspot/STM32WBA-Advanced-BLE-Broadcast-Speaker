#!/bin/sh

# the AWS server less need absolutely a lambda python and subfolders dependencies
# to make sure the python files are up to date and keep the Devl organization unchanged
# we replicate by copy the folder organization locally ( few files)
# then we can run the local server or build the aws lambda distribution
# files copied can be deleted after usage
#set -x

mkdir -p fbeamforming/src/scripts
mkdir -p scripts/core

cp  ../../src/algos/fbeamforming/src/scripts/algo.py  fbeamforming/src/scripts
cp  ../../src/algos/scripts/core/algo.py              scripts/core
cp  ../../src/algos/scripts/core/utils.py             scripts/core
cp  ../../src/algos/scripts/core/keys.py              scripts/core
cp  ../../src/algos/scripts/core/audio_chunk.py       scripts/core
cp  ../../src/algos/scripts/core/audio_chain_const.py scripts/core
