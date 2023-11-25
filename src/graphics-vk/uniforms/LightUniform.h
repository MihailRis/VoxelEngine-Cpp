//
// Created by chelovek on 11/19/23.
//

#ifndef LIGHTUNIFORM_H
#define LIGHTUNIFORM_H

#include <glm/glm.hpp>

struct LightUniform {
    alignas(16) glm::vec3 torchlightColor;
    alignas(4) float torchlightDistance;
};

#endif //LIGHTUNIFORM_H
