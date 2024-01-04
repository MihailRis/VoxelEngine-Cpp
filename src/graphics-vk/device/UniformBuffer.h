//
// Created by chelovek on 11/23/23.
//

#ifndef UNIFORMBUFFER_H
#define UNIFORMBUFFER_H

#include <cstring>

#include "Buffer.h"
#include "../Inistializers.h"

namespace vulkan {

    class UniformBuffer : public Buffer {
        size_t m_baseSize = 0;
        bool m_isDynamic = false;

    public:
        UniformBuffer(size_t size);
        UniformBuffer(size_t size, size_t count);

        initializers::UniformBufferInfo getBufferInfo() const;

        template<typename T>
        void uploadData(T data) {
            void *mapedData = nullptr;

            mapMemory(&mapedData);

            std::memcpy(mapedData, &data, m_size);

            unmapMemory();
        }

        template<typename T>
        void uploadDataDynamic(T data, size_t offset) {
            char *mapedData = nullptr;

            mapMemory(reinterpret_cast<void**>(&mapedData));

            std::memcpy(mapedData + offset, &data, sizeof(T));

            unmapMemory();
        }
    };

} // vulkan

#endif //UNIFORMBUFFER_H
