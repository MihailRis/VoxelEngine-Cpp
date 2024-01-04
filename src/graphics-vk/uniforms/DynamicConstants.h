//
// Created by chelovek on 12/3/23.
//

#ifndef DYNAMICCONSTANTS_H
#define DYNAMICCONSTANTS_H

#include <glm/glm.hpp>

struct DynamicConstants {
    glm::mat4 model;
    glm::vec3 torchlightColor;
    float torchlightDistance;
};

#endif //DYNAMICCONSTANTS_H
