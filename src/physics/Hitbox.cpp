#include "Hitbox.h"

Hitbox::Hitbox(vec3 position, vec3 halfsize) : position(position), halfsize(halfsize), velocity(0.0f,0.0f,0.0f), linear_damping(0.1f) {
}
