#include "PhysicsSolver.hpp"
#include "Hitbox.hpp"

#include "../maths/aabb.hpp"
#include <voxels/Block.hpp>
#include <voxels/Chunks.hpp>
#include <voxels/voxel.hpp>

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

const float E = 0.03f;
const float MAX_FIX = 0.1f;

PhysicsSolver::PhysicsSolver(glm::vec3 gravity) : gravity(gravity) {
}

void PhysicsSolver::step(
    Chunks* chunks, 
    Hitbox* hitbox, 
    float delta, 
    uint substeps, 
    entityid_t entity
) {
    float dt = delta / static_cast<float>(substeps);
    float linearDamping = hitbox->linearDamping;
    float s = 2.0f/BLOCK_AABB_GRID;

    const glm::vec3& half = hitbox->halfsize;
    glm::vec3& pos = hitbox->position;
    glm::vec3& vel = hitbox->velocity;
    float gravityScale = hitbox->gravityScale;
    
    bool prevGrounded = hitbox->grounded;
    hitbox->grounded = false;
    for (uint i = 0; i < substeps; i++) {
        float px = pos.x;
        float py = pos.y;
        float pz = pos.z;
        
        vel += gravity * dt * gravityScale;
        if (hitbox->type == BodyType::DYNAMIC) {
            colisionCalc(chunks, hitbox, vel, pos, half, 
                         (prevGrounded && gravityScale > 0.0f) ? 0.5f : 0.0f);
        }
        vel.x *= glm::max(0.0f, 1.0f - dt * linearDamping);
        if (hitbox->verticalDamping) {
            vel.y *= glm::max(0.0f, 1.0f - dt * linearDamping);
        }
        vel.z *= glm::max(0.0f, 1.0f - dt * linearDamping);

        pos += vel * dt + gravity * gravityScale * dt * dt * 0.5f;
        if (hitbox->grounded && pos.y < py) {
            pos.y = py;
        }

        if (hitbox->crouching && hitbox->grounded){
            float y = (pos.y-half.y-E);
            hitbox->grounded = false;
            for (float x = (px-half.x+E); x <= (px+half.x-E); x+=s){
                for (float z = (pos.z-half.z+E); z <= (pos.z+half.z-E); z+=s){
                    if (chunks->isObstacleAt(x,y,z)){
                        hitbox->grounded = true;
                        break;
                    }
                }
            }
            if (!hitbox->grounded) {
                pos.z = pz;
            }
            hitbox->grounded = false;
            for (float x = (pos.x-half.x+E); x <= (pos.x+half.x-E); x+=s){
                for (float z = (pz-half.z+E); z <= (pz+half.z-E); z+=s){
                    if (chunks->isObstacleAt(x,y,z)){
                        hitbox->grounded = true;
                        break;
                    }
                }
            }
            if (!hitbox->grounded) {
                pos.x = px;
            }
            hitbox->grounded = true;
        }
    }
    AABB aabb;
    aabb.a = hitbox->position - hitbox->halfsize;
    aabb.b = hitbox->position + hitbox->halfsize;
    for (size_t i = 0; i < sensors.size(); i++) {
        auto& sensor = *sensors[i];
        if (sensor.entity == entity) {
            continue;
        }

        bool triggered = false;
        switch (sensor.type) {
            case SensorType::AABB:
                triggered = aabb.intersect(sensor.calculated.aabb);
                break;
            case SensorType::RADIUS:
                triggered = glm::distance2(
                    hitbox->position, glm::vec3(sensor.calculated.radial))
                     < sensor.calculated.radial.w;
                break;
        }
        if (triggered) {
            if (sensor.prevEntered.find(entity) == sensor.prevEntered.end()) {
                sensor.enterCallback(sensor.entity, sensor.index, entity);
            }
            sensor.nextEntered.insert(entity);
        }
    }
}

