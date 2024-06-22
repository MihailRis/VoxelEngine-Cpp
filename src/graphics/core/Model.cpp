#include "Model.hpp"

#include <algorithm>

using namespace model;

inline constexpr glm::vec3 X(1, 0, 0);
inline constexpr glm::vec3 Y(0, 1, 0);
inline constexpr glm::vec3 Z(0, 0, 1);

void Mesh::addPlane(glm::vec3 pos, glm::vec3 right, glm::vec3 up, glm::vec3 norm) {
    vertices.push_back({pos-right-up, {0,0}, norm});
    vertices.push_back({pos+right-up, {1,0}, norm});
    vertices.push_back({pos+right+up, {1,1}, norm});

    vertices.push_back({pos-right-up, {0,0}, norm});
    vertices.push_back({pos+right+up, {1,1}, norm});
    vertices.push_back({pos-right+up, {0,1}, norm});
}

void Mesh::addBox(glm::vec3 pos, glm::vec3 size) {
    addPlane(pos+Z*size, X*size, Y*size, Z);
    addPlane(pos-Z*size, -X*size, Y*size, -Z);

    addPlane(pos+Y*size, X*size, -Z*size, Y);
    addPlane(pos-Y*size, X*size, Z*size, -Y);

    addPlane(pos+X*size, -Z*size, Y*size, X);
    addPlane(pos-X*size, Z*size, Y*size, -X);
}


void Model::clean() {
    meshes.erase(
        std::remove_if(meshes.begin(), meshes.end(), 
        [](const Mesh& mesh){
            return mesh.vertices.empty();
        }),
        meshes.end()
    );
}
