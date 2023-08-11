#!/bin/bash

mkdir build
cd build
cmake ..
cmake --build .
cp compile_commands.json ..
cd ..

