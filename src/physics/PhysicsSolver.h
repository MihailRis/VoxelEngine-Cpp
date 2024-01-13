#ifndef PHYSICS_PHYSICSSOLVER_H_
#define PHYSICS_PHYSICSSOLVER_H_

#include <glm/glm.hpp>
#include "../typedefs.h"

class Hitbox;
class Chunks;

class PhysicsSolver {
    glm::vec3 gravity;
public:
    PhysicsSolver(glm::vec3 gravity);
    void step(Chunks* chunks,
            Hitbox* hitbox,
            float delta,
            uint substeps,
            bool shifting,
            float gravityScale,
            bool collisions);
    void colisionCalc(
            Chunks* chunks, 
            Hitbox* hitbox, 
            glm::vec3& vel, 
            glm::vec3& pos, 
            const glm::vec3 half,
            float stepHeight);
    bool isBlockInside(int x, int y, int z, Hitbox* hitbox);
};

#endif /* PHYSICS_PHYSICSSOLVER_H_ */
