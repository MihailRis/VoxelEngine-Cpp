#ifndef GRAPHICS_CORE_ATLAS_H_
#define GRAPHICS_CORE_ATLAS_H_

#include <set>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "../../maths/UVRegion.h"
#include "../../typedefs.h"

class ImageData;
class Texture;

class Atlas {
    std::unique_ptr<Texture> texture;
    std::unique_ptr<ImageData> image;
    std::unordered_map<std::string, UVRegion> regions;
public:
    Atlas(ImageData* image, std::unordered_map<std::string, UVRegion> regions);
    ~Atlas();

    void prepare();

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
    std::set<std::string> names;
public:
    AtlasBuilder() {}
    void add(std::string name, ImageData* image);
    bool has(const std::string& name) const;
    const std::set<std::string>& getNames() { return names; };

    Atlas* build(uint extrusion, uint maxResolution=8192);
};

#endif // GRAPHICS_CORE_ATLAS_H_
