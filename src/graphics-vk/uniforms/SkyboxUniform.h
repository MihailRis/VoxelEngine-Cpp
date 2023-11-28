//
// Created by chelovek on 11/28/23.
//

#ifndef SKYBOXUNIFORM_H
#define SKYBOXUNIFORM_H

#include <glm/glm.hpp>

class SkyboxUniform {
    alignas(16) glm::vec3 xaxis;
    alignas(16) glm::vec3 yaxis;
    alignas(16) glm::vec3 zaxis;
    alignas(16) glm::vec3 lightDir;
    alignas(4) int quality;
    alignas(4) float mie;
};



#endif //SKYBOXUNIFORM_H
