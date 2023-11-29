#include "PhysicsSolver.h"
#include "Hitbox.h"

#include "../maths/aabb.h"
#include "../voxels/Block.h"
#include "../voxels/Chunks.h"

#define E 0.03

PhysicsSolver::PhysicsSolver(vec3 gravity) : gravity(gravity) {
}

void PhysicsSolver::step(Chunks* chunks, Hitbox* hitbox, float delta, unsigned substeps, bool shifting,
		float gravityScale,
		bool collisions) {
	hitbox->grounded = false;
	for (unsigned i = 0; i < substeps; i++){
		float dt = delta / (float)substeps;
		float linear_damping = hitbox->linear_damping;
		vec3& pos = hitbox->position;
		vec3& half = hitbox->halfsize;
		vec3& vel = hitbox->velocity;
		vel.x += gravity.x*dt * gravityScale;
		vel.y += gravity.y*dt * gravityScale;
		vel.z += gravity.z*dt * gravityScale;

		float px = pos.x;
		float pz = pos.z;

		if (collisions) {
			colisionCalc(chunks, hitbox, &vel, &pos, half);
		}

		vel.x *= max(0.0, 1.0 - dt * linear_damping);
		vel.z *= max(0.0, 1.0 - dt * linear_damping);

		pos.x += vel.x * dt;
		pos.y += vel.y * dt;
		pos.z += vel.z * dt;

		if (shifting && hitbox->grounded){
			float y = (pos.y-half.y-E);

			hitbox->grounded = false;
			for (float x = (px-half.x+E); x <= (px+half.x-E); x++){
				for (float z = (pos.z-half.z+E); z <= (pos.z+half.z-E); z++){
					if (chunks->isObstacle(x,y,z)){
						hitbox->grounded = true;
						break;
					}
				}
			}
			if (!hitbox->grounded)
				pos.z = pz;
			hitbox->grounded = false;

			for (float x = (pos.x-half.x+E); x <= (pos.x+half.x-E); x++){
				for (float z = (pz-half.z+E); z <= (pz+half.z-E); z++){
					if (chunks->isObstacle(x,y,z)){
						hitbox->grounded = true;
						break;
					}
				}
			}
			if (!hitbox->grounded)
				pos.x = px;

			hitbox->grounded = true;
		}
	}
}

void PhysicsSolver::colisionCalc(Chunks* chunks, Hitbox* hitbox, vec3* vel, vec3* pos, vec3 half){
	// step size (smaller - more accurate, but slower)
	float s = 2.0/BLOCK_AABB_GRID;

	const AABB* aabb;
	
	if (vel->x < 0.0){
		for (float y = (pos->y-half.y+E); y <= (pos->y+half.y-E); y+=s){
			for (float z = (pos->z-half.z+E); z <= (pos->z+half.z-E); z+=s){
				float x = (pos->x-half.x-E);
				if ((aabb = chunks->isObstacle(x,y,z))){
					vel->x *= 0.0;
					pos->x = floor(x) + aabb->max().x + half.x + E;
					break;
				}
			}
		}
	}
	if (vel->x > 0.0){
		for (float y = (pos->y-half.y+E); y <= (pos->y+half.y-E); y+=s){
			for (float z = (pos->z-half.z+E); z <= (pos->z+half.z-E); z+=s){
				float x = (pos->x+half.x+E);
				if ((aabb = chunks->isObstacle(x,y,z))){
					vel->x *= 0.0;
					pos->x = floor(x) - half.x + aabb->min().x - E;
					break;
				}
			}
		}
	}

	if (vel->z < 0.0){
		for (float y = (pos->y-half.y+E); y <= (pos->y+half.y-E); y+=s){
			for (float x = (pos->x-half.x+E); x <= (pos->x+half.x-E); x+=s){
				float z = (pos->z-half.z-E);
				if ((aabb = chunks->isObstacle(x,y,z))){
					vel->z *= 0.0;
					pos->z = floor(z) + aabb->max().z + half.z + E;
					break;
				}
			}
		}
	}

	if (vel->z > 0.0){
		for (float y = (pos->y-half.y+E); y <= (pos->y+half.y-E); y+=s){
			for (float x = (pos->x-half.x+E); x <= (pos->x+half.x-E); x+=s){
				float z = (pos->z+half.z+E);
				if ((aabb = chunks->isObstacle(x,y,z))){
					vel->z *= 0.0;
					pos->z = floor(z) - half.z + aabb->min().z - E;
					break;
				}
			}
		}
	}

	if (vel->y < 0.0){
		for (float x = (pos->x-half.x+E); x <= (pos->x+half.x-E); x+=s){
			for (float z = (pos->z-half.z+E); z <= (pos->z+half.z-E); z+=s){
				float y = (pos->y-half.y-E);
				if ((aabb = chunks->isObstacle(x,y,z))){
					vel->y *= 0.0;
					pos->y = floor(y) + aabb->max().y + half.y;
					hitbox->grounded = true;
					break;
				}
			}
		}
	}
	if (vel->y > 0.0){
		for (float x = (pos->x-half.x+E); x <= (pos->x+half.x-E); x+=s){
			for (float z = (pos->z-half.z+E); z <= (pos->z+half.z-E); z+=s){
				float y = (pos->y+half.y+E);
				if ((aabb = chunks->isObstacle(x,y,z))){
					vel->y *= 0.0;
					pos->y = floor(y) - half.y + aabb->min().y - E;
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

