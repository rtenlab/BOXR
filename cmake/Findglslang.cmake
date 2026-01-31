# Findglslang.cmake
#
# Finds the glslang library
# This will define the following variables:
#
#    glslang_FOUND
#    glslang_INCLUDE_DIRS
#    glslang_LIBRARIES

find_path(glslang_INCLUDE_DIR
    NAMES glslang/Public/ShaderLang.h glslang/ShaderLang.h
    PATH_SUFFIXES include
)

find_library(glslang_LIBRARY NAMES glslang)
find_library(OGLCompiler_LIBRARY NAMES OGLCompiler)
find_library(OSDependent_LIBRARY NAMES OSDependent)
find_library(SPIRV_LIBRARY NAMES SPIRV)
find_library(HLSL_LIBRARY NAMES HLSL)

set(glslang_LIBRARIES
    ${glslang_LIBRARY}
    ${OGLCompiler_LIBRARY}
    ${OSDependent_LIBRARY}
    ${SPIRV_LIBRARY}
    ${HLSL_LIBRARY}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glslang DEFAULT_MSG
    glslang_INCLUDE_DIR
    glslang_LIBRARY
)

if(glslang_FOUND)
    set(glslang_INCLUDE_DIRS ${glslang_INCLUDE_DIR})
endif()

mark_as_advanced(
    glslang_INCLUDE_DIR
    glslang_LIBRARY
    OGLCompiler_LIBRARY
    OSDependent_LIBRARY
    SPIRV_LIBRARY
    HLSL_LIBRARY
)
