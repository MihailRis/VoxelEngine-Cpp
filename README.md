# ![voxel-core](dev/VoxelCore.png) VoxelCore

## Latest release

- [Download](https://github.com/MihailRis/VoxelEngine-Cpp/releases/latest) | [Скачать](https://github.com/MihailRis/VoxelEngine-Cpp/releases/latest)
- [Documentation](https://github.com/MihailRis/VoxelEngine-Cpp/blob/release-0.25/doc/en/main-page.md) | [Документация](https://github.com/MihailRis/VoxelEngine-Cpp/blob/release-0.25/doc/ru/main-page.md)

## Build project in Linux

### Install libraries

#### Install EnTT

```sh
git clone https://github.com/skypjack/entt.git
cd entt/build
cmake -DCMAKE_BUILD_TYPE=Release ..
sudo make install
```

> [!WARNING]
> If you are using ALT Linux, you should not use this EnTT installation method

#### ALT Linux based distro

```sh
su -
apt-get install entt-devel libglfw3-devel libGLEW-devel libglm-devel libpng-devel libvorbis-devel libopenal-devel libluajit-devel libstdc++13-devel-static libcurl-devel
```

#### Debian based distro

```sh
sudo apt install libglfw3-dev libglfw3 libglew-dev libglm-dev libpng-dev libopenal-dev libluajit-5.1-dev libvorbis-dev libcurl4-openssl-dev
```

> [!TIP]
> CMake missing LUA_INCLUDE_DIR and LUA_LIBRARIES fix:
>
> ```sh
> sudo ln -s /usr/lib/x86_64-linux-gnu/libluajit-5.1.a /usr/lib/x86_64-linux-gnu/liblua5.1.a
> sudo ln -s /usr/include/luajit-2.1 /usr/include/lua
> ```

#### RHEL based distro

```sh
sudo dnf install glfw-devel glfw glew-devel glm-devel libpng-devel libvorbis-devel openal-devel luajit-devel libcurl-devel
```

#### Arch based distro

If you use X11

```sh
sudo pacman -S glfw-x11 glew glm libpng libvorbis openal luajit libcurl
```

If you use Wayland

```sh
sudo pacman -S glfw-wayland glew glm libpng libvorbis openal luajit libcurl
```

### Build engine with CMake

```sh
git clone --recursive https://github.com/MihailRis/VoxelEngine-Cpp.git
cd VoxelEngine-Cpp
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Build project in macOS

### Install libraries

```sh
brew install glfw3 glew glm libpng libvorbis lua luajit libcurl openal-soft skypjack/entt/entt
```

> [!TIP]
> If homebrew for some reason could not install the necessary packages:
> ```lua luajit openal-soft```, then download, install and compile them manually
> (Lua, LuaJIT and OpenAL).

### Build engine with CMake

```sh
git clone --recursive https://github.com/MihailRis/VoxelEngine-Cpp.git
cd VoxelEngine-Cpp
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Build in Windows

>[!NOTE]
> Requirement:
>
> vcpkg, CMake

```sh
git clone --recursive https://github.com/MihailRis/VoxelEngine-Cpp.git
cd VoxelEngine-Cpp
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DVOXELENGINE_BUILD_WINDOWS_VCPKG=ON ..
del CMakeCache.txt
rmdir /s /q CMakeFiles
cmake -DCMAKE_BUILD_TYPE=Release -DVOXELENGINE_BUILD_WINDOWS_VCPKG=ON ..
cmake --build . --config Release
```

> [!TIP]
> You can use ```rm CMakeCache.txt``` and ```rm -rf CMakeFiles``` while using Git Bash

> [!WARNING]
> If you have issues during the vcpkg integration, try navigate to ```vcpkg\downloads```
> and extract PowerShell-[version]-win-x86 to ```vcpkg\downloads\tools``` as powershell-core-[version]-windows.
> Then rerun ```cmake -DCMAKE_BUILD_TYPE=Release -DVOXELENGINE_BUILD_WINDOWS_VCPKG=ON ..```

## Build using Docker

### Step 0. Install docker on your system

See <https://docs.docker.com/engine/install>

### Step 1. Build docker container

```sh
docker build -t voxel-engine .
```

### Step 2. Build project using the docker container

```sh
docker run --rm -it -v$(pwd):/project voxel-engine bash -c "cmake -DCMAKE_BUILD_TYPE=Release -Bbuild && cmake --build build"
```

### Step 3. Run project using the docker container

```sh
docker run --rm -it -v$(pwd):/project -v/tmp/.X11-unix:/tmp/.X11-unix -v${XAUTHORITY}:/home/user/.Xauthority:ro -eDISPLAY --network=host voxel-engine ./build/VoxelEngine
```
