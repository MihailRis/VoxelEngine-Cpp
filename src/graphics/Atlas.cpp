#include "Atlas.h"

#include <stdexcept>
#include "../maths/LMPacker.h"
#include "Texture.h"
#include "ImageData.h"

using std::vector;
using std::string;
using std::unique_ptr;
using std::shared_ptr;
using std::unordered_map;

Atlas::Atlas(ImageData* image, 
             unordered_map<string, UVRegion> regions)
      : texture(Texture::from(image)),
        image(image),
        regions(regions) {        
}

Atlas::~Atlas() {
    delete image;
    delete texture;
}

bool Atlas::has(string name) const {
    return regions.find(name) != regions.end();
}

const UVRegion& Atlas::get(string name) const {
    return regions.at(name);
}

Texture* Atlas::getTexture() const {
    return texture;
}

ImageData* Atlas::getImage() const {
    return image;
}

void AtlasBuilder::add(string name, ImageData* image) {
    entries.push_back(atlasentry{name, shared_ptr<ImageData>(image)});
    names.insert(name);
}

bool AtlasBuilder::has(string name) const {
    return names.find(name) != names.end();
}

Atlas* AtlasBuilder::build(uint extrusion, uint maxResolution) {
    unique_ptr<uint[]> sizes (new uint[entries.size() * 2]);
    uint index = 0;
    for (auto& entry : entries) {
        auto image = entry.image;
        sizes[index++] = image->getWidth();
        sizes[index++] = image->getHeight();
    }
    LMPacker packer(sizes.get(), entries.size()*2);
    sizes.reset(nullptr);

    uint width = 32;
    uint height = 32;
    while (!packer.buildCompact(width, height, extrusion)) {
        if (width > height) {
            height *= 2;
        } else {
            width *= 2;
        }
        if (width > maxResolution || height > maxResolution) {
            throw std::runtime_error("max atlas resolution "+
                                     std::to_string(maxResolution)+" exceeded");
        }
    }

    unordered_map<string, UVRegion> regions;
    unique_ptr<ImageData> canvas (new ImageData(ImageFormat::rgba8888, width, height));
    vector<rectangle> rects = packer.getResult();
    for (uint i = 0; i < entries.size(); i++) {
        const rectangle& rect = rects[i];
        const atlasentry& entry = entries[rect.idx];
        uint x = rect.x;
        uint y = rect.y;
        uint w = rect.width;
        uint h = rect.height;
        canvas->blit(entry.image.get(), rect.x, rect.y);
        for (uint j = 0; j < extrusion; j++) {
            canvas->extrude(x - j, y - j, w + j*2, h + j*2);
        }
        float unitX = 1.0f / width;
        float unitY = 1.0f / height;
        regions[entry.name] = UVRegion(unitX * x, unitY * y, 
                                       unitX * (x + w), unitY * (y + h));
    }
    return new Atlas(canvas.release(), regions);
}