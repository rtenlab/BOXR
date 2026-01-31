#!/bin/bash
export PKG_CONFIG_PATH=/home/bowmanhan/boxr/build/install/lib/pkgconfig:$PKG_CONFIG_PATH
echo "PKG_CONFIG_PATH: $PKG_CONFIG_PATH"
pkg-config --list-all | grep spatial
pkg-config --debug --libs spatialaudio
pkg-config --libs zlib
ls -l /home/bowmanhan/boxr/build/install/lib/pkgconfig/
cat /home/bowmanhan/boxr/build/install/lib/pkgconfig/spatialaudio.pc
