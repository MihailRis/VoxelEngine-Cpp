#ifndef CODERS_PNG_H_
#define CODERS_PNG_H_

#include <string>
#include "../typedefs.h"

class Texture;
class ImageData;

namespace png {
    extern ImageData* load_image(std::string filename);
    extern void write_image(std::string filename, const ImageData* image);
    extern Texture* load_texture(std::string filename);
}

#endif /* CODERS_PNG_H_ */
