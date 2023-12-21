# Run with compiled executable file:

[Windows (64 bit)](https://drive.google.com/file/d/1hkqCFP6MG9t6V6hjfL4UXdIW9VRsRaAt/view?usp=sharing)

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
cmake ..
cmake --build .
```

## Install libs:

#### Windows:
For vulkan need install [**Vulkan SDK**](https://vulkan.lunarg.com/sdk/home#windows)

#### Debian-based distro:
`$ sudo apt install libglfw3-dev libglfw3 libglew-dev libglm-dev libpng-dev libopenal-dev`

#### RHEL-based distro:
`$ sudo dnf install glfw-devel glfw glew-devel glm-devel libpng-devel openal-devel`

#### Arch-based distro:
If you use X11
`$ sudo pacman -S glfw-x11 glew glm libpng openal`

If you use Wayland
`$ sudo pacman -S glfw-wayland glew glm libpng openal`

For vulkan need install **vulkan-devel**:

`$ sudo pacman -S vulkan-devel`

#### macOS:

`$ brew install glfw3 glew glm libpng`

Download, compile and install OpenAL
