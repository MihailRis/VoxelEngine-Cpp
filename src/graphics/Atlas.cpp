#include "Atlas.h"

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
}

Atlas* AtlasBuilder::build(uint extrusion) {
    unique_ptr<uint[]> sizes (new uint[entries.size() * 2]);
    for (uint i = 0; i < entries.size(); i++) {
        auto& entry = entries[i];
        auto image = entry.image;
        sizes[i*2] = image->getWidth();
        sizes[i*2+1] = image->getHeight();
    }
    LMPacker packer(sizes.get(), entries.size()*2);
    sizes.reset(nullptr);

    int width = 32;
    int height = 32;
    while (!packer.buildCompact(width, height, extrusion)) {
        if (width > height) {
            height *= 2;
        } else {
            width *= 2;
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