//
// Created by chelovek on 11/24/23.
//

#ifndef VULKANDEFENITIONS_H
#define VULKANDEFENITIONS_H

#ifndef NDEBUG
    constexpr bool IS_DEBUG = true;
    #include <iostream>
    #define CHECK_VK(fn) { VkResult result = fn; \
                            if (result != VK_SUCCESS) { \
                                std::cout << "Error while call " << #fn << std::endl; \
                                std::abort(); \
                            } \
                          }
#else
    constexpr bool IS_DEBUG = false;
    #define CHECK_VK(fn) fn
#endif

#endif //VULKANDEFENITIONS_H
