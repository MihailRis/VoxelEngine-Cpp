# Run in windows with compiled executable file

[EXE for windows](https://drive.google.com/file/d/1lkFc5nyYOs0Yyu1wmOoAAwEp4r9jO1tE/view?usp=sharing)<br>
[MinGW libraries (include & lib) + glew32.dll](https://drive.google.com/file/d/1k1Hnbz2Uhr4-03upt2yHxKws396HQDra/view?usp=sharing)

# Run in linux with
`$ git clone https://github.com/MihailRis/VoxelEngine-Cpp.git`

`$ cd VoxelEngine-Cpp/Debug`

### Debian-based distro:

	$ sudo apt install libglfw3-dev libglfw3 libglew-dev libglm-dev libpng-dev

### RHEL-based distro:

	$ sudo dnf install glfw-devel glfw glew-devel glm-devel libpng-devel

`$ make`

`$ cd ..`

`$ ./Debug/voxel_engine`

# Note for MinGW compiling

To fix problem with `#include <mingw.thread.h>` get headers `mingw.thread.h` and `mingw.invoke.h` from: 
https://github.com/meganz/mingw-std-threads
