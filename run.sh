#!/bin/bash

if [ ! -d build ]
then
    mkdir build
    echo "-- Create build folder"
else
    cd build
    echo "-- Exist build folder"
fi
echo "-- Start compile..."
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j4
cd ..
build/VoxelEngine