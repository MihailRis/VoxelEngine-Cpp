//
// Created by chelovek on 11/24/23.
//

#ifndef VULKANDEFENITIONS_H
#define VULKANDEFENITIONS_H

#ifndef NDEBUG
    constexpr bool IS_DEBUG = true;
    #define CHECK_VK(fn) fn
#else
    constexpr bool IS_DEBUG = false;
    #define CHECK_VK(fn) fn
#endif

#endif //VULKANDEFENITIONS_H
