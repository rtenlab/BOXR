#!/bin/bash

# put in ILLIXR/build 
# run with sudo 

rm -rf build/ && mkdir build && cp data.zip ./build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DYAML_FILE=profiles/native_gl.yaml
cmake --build . -j$(nproc)
cmake --build . -t docs 
cmake --install .