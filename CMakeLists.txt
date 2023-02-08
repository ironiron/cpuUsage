cmake_minimum_required(VERSION 3.10)

project(cpuUsage LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

if("${TARGET}" STREQUAL "unit_tests")
    add_executable(cpuUsage unit_tests.c utils.c utils.h)

else()
    add_executable(cpuUsage main.c utils.c utils.h)

    if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
      target_compile_options(cpuUsage PRIVATE -Weverything)
      message("Compiling with Clang")
    endif()

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
      target_compile_options(cpuUsage PRIVATE -Wall -Wextra -Wpedantic )
      message("Compiling with GCC")
    endif()
endif()