void PhysicsSolver::colisionCalc(
    Chunks* chunks, 
    Hitbox* hitbox, 
    glm::vec3& vel, 
    glm::vec3& pos, 
    const glm::vec3 half,
    float stepHeight
) {
    // step size (smaller - more accurate, but slower)
    float s = 2.0f/BLOCK_AABB_GRID;

    if (stepHeight > 0.0f) {
        for (float x = (pos.x-half.x+E); x <= (pos.x+half.x-E); x+=s){
            for (float z = (pos.z-half.z+E); z <= (pos.z+half.z-E); z+=s){
                if (chunks->isObstacleAt(x, pos.y+half.y+stepHeight, z)) {
                    stepHeight = 0.0f;
                    break;
                }
            }
        }
    }

    const AABB* aabb;
    
    if (vel.x < 0.0f){
        for (float y = (pos.y-half.y+E+stepHeight); y <= (pos.y+half.y-E); y+=s){
            for (float z = (pos.z-half.z+E); z <= (pos.z+half.z-E); z+=s){
                float x = (pos.x-half.x-E);
                if ((aabb = chunks->isObstacleAt(x,y,z))){
                    vel.x = 0.0f;
                    float newx = floor(x) + aabb->max().x + half.x + E;
                    if (glm::abs(newx-pos.x) <= MAX_FIX) {
                        pos.x = newx;
                    }
                    break;
                }
            }
        }
    }
    if (vel.x > 0.0f){
        for (float y = (pos.y-half.y+E+stepHeight); y <= (pos.y+half.y-E); y+=s){
            for (float z = (pos.z-half.z+E); z <= (pos.z+half.z-E); z+=s){
                float x = (pos.x+half.x+E);
                if ((aabb = chunks->isObstacleAt(x,y,z))){
                    vel.x = 0.0f;
                    float newx = floor(x) - half.x + aabb->min().x - E;
                    if (glm::abs(newx-pos.x) <= MAX_FIX) {
                        pos.x = newx;
                    }
                    break;
                }
            }
        }
    }

    if (vel.z < 0.0f){
        for (float y = (pos.y-half.y+E+stepHeight); y <= (pos.y+half.y-E); y+=s){
            for (float x = (pos.x-half.x+E); x <= (pos.x+half.x-E); x+=s){
                float z = (pos.z-half.z-E);
                if ((aabb = chunks->isObstacleAt(x,y,z))){
                    vel.z = 0.0f;
                    float newz = floor(z) + aabb->max().z + half.z + E;
                    if (glm::abs(newz-pos.z) <= MAX_FIX) { 
                        pos.z = newz;
                    }
                    break;
                }
            }
        }
    }

    if (vel.z > 0.0f){
        for (float y = (pos.y-half.y+E+stepHeight); y <= (pos.y+half.y-E); y+=s){
            for (float x = (pos.x-half.x+E); x <= (pos.x+half.x-E); x+=s){
                float z = (pos.z+half.z+E);
                if ((aabb = chunks->isObstacleAt(x,y,z))){
                    vel.z = 0.0f;
                    float newz = floor(z) - half.z + aabb->min().z - E;
                    if (glm::abs(newz-pos.z) <= MAX_FIX) {
                        pos.z = newz;
                    }
                    break;
                }
            }
        }
    }

    if (vel.y < 0.0f){
        for (float x = (pos.x-half.x+E); x <= (pos.x+half.x-E); x+=s){
            for (float z = (pos.z-half.z+E); z <= (pos.z+half.z-E); z+=s){
                float y = (pos.y-half.y-E);
                if ((aabb = chunks->isObstacleAt(x,y,z))){
                    vel.y = 0.0f;
                    float newy = floor(y) + aabb->max().y + half.y;
                    if (glm::abs(newy-pos.y) <= MAX_FIX) {
                        pos.y = newy;	
                    }
                    hitbox->grounded = true;
                    break;
                }
            }
        }
    }
    if (stepHeight > 0.0 && vel.y <= 0.0f){
        for (float x = (pos.x-half.x+E); x <= (pos.x+half.x-E); x+=s){
            for (float z = (pos.z-half.z+E); z <= (pos.z+half.z-E); z+=s){
                float y = (pos.y-half.y+E);
                if ((aabb = chunks->isObstacleAt(x,y,z))){
                    vel.y = 0.0f;
                    float newy = floor(y) + aabb->max().y + half.y;
                    if (glm::abs(newy-pos.y) <= MAX_FIX+stepHeight) {
                        pos.y = newy;	
                    }
                    break;
                }
            }
        }
    }
    if (vel.y > 0.0f){
        for (float x = (pos.x-half.x+E); x <= (pos.x+half.x-E); x+=s){
            for (float z = (pos.z-half.z+E); z <= (pos.z+half.z-E); z+=s){
                float y = (pos.y+half.y+E);
                if ((aabb = chunks->isObstacleAt(x,y,z))){
                    vel.y = 0.0f;
                    float newy = floor(y) - half.y + aabb->min().y - E;
                    if (glm::abs(newy-pos.y) <= MAX_FIX) {
                        pos.y = newy;
                    }
                    break;
                }
            }
        }
    }
}

bool PhysicsSolver::isBlockInside(int x, int y, int z, Hitbox* hitbox) {
    const glm::vec3& pos = hitbox->position;
    const glm::vec3& half = hitbox->halfsize;
    return x >= floor(pos.x-half.x) && x <= floor(pos.x+half.x) &&
           z >= floor(pos.z-half.z) && z <= floor(pos.z+half.z) &&
           y >= floor(pos.y-half.y) && y <= floor(pos.y+half.y);
}

bool PhysicsSolver::isBlockInside(int x, int y, int z, Block* def, blockstate state, Hitbox* hitbox) {
    const float E = 0.001f; // inaccuracy
    const glm::vec3& pos = hitbox->position;
    const glm::vec3& half = hitbox->halfsize;
    const auto& boxes = def->rotatable 
                      ? def->rt.hitboxes[state.rotation] 
                      : def->hitboxes;
    for (const auto& block_hitbox : boxes) {
        glm::vec3 min = block_hitbox.min();
        glm::vec3 max = block_hitbox.max();
        if (min.x < pos.x+half.x-x-E && max.x > pos.x-half.x-x+E &&
            min.z < pos.z+half.z-z-E && max.z > pos.z-half.z-z+E &&
            min.y < pos.y+half.y-y-E && max.y > pos.y-half.y-y+E)
            return true;
    }
    return false;
}

void PhysicsSolver::removeSensor(Sensor* sensor) {
    sensors.erase(std::remove(sensors.begin(), sensors.end(), sensor), sensors.end());
}
