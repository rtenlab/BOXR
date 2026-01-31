#!/bin/bash
ls -l /home/bowmanhan/boxr/build/install/lib/pkgconfig/spatialaudio.pc
export PKG_CONFIG_PATH=/home/bowmanhan/boxr/build/install/lib/pkgconfig:$PKG_CONFIG_PATH
pkg-config --print-errors --libs spatialaudio
