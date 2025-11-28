#!/bin/sh
#copy original files locally to use lambdafunction in the same context that aws 
. ./installLocal.sh


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

CheckPythonVersion

python -u ./localServer-127.0.0_1234.py

