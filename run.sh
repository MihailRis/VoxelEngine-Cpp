mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j 16
cd ..
build/VoxelEngine
