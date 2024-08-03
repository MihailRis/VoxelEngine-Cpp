#include "VoxelsVolume.hpp"

VoxelsVolume::VoxelsVolume(int x, int y, int z, int w, int h, int d)
    : x(x),
      y(y),
      z(z),
      w(w),
      h(h),
      d(d),
      voxels(std::make_unique<voxel[]>(w * h * d)),
      lights(std::make_unique<light_t[]>(w * h * d)) {
    for (int i = 0; i < w * h * d; i++) {
        voxels[i].id = BLOCK_VOID;
    }
}

VoxelsVolume::VoxelsVolume(int w, int h, int d)
    : VoxelsVolume(0, 0, 0, w, h, d) {
}

VoxelsVolume::~VoxelsVolume() {
}

void VoxelsVolume::setPosition(int x, int y, int z) {
    this->x = x;
    this->y = y;
    this->z = z;
}
