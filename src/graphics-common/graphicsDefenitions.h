//
// Created by chelovek on 1/5/24.
//

#ifndef UTILS_H
#define UTILS_H

#ifdef USE_VULKAN
#include "../graphics-vk/Batch2D.h"
#include "../graphics-vk/Batch3D.h"
#include "../graphics-vk/LineBatch.h"

using vulkan::Batch2D;
using vulkan::Batch3D;
using vulkan::LineBatch;

#else
#include "../graphics/Batch2D.h"
#include "../graphics/Batch3D.h"
#include "../graphics/LineBatch.h"
#endif

#endif //UTILS_H
