#!/usr/bin/env bash

# Use local build libraries
export LD_LIBRARY_PATH=$(pwd)/build/install/lib:$LD_LIBRARY_PATH

echo "Debug: LD_LIBRARY_PATH=$LD_LIBRARY_PATH"

./build/install/bin/main.dbg.exe "$@"