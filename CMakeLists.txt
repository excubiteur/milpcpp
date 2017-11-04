cmake_minimum_required(VERSION 3.8)

project(milpcpp_all)

set(GLPK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../glpk-4.63)
set(LPSOLVE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lp_solve_5.5.2.5)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../milpcpp.out/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../milpcpp.out/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../milpcpp.out/bin)

add_subdirectory(milpcpp)
add_subdirectory(examples)
