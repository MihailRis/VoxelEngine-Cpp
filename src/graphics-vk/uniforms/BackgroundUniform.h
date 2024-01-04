//
// Created by chelovek on 11/28/23.
//

#ifndef BACKGROUNDUNIFORM_H
#define BACKGROUNDUNIFORM_H

#include <glm/glm.hpp>

struct BackgroundUniform {
    glm::mat4 view;
    alignas(4) float ar;
    alignas(4) float zoom;
};



#endif //BACKGROUNDUNIFORM_H
