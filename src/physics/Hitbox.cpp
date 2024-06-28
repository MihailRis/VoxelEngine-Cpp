#include "Hitbox.hpp"

Hitbox::Hitbox(glm::vec3 position, glm::vec3 halfsize) 
  : position(position), 
    halfsize(halfsize), 
    velocity(0.0f,0.0f,0.0f), 
    linearDamping(0.1f) 
{}
