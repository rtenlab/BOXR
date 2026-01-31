#!/bin/bash
BUILD_DIR=$1
SOURCE_DIR=$2

echo "Patching Audio_Pipeline in $SOURCE_DIR using BUILD_DIR=$BUILD_DIR"
export BUILD_DIR
perl -i -pe 's|pkg_check_modules\(SPATIALAUDIO spatialaudio\)|set(SPATIALAUDIO_INCLUDE_DIRS "$ENV{BUILD_DIR}/install/include")\nset(SPATIALAUDIO_LIBRARIES "spatialaudio")|g' "$SOURCE_DIR/CMakeLists.txt"
