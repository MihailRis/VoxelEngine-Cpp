//
// Created by chelovek on 11/21/23.
//

#include "GraphicsContext.h"

namespace vulkan {
    GraphicsContext::GraphicsContext(const GraphicsContext* parent, Viewport& viewport)
        : m_parent(parent), m_viewport(viewport) {
    }

    GraphicsContext::~GraphicsContext() {
        if (m_parent == nullptr)
            return;
        if (m_depthTest != m_parent->m_depthTest) {
            if (m_depthTest) {

            }
            else {

            }
        }
        if (m_cullFace != m_parent->m_cullFace) {
            if (m_cullFace) {

            }
            else {

            }
        }
    }
} // vulkan