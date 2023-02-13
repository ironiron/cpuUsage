cmake_minimum_required(VERSION 3.10)

project(cpuUsage LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED OFF)

if("${TARGET}" STREQUAL "unit_tests")
    add_executable(cpuUsage unit_tests.c utils.c utils.h)
endif()

if("${TARGET}" STREQUAL "Debug")
    add_executable(cpuUsage main.c utils.c utils.h)
endif()

target_compile_options(cpuUsage PRIVATE -g -O0)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  target_compile_options(cpuUsage PRIVATE -Weverything -Wno-declaration-after-statement -Wno-padded -Wno-disabled-macro-expansion)
  message("Compiling with Clang")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  target_compile_options(cpuUsage PRIVATE -Wall -Wextra -Wpedantic )
  message("Compiling with GCC")
endif()