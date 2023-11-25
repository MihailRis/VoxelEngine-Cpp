//
// Created by chelovek on 11/16/23.
//

#ifndef QUEUE_H
#define QUEUE_H
#include <optional>
#include <vulkan/vulkan_core.h>

#include "../../typedefs.h"


class Queue {
    std::optional<u32> m_index = std::nullopt;
    VkQueue m_queue = VK_NULL_HANDLE;

public:
    Queue() = default;
    Queue(u32 index, VkQueue queue);

    void setIndex(u32 index);
    void setQueue(VkQueue queue);

    u32 getIndex() const;
    bool hasIndex() const;

    operator VkQueue() const;
};



#endif //QUEUE_H
