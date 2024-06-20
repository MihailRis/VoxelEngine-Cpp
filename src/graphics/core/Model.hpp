#ifndef GRAPHICS_CORE_MODEL_HPP_
#define GRAPHICS_CORE_MODEL_HPP_

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace model {
    struct Vertex {
        glm::vec3 coord;
        glm::vec2 uv;
        glm::vec3 normal;
    };

    struct Mesh {
        std::string texture;
        std::vector<Vertex> vertices;
    
        void addPlane(glm::vec3 pos, glm::vec3 right, glm::vec3 up, glm::vec3 norm);
        void addBox(glm::vec3 pos, glm::vec3 size);
    };

    struct Model {
        std::vector<Mesh> meshes;

        Mesh& addMesh(const std::string& texture) {
            meshes.push_back({texture, {}});
            return meshes[meshes.size()-1];
        }
    };
}

#endif // GRAPHICS_CORE_MODEL_HPP_
