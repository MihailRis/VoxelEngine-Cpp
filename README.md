# Run in windows with compiled executable file:

[EXE for windows (old version)](https://drive.google.com/file/d/1lkFc5nyYOs0Yyu1wmOoAAwEp4r9jO1tE/view?usp=sharing)<br>

# Controls:
- <kbd>Tab</kbd> - toggle camera control
- <kbd>W</kbd> <kbd>A</kbd> <kbd>S</kbd> <kbd>D</kbd> - movement
- <kbd>Space</kbd> - jump
- <kbd>LMB</kbd> - remove block 
- <kbd>RMB</kbd> - place block
- <kbd>F</kbd> - toggle flight mode
- <kbd>N</kbd> - noclip mode
- <kbd>Esc</kbd> - exit

#### Build with CMake
```sh
git clone --recursive https://github.com/MihailRis/VoxelEngine-Cpp.git
cd VoxelEngine-Cpp
mkdir build
cd build
cmake ../
cmake --build .
```

## Instal libs:
#### Debian-based distro:
`$ sudo apt install libglfw3-dev libglfw3 libglew-dev libglm-dev libpng-dev libopenal-dev`

#### RHEL-based distro:
`$ sudo dnf install glfw-devel glfw glew-devel glm-devel libpng-devel openal-devel`

#### Arch-based distro:
If you use X11
`$ sudo pacman -S glfw-x11 glew glm libpng openal`

If you use Wayland
`$ sudo pacman -S glfw-wayland glew glm libpng openal`
