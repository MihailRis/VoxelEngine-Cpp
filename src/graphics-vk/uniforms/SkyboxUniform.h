//
// Created by chelovek on 11/28/23.
//

#ifndef SKYBOXUNIFORM_H
#define SKYBOXUNIFORM_H

#include <glm/glm.hpp>

struct vec3alig {
    alignas(16) glm::vec3 val;
};

struct SkyboxUniform {
    alignas(16) vec3alig xaxis[6];
    alignas(16) vec3alig yaxis[6];
    alignas(16) vec3alig zaxis[6];
    alignas(16) glm::vec3 lightDir;
    alignas(4) int quality;
    alignas(4) float mie;
    alignas(4) float fog;
};



#endif //SKYBOXUNIFORM_H
