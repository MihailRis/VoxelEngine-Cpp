//
// Created by chelovek on 11/19/23.
//

#ifndef FOGUNIFORM_H
#define FOGUNIFORM_H

#include <glm/glm.hpp>

struct FogUniform {
    alignas(4) float fogFactor;
    alignas(4) float fogCurve;
};

#endif //FOGUNIFORM_H
