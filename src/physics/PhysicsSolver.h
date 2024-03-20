#ifndef PHYSICS_PHYSICSSOLVER_H_
#define PHYSICS_PHYSICSSOLVER_H_

#include <glm/glm.hpp>
#include "../typedefs.h"
#include "../voxels/Block.h"

class Hitbox;
class ChunksStorage;

class PhysicsSolver {
    glm::vec3 gravity;
public:
    PhysicsSolver(glm::vec3 gravity);
    void step(ChunksStorage* chunks,
            Hitbox* hitbox,
            float delta,
            uint substeps,
            bool shifting,
            float gravityScale,
            bool collisions);
    void colisionCalc(
            ChunksStorage* chunks, 
            Hitbox* hitbox, 
            glm::vec3& vel, 
            glm::vec3& pos, 
            const glm::vec3 half,
            float stepHeight);
    bool isBlockInside(int x, int y, int z, Hitbox* hitbox);
    bool isBlockInside(int x, int y, int z, Block* def, blockstate_t states, Hitbox* hitbox);
};

#endif /* PHYSICS_PHYSICSSOLVER_H_ */
