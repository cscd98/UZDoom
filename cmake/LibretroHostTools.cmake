# Cross-built generators cannot run on the build host. Build just the native
# generators and their bundled compression libraries in an isolated project.
include(ExternalProject)

find_program(LIBRETRO_HOST_C_COMPILER NAMES cc gcc clang cl NO_CMAKE_FIND_ROOT_PATH)
find_program(LIBRETRO_HOST_CXX_COMPILER NAMES c++ g++ clang++ cl NO_CMAKE_FIND_ROOT_PATH)
if(NOT LIBRETRO_HOST_C_COMPILER OR NOT LIBRETRO_HOST_CXX_COMPILER)
    message(FATAL_ERROR "Native resource generators require host C and C++ compilers. Set LIBRETRO_HOST_C_COMPILER and LIBRETRO_HOST_CXX_COMPILER.")
endif()

set(host_tools_dir "${CMAKE_BINARY_DIR}/native-tools")
set(host_tool_suffix "")
if(CMAKE_HOST_WIN32)
    set(host_tool_suffix ".exe")
endif()

# Do not inherit target compiler flags or an environment-provided toolchain.
set(host_tools_env ${CMAKE_COMMAND} -E env
    --unset=CC --unset=CXX --unset=CFLAGS --unset=CXXFLAGS --unset=CPPFLAGS
    --unset=LDFLAGS --unset=CMAKE_TOOLCHAIN_FILE)
ExternalProject_Add(libretro_host_tools
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/tools/native"
    BINARY_DIR "${host_tools_dir}"
    CONFIGURE_COMMAND ${host_tools_env} ${CMAKE_COMMAND}
        -S <SOURCE_DIR> -B <BINARY_DIR> -G "${CMAKE_GENERATOR}"
        "-DCMAKE_C_COMPILER:FILEPATH=${LIBRETRO_HOST_C_COMPILER}"
        "-DCMAKE_CXX_COMPILER:FILEPATH=${LIBRETRO_HOST_CXX_COMPILER}"
        -DCMAKE_BUILD_TYPE:STRING=Release
        -DCMAKE_POLICY_VERSION_MINIMUM:STRING=3.5
    BUILD_COMMAND ${host_tools_env} ${CMAKE_COMMAND} --build <BINARY_DIR>
        --config Release --target zipdir lemon re2c
    BUILD_ALWAYS TRUE
    BUILD_BYPRODUCTS
        "${host_tools_dir}/bin/zipdir${host_tool_suffix}"
        "${host_tools_dir}/bin/lemon${host_tool_suffix}"
        "${host_tools_dir}/bin/re2c${host_tool_suffix}"
        "${host_tools_dir}/bin/lempar.c"
    INSTALL_COMMAND "")

foreach(tool zipdir lemon re2c)
    add_executable(${tool} IMPORTED GLOBAL)
    set_target_properties(${tool} PROPERTIES
        IMPORTED_LOCATION "${host_tools_dir}/bin/${tool}${host_tool_suffix}")
    add_dependencies(${tool} libretro_host_tools)
endforeach()
