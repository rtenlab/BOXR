#!/usr/bin/env bash
cd build
cmake --build . -j$(nproc)
cmake --install . --prefix ./install