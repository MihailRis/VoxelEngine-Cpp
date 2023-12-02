//
// Created by chelovek on 11/28/23.
//

#ifndef SKYBOX_H
#define SKYBOX_H
#include "Mesh.h"
#include "texture/ImageCube.h"

class IShader;

namespace vulkan {

    class Skybox {
        Mesh<VertexBackSkyGen> *m_mesh;
        IShader *m_shader;
        ImageCube m_cubemap;
        bool m_ready = false;
    public:
        Skybox(uint size, IShader* shader);
        ~Skybox();

        void draw(IShader* shader);

        void refresh(float t, float mie, uint quality);
        void bind();
        void unbind() const;
        bool isReady() const;
    };

} // vulkan

#endif //SKYBOX_H
