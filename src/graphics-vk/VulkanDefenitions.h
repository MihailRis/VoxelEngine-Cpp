//
// Created by chelovek on 11/24/23.
//

#ifndef VULKANDEFENITIONS_H
#define VULKANDEFENITIONS_H

#ifndef NDEBUG
    constexpr bool IS_DEBUG = true;
    #include <iostream>
    #define CHECK_VK_FUNCTION(fn) { const VkResult result = fn; \
                            if (result != VK_SUCCESS) { \
                                std::cout << "Error while call " << #fn << std::endl; \
                                std::abort(); \
                            } \
                          }
    #define CHECK_VK_RESULT(res) { if (res != VK_SUCCESS) { \
                                            std::cout << "Error while get next image" << std::endl; \
                                            std::abort(); \
                                        } \
                                      }
#else
    constexpr bool IS_DEBUG = false;
    #define CHECK_VK_FUNCTION(fn) fn
    #define CHECK_VK_NEXT_IMAGE(result)
#endif

#endif //VULKANDEFENITIONS_H
