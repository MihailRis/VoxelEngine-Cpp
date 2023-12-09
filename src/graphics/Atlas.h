#ifndef GRAPHICS_ATLAS_H_
#define GRAPHICS_ATLAS_H_

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "UVRegion.h"
#include "../typedefs.h"

class ImageData;
class Texture;

class Atlas {
    Texture* texture;
    ImageData* image;
    std::unordered_map<std::string, UVRegion> regions;
public:
    Atlas(ImageData* image, const std::unordered_map<std::string, UVRegion>& regions);
    ~Atlas();

    bool has(const std::string& name) const;
    const UVRegion& get(const std::string& name) const;

    Texture* getTexture() const;
    ImageData* getImage() const;
};


struct atlasentry {
    std::string name;
    std::shared_ptr<ImageData> image;
};

class AtlasBuilder {
    std::vector<atlasentry> entries;   
public:
    AtlasBuilder() {}
    void add(const std::string& name, ImageData* image);

    Atlas* build(uint extrusion, uint maxResolution=8192);
};

#endif // GRAPHICS_ATLAS_H_