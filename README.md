# Run with compiled executable file:

[Windows (64 bit)](https://drive.google.com/file/d/15v_PwtbPRQ0ytDok-1ucHYks2T2cZdnt/view?usp=sharing)

[Linux AppImage (x86_64)](https://github.com/MihailRis/VoxelEngine-Cpp/releases/download/v16/VoxelEngine-0.16-x86_64.AppImage)

# Controls:
- <kbd>**Esc**</kbd> - pause
- <kbd>**Tab**</kbd> - open inventory
- <kbd>**W**</kbd> <kbd>**A**</kbd> <kbd>**S**</kbd> <kbd>**D**</kbd> - movement
- <kbd>**Space**</kbd> - jump
- <kbd>**LMB**</kbd> - remove block 
- <kbd>**RMB**</kbd> - place block
- <kbd>**F**</kbd> - toggle flight mode
- <kbd>**N**</kbd> - noclip mode
- <kbd>**F1**</kbd> - toggle interface
- <kbd>**F2**</kbd> - save screenshot
- <kbd>**F3**</kbd> - debug mode
- <kbd>**F5**</kbd> - reload chunks

#### Build with CMake
```sh
git clone --recursive https://github.com/MihailRis/VoxelEngine-Cpp.git
cd VoxelEngine-Cpp
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Install libs:

#### Debian-based distro:
```sh
sudo apt install libglfw3-dev libglfw3 libglew-dev libglm-dev libpng-dev libopenal-dev libluajit-5.1-dev
```

CMake missing LUA_INCLUDE_DIR and LUA_LIBRARIES fix:
```sh
sudo ln -s /usr/lib/x86_64-linux-gnu/libluajit-5.1.a /usr/lib/x86_64-linux-gnu/liblua5.1.a
sudo ln -s /usr/include/luajit-2.1 /usr/include/lua
```

#### RHEL-based distro:
```sh
sudo dnf install glfw-devel glfw glew-devel glm-devel libpng-devel openal-devel
```

\+ install LuaJIT

#### Arch-based distro:
If you use X11
```sh
sudo pacman -S glfw-x11 glew glm libpng openal
```

If you use Wayland
```sh
sudo pacman -S glfw-wayland glew glm libpng openal
```

\+ install LuaJIT

#### LuaJIT installation:
```sh
git clone https://luajit.org/git/luajit.git
cd luajit
make && sudo make install INSTALL_INC=/usr/include/lua
```

#### macOS:

```
brew install glfw3 glew glm libpng
```

Download, compile and install OpenAL and LuaJIT
