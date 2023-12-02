//
// Created by chelovek on 11/23/23.
//

#ifndef UNIFORMBUFFER_H
#define UNIFORMBUFFER_H

#include <cstring>

#include "Buffer.h"

namespace vulkan {

    class UniformBuffer : public Buffer {
    public:
        UniformBuffer(size_t size);

        VkDescriptorBufferInfo getBufferInfo() const;

        template<typename T>
        void uploadData(T data) const {
            void *mapedData = nullptr;

            mapMemory(&mapedData);

            memcpy(mapedData, &data, sizeof(T));

            unmapMemory();
        }
    };

} // vulkan

#endif //UNIFORMBUFFER_H
