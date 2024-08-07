#ifndef PHYSICS_PHYSICSSOLVER_HPP_
#define PHYSICS_PHYSICSSOLVER_HPP_

#include "Hitbox.hpp"

#include <typedefs.hpp>
#include <voxels/voxel.hpp>

#include <vector>
#include <glm/glm.hpp>

class Block;
class Chunks;
struct Sensor;

class PhysicsSolver {
    glm::vec3 gravity;
    std::vector<Sensor*> sensors;
public:
    PhysicsSolver(glm::vec3 gravity);
    void step(
        Chunks* chunks,
        Hitbox* hitbox,
        float delta,
        uint substeps,
        entityid_t entity
    );
    void colisionCalc(
        Chunks* chunks,
        Hitbox* hitbox,
        glm::vec3& vel,
        glm::vec3& pos,
        const glm::vec3 half,
        float stepHeight
    );
    bool isBlockInside(int x, int y, int z, Hitbox* hitbox);
    bool isBlockInside(int x, int y, int z, Block* def, blockstate state, Hitbox* hitbox);

    void setSensors(std::vector<Sensor*> sensors) {
        this->sensors = std::move(sensors);
    }

    void removeSensor(Sensor* sensor);
};

#endif // PHYSICS_PHYSICSSOLVER_HPP_
