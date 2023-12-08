//
// Created by chelovek on 11/28/23.
//

#ifndef SKYBOXUNIFORM_H
#define SKYBOXUNIFORM_H

#include <glm/glm.hpp>

struct SkyboxUniform {
    alignas(16) glm::vec3 xaxis[6];
    alignas(16) glm::vec3 yaxis[6];
    alignas(16) glm::vec3 zaxis[6];
    alignas(16) glm::vec3 lightDir;
    alignas(4) int quality;
    alignas(4) float mie;
    alignas(4) float fog;
};



#endif //SKYBOXUNIFORM_H
