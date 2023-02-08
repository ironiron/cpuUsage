#!/bin/bash


cmake -S . -B tests -DTARGET=unit_tests
cmake --build tests
if [ $? -eq 0 ]; then
    ./tests/cpuUsage
else
    echo BUILDING COMMAND FAILED
fi

#./build/cpuUsage

