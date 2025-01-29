#!/bin/bash



function delete {
    echo "[RUN SCRIPT] Delete build directory"
    rm -rf build
}


function build {
    echo "[RUN SCRIPT] Build project"
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . -j$(nproc)
    cd ..
}


function rebuild {
    delete
    build
}


run=true
function norun {
    echo "[RUN SCRIPT] Build without run"
    run=
}


while [ -n "$1" ]; do
    case "$1" in
        -d | --delete) delete ;;
        -b | --build) build ;;
        -r | --rebuild) rebuild ;;
        -R | --norun) norun ;;
        *) echo "[RUN SCRIPT] Unknown argument: $1"
           norun
           break ;;
    esac
    shift
done


if [[ $run ]]; then
    echo "[RUN SCRIPT] Run project"
    ./build/VoxelEngine
fi