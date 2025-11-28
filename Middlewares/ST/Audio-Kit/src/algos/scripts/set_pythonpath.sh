#!/bin/bash

# Get the current directory
CURRENT_DIR=$(pwd)/..

CURRENT_DIR=$(realpath "$CURRENT_DIR")



# Add the current directory to PYTHONPATH
if [ -z "$PYTHONPATH" ]; then
	export PYTHONPATH="$CURRENT_DIR"
else
	export PYTHONPATH="$CURRENT_DIR:$PYTHONPATH"
fi

# Optionally, you can echo the PYTHONPATH to verify it's been set
echo "PYTHONPATH is now: $PYTHONPATH"

# To make this change permanent for all future terminal sessions, you can add it to your shell profile
# echo "export PYTHONPATH=\"$CURRENT_DIR:\$PYTHONPATH\"" >> ~/.bashrc