//
// Created by chelovek on 11/16/23.
//

#include "Queue.h"

Queue::Queue(u32 index, VkQueue queue)
    : m_index(index),
      m_queue(queue) { }

void Queue::setIndex(u32 index) {
    m_index = index;
}

void Queue::setQueue(VkQueue queue) {
    m_queue = queue;
}


u32 Queue::getIndex() const {
    return m_index.value();
}

bool Queue::hasIndex() const {
    return m_index.has_value();
}

Queue::operator VkQueue() const {
    return m_queue;
}


