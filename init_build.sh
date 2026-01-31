#!/usr/bin/env bash
rm -rf build/ && mkdir build && cd build
if [ ! -f "data.zip" ]; then
    if [ -f "../data.zip" ]; then
        cp ../data.zip  ./data.zip
    fi
fi
# rm -rf build/ && mkdir build && cp data.zip ./build && cd build # when you obtained the data.zip file, you can save it and do not need to download it again
cmake .. -DCMAKE_BUILD_TYPE=Debug -DYAML_FILE=profiles/native_gl.yaml
cmake --build . -j$(nproc)
cmake --build . -t docs 
cmake --install . --prefix ./install