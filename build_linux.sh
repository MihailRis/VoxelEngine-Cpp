#!/bin/bash
# required sudo mode

install_lua(){
    echo "-- install linux lua-jit..."
    git clone https://luajit.org/git/luajit.git
    cd luajit
    make && sudo make install INSTALL_INC=/usr/include/lua
    cd .. && rm -d -r luajit
    echo "-- instll done"
}

install_linux(){
    echo "-- install linux desp..."
    sudo apt install libglfw3-dev libglfw3 libglew-dev libglm-dev libpng-dev libopenal-dev libluajit-5.1-dev -y
    echo "-- instll done"
    install_lua
}

install_linux