//
// Created by chelovek on 11/21/23.
//

#ifndef GRAPHICSCONTEXT_H
#define GRAPHICSCONTEXT_H
#include "../graphics/Viewport.h"

namespace vulkan {

    class GraphicsContext {
        const GraphicsContext* m_parent;
        Viewport &m_viewport;
        bool m_depthTest = false;
        bool m_cullFace = false;
    public:
        GraphicsContext(const GraphicsContext *parent, Viewport &viewport);
        ~GraphicsContext();

        const Viewport &getViewport() const;
        GraphicsContext sub() const;

        void depthTest(bool flag);
        void cullFace(bool flag);
    };

} // vulkan

#endif //GRAPHICSCONTEXT_H
