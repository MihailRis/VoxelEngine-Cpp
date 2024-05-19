#ifndef PHYSICS_PHYSICSSOLVER_HPP_
#define PHYSICS_PHYSICSSOLVER_HPP_

#include "../typedefs.hpp"

#include <glm/glm.hpp>

class Block;
class Hitbox;
class Chunks;

class PhysicsSolver {
    glm::vec3 gravity;
public:
    PhysicsSolver(glm::vec3 gravity);
    void step(
        Chunks* chunks,
        Hitbox* hitbox,
        float delta,
        uint substeps,
        bool shifting,
        float gravityScale,
        bool collisions
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
    bool isBlockInside(int x, int y, int z, Block* def, blockstate_t states, Hitbox* hitbox);
};

#endif // PHYSICS_PHYSICSSOLVER_HPP_
