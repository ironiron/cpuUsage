#!/bin/bash


cmake -S . -B debug -DTARGET=Debug
cmake --build debug
if [ $? -eq 0 ]; then
    ./debug/cpuUsage
else
    echo BUILDING COMMAND FAILED
fi