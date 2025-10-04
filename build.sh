#!/bin/sh

rm -rf build > /dev/null
mkdir build && cd build
cmake ../
cmake --build .