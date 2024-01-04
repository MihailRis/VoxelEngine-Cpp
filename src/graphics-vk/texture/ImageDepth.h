//
// Created by chelovek on 11/18/23.
//

#ifndef IMAGEDEPTH_H
#define IMAGEDEPTH_H

#include "Image.h"

class ImageDepth final : public Image {
public:
    ImageDepth(VkExtent3D extent);
};



#endif //IMAGEDEPTH_H
