cmake_minimum_required(VERSION 3.16)
project(UZDoomNativeTools LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}/bin")
endforeach()

# These libraries use this helper in the main project to suppress debug builds.
# This isolated project is already configured and built in Release mode.
macro(make_release_only)
endmacro()

get_filename_component(root "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
add_subdirectory("${root}/libraries/miniz" miniz)
add_subdirectory("${root}/libraries/bzip2" bzip2)
add_subdirectory("${root}/libraries/lzma" lzma)
set(BZIP2_INCLUDE_DIR "${root}/libraries/bzip2")
set(BZIP2_LIBRARIES bz2)
set(LZMA_INCLUDE_DIR "${root}/libraries/lzma/C")
add_subdirectory("${root}/tools" tools)
