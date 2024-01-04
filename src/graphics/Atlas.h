#ifndef GRAPHICS_ATLAS_H_
#define GRAPHICS_ATLAS_H_

#include <set>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "UVRegion.h"
#include "../typedefs.h"

class ImageData;
class ITexture;

class Atlas {
    ITexture* texture;
    ImageData* image;
    std::unordered_map<std::string, UVRegion> regions;
public:
    Atlas(ImageData* image, std::unordered_map<std::string, UVRegion> regions);
    ~Atlas();

    bool has(std::string name) const;
    const UVRegion& get(std::string name) const;

    ITexture* getTexture() const;
    ImageData* getImage() const;
};


struct atlasentry {
    std::string name;
    std::shared_ptr<ImageData> image;
};

class AtlasBuilder {
    std::vector<atlasentry> entries;
    std::set<std::string> names;
public:
    AtlasBuilder() {}
    void add(std::string name, ImageData* image);
    bool has(std::string name) const;

    Atlas* build(uint extrusion, uint maxResolution=8192);
};

#endif // GRAPHICS_ATLAS_H_