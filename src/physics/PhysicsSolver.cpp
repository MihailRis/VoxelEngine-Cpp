#include "PhysicsSolver.h"
#include "Hitbox.h"

#include "../maths/aabb.h"
#include "../voxels/Block.h"
#include "../voxels/Chunks.h"

const double E = 0.03;
const double MAX_FIX = 0.1;

using glm::vec3;

PhysicsSolver::PhysicsSolver(vec3 gravity) : gravity(gravity) {
}

void PhysicsSolver::step(
		Chunks* chunks, 
		Hitbox* hitbox, 
		float delta, 
		uint substeps, 
		bool shifting,
		float gravityScale,
		bool collisions)
{
	float dt = delta / float(substeps);
	float linear_damping = hitbox->linear_damping;
	float s = 2.0f/BLOCK_AABB_GRID;
    
    bool prevGrounded = hitbox->grounded;
	hitbox->grounded = false;
	for (uint i = 0; i < substeps; i++) {
		vec3& pos = hitbox->position;
		vec3& half = hitbox->halfsize;
		vec3& vel = hitbox->velocity;
		float px = pos.x;
		float pz = pos.z;
		
		vel += gravity * dt * gravityScale;
		if (collisions) {
			colisionCalc(chunks, hitbox, vel, pos, half, 
                         (prevGrounded && gravityScale > 0.0f) ? 0.5f : 0.0f);
		}
		vel.x *= glm::max(0.0f, 1.0f - dt * linear_damping);
		vel.z *= glm::max(0.0f, 1.0f - dt * linear_damping);
		pos += vel * dt;

		if (shifting && hitbox->grounded){
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
}

void PhysicsSolver::colisionCalc(
		Chunks* chunks, 
		Hitbox* hitbox, 
		vec3& vel, 
		vec3& pos, 
		const vec3 half,
        float stepHeight)
{
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
					vel.x *= 0.0f;
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
					vel.x *= 0.0f;
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
					vel.z *= 0.0f;
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
					vel.z *= 0.0f;
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
					vel.y *= 0.0f;
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
					vel.y *= 0.0f;
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
					vel.y *= 0.0f;
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
	vec3& pos = hitbox->position;
	vec3& half = hitbox->halfsize;
	return x >= floor(pos.x-half.x) && x <= floor(pos.x+half.x) &&
			z >= floor(pos.z-half.z) && z <= floor(pos.z+half.z) &&
			y >= floor(pos.y-half.y) && y <= floor(pos.y+half.y);
}

