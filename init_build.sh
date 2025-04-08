#!/bin/bash
rm -rf build/ && mkdir build && cd build
# rm -rf build/ && mkdir build && cp data.zip ./build && cd build # when you obtained the data.zip file, you can save it and do not need to download it again
cmake .. -DCMAKE_BUILD_TYPE=Debug -DYAML_FILE=profiles/native_gl.yaml
cmake --build . -j$(nproc)
cmake --build . -t docs 
cmake --install .