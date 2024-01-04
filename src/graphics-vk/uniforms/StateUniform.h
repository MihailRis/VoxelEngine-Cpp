//
// Created by chelovek on 11/19/23.
//

#ifndef STATEUNIFORM_H
#define STATEUNIFORM_H
#include <glm/glm.hpp>

struct StateUniform {
    glm::mat4 projection;
    glm::mat4 view;
    alignas(16) glm::vec3 cameraPos;
    alignas(4) float gamma;
};



#endif //STATEUNIFORM_H
