//
// Created by chelovek on 11/28/23.
//

#include "ImageCube.h"

ImageCube::ImageCube(int width, int height, VkFormat format)
    : Image({static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        format,
        VK_IMAGE_VIEW_TYPE_CUBE,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
}
