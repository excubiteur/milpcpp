cmake_minimum_required(VERSION 3.8)

project (examples)

file(GLOB SRC src/*.cpp)

include_directories( ../milpcpp/include )

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()


add_executable(examples ${SRC} )

file(TO_NATIVE_PATH ${GLPK_DIR}/w64/glpk_4_63.lib GLPK_LIB)
file(TO_NATIVE_PATH ${LPSOLVE_DIR}/lpsolve55.lib LPSOLVE_LIB)

file(TO_NATIVE_PATH ${GLPK_DIR}/w64 GLPK_LIB_DIR)
file(TO_NATIVE_PATH ${LPSOLVE_DIR} LPSOLVE_LIB_DIR)

file(TO_NATIVE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} EXECUTABLES_DIR)

target_link_libraries (examples milpcpp ${GLPK_LIB} ${LPSOLVE_LIB} )

add_custom_command(TARGET examples POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${GLPK_LIB_DIR}\\glpk_4_63.dll ${EXECUTABLES_DIR}\\Debug
)

add_custom_command(TARGET examples POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${LPSOLVE_LIB_DIR}\\lpsolve55.dll ${EXECUTABLES_DIR}\\Debug
)