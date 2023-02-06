#!/bin/bash


cmake -S . -B build
cmake --build build
if [ $? -eq 0 ]; then
    ./build/cpuUsage
else
    echo BUILDING COMMAND FAILED
fi

#./build/cpuUsage

