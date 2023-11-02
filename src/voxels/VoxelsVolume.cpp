#include "VoxelsVolume.h"

VoxelsVolume::VoxelsVolume(int x, int y, int z, int w, int h, int d)
						 : x(x), y(y), z(z), w(w), h(h), d(d) {
	voxels = new voxel[w * h * d];
	for (int i = 0; i < w * h * d; i++) {
		voxels[i].id = BLOCK_VOID;
	}
	lights = new light_t[w * h * d];
}

VoxelsVolume::VoxelsVolume(int w, int h, int d)
	: x(0), y(0), z(0), w(w), h(h), d(d) {
	voxels = new voxel[w * h * d];
	for (int i = 0; i < w * h * d; i++) {
		voxels[i].id = BLOCK_VOID;
	}
	lights = new light_t[w * h * d];
}

VoxelsVolume::~VoxelsVolume() {
	delete[] lights;
	delete[] voxels;
}

void VoxelsVolume::setPosition(int x, int y, int z) {
	this->x = x;
	this->y = y;
	this->z = z;
}
