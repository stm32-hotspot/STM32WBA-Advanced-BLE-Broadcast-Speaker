cmake_minimum_required (VERSION 3.14)
#SET(ROOT ../../../)

# Define the project
project (testcmsisdsp VERSION 0.1)

# Define the path to CMSIS-DSP (ROOT is defined on command line when using cmake)
set(DSP ${ROOT}/CMSIS/DSP)
set(DEPS_CMAKE ${ROOT}/CMSIS/DSP/Lib/presets/configs)

# Add DSP folder to module path
list(APPEND CMAKE_MODULE_PATH ${DSP})
list(APPEND CMAKE_MODULE_PATH ${DEPS_CMAKE})
include_directories(${DEPS_CMAKE})

########### 
#
# CMSIS DSP
#

# Load CMSIS-DSP definitions. Libraries will be built in bin_dsp
add_subdirectory(${DSP}/Source bin)
