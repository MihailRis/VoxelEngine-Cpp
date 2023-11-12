#ifndef CODERS_PNG_H_
#define CODERS_PNG_H_

#include <string>

class Texture;
class ImageData;

namespace png {
    extern ImageData load_image(std::string filename);
    extern Texture* load_texture(std::string filename);
}

#endif /* CODERS_PNG_H_ */
