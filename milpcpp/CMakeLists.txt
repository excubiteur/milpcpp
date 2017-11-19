cmake_minimum_required(VERSION 3.8)

project (milpcpp)

file(GLOB SRC src/*.cpp)

file(GLOB_RECURSE HEADER include/*.h)

include_directories( include ${GLPK_DIR}/src  ${LPSOLVE_DIR}  ${RANGEV3_DIR}/include )

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()


add_library(milpcpp STATIC ${SRC} ${HEADER})