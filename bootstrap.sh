#!/bin/sh

rm -rf build
mkdir build
cd build
cmake -GNinja ../src -DCMAKE_BUILD_TYPE=Debug

