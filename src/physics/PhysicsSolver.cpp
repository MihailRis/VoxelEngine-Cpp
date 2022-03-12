#include "PhysicsSolver.h"
#include "Hitbox.h"
#include "../voxels/Chunks.h"

#include <iostream>

#define E 0.03
#define DEFAULT_FRICTION 10.0

PhysicsSolver::PhysicsSolver(vec3 gravity) : gravity(gravity) {
}

void PhysicsSolver::step(Chunks* chunks, Hitbox* hitbox, float delta, unsigned substeps, bool shifting, float gravityScale) {
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

		if (vel.x < 0.0){
			for (int y = floor(pos.y-half.y+E); y <= floor(pos.y+half.y-E); y++){
				for (int z = floor(pos.z-half.z+E); z <= floor(pos.z+half.z-E); z++){
					int x = floor(pos.x-half.x-E);
					if (chunks->isObstacle(x,y,z)){
						vel.x *= 0.0;
						pos.x = x + 1 + half.x + E;
						break;
					}
				}
			}
		}
		if (vel.x > 0.0){
			for (int y = floor(pos.y-half.y+E); y <= floor(pos.y+half.y-E); y++){
				for (int z = floor(pos.z-half.z+E); z <= floor(pos.z+half.z-E); z++){
					int x = floor(pos.x+half.x+E);
					if (chunks->isObstacle(x,y,z)){
						vel.x *= 0.0;
						pos.x = x - half.x - E;
						break;
					}
				}
			}
		}

		if (vel.z < 0.0){
			for (int y = floor(pos.y-half.y+E); y <= floor(pos.y+half.y-E); y++){
				for (int x = floor(pos.x-half.x+E); x <= floor(pos.x+half.x-E); x++){
					int z = floor(pos.z-half.z-E);
					if (chunks->isObstacle(x,y,z)){
						vel.z *= 0.0;
						pos.z = z + 1 + half.z + E;
						break;
					}
				}
			}
		}

		if (vel.z > 0.0){
			for (int y = floor(pos.y-half.y+E); y <= floor(pos.y+half.y-E); y++){
				for (int x = floor(pos.x-half.x+E); x <= floor(pos.x+half.x-E); x++){
					int z = floor(pos.z+half.z+E);
					if (chunks->isObstacle(x,y,z)){
						vel.z *= 0.0;
						pos.z = z - half.z - E;
						break;
					}
				}
			}
		}

		if (vel.y < 0.0){
			for (int x = floor(pos.x-half.x+E); x <= floor(pos.x+half.x-E); x++){
				bool broken = false;
				for (int z = floor(pos.z-half.z+E); z <= floor(pos.z+half.z-E); z++){
					int y = floor(pos.y-half.y-E);
					if (chunks->isObstacle(x,y,z)){
						vel.y *= 0.0;
						pos.y = y + 1 + half.y;
						int f = DEFAULT_FRICTION;
						vel.x *= max(0.0, 1.0 - dt * f);
						vel.z *= max(0.0, 1.0 - dt * f);
						hitbox->grounded = true;
						broken = true;
						break;
					}
				}
				if (broken)
					break;
			}
		}
		if (vel.y > 0.0){
			for (int x = floor(pos.x-half.x+E); x <= floor(pos.x+half.x-E); x++){
				for (int z = floor(pos.z-half.z+E); z <= floor(pos.z+half.z-E); z++){
					int y = floor(pos.y+half.y+E);
					if (chunks->isObstacle(x,y,z)){
						vel.y *= 0.0;
						pos.y = y - half.y - E;
						break;
					}
				}
			}
		}

		vel.x *= max(0.0, 1.0 - dt * linear_damping);
		vel.z *= max(0.0, 1.0 - dt * linear_damping);

		pos.x += vel.x * dt;
		pos.y += vel.y * dt;
		pos.z += vel.z * dt;

		if (shifting && hitbox->grounded){
			int y = floor(pos.y-half.y-E);

			hitbox->grounded = false;
			for (int x = floor(px-half.x+E); x <= floor(px+half.x-E); x++){
				for (int z = floor(pos.z-half.z+E); z <= floor(pos.z+half.z-E); z++){
					if (chunks->isObstacle(x,y,z)){
						hitbox->grounded = true;
						break;
					}
				}
			}
			if (!hitbox->grounded)
				pos.z = pz;
			hitbox->grounded = false;

			for (int x = floor(pos.x-half.x+E); x <= floor(pos.x+half.x-E); x++){
				for (int z = floor(pz-half.z+E); z <= floor(pz+half.z-E); z++){
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

bool PhysicsSolver::isBlockInside(int x, int y, int z, Hitbox* hitbox) {
	vec3& pos = hitbox->position;
	vec3& half = hitbox->halfsize;
	return x >= floor(pos.x-half.x) && x <= floor(pos.x+half.x) &&
			z >= floor(pos.z-half.z) && z <= floor(pos.z+half.z) &&
			y >= floor(pos.y-half.y) && y <= floor(pos.y+half.y);
}

