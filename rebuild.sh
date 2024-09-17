#!/bin/bash
cd build
cmake --build . -j$(nproc)
cmake --install .