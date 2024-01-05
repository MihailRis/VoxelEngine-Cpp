#include "WorldConverter.h"

#include <memory>
#include <iostream>
#include <stdexcept>
#include "WorldFiles.h"
#include "../voxels/Chunk.h"
#include "../content/ContentLUT.h"

namespace fs = std::filesystem;

WorldConverter::WorldConverter(fs::path folder, 
                               const Content* content, 
                               const ContentLUT* lut) 
    : lut(lut), content(content) {
    DebugSettings settings;
    wfile = new WorldFiles(folder, settings);

    fs::path regionsFolder = wfile->getRegionsFolder();
    if (!fs::is_directory(regionsFolder)) {
        std::cerr << "nothing to convert" << std::endl;
        return;
    }
    for (auto file : fs::directory_iterator(regionsFolder)) {
        regions.push(file.path());
    }
}

WorldConverter::~WorldConverter() {
    delete wfile;
}

bool WorldConverter::hasNext() const {
    return !regions.empty();
}

void WorldConverter::convertNext() {
    if (!hasNext()) {
        throw std::runtime_error("no more regions to convert");
    }
    fs::path regfile = regions.front();
    regions.pop();
    if (!fs::is_regular_file(regfile))
        return;
    int x, z;
    std::string name = regfile.stem().string();
    if (!WorldFiles::parseRegionFilename(name, x, z)) {
        std::cerr << "could not parse name " << name << std::endl;
        return;
    }
    std::cout << "converting region " << name << std::endl;
    for (uint cz = 0; cz < REGION_SIZE; cz++) {
        for (uint cx = 0; cx < REGION_SIZE; cx++) {
            int gx = cx + x * REGION_SIZE;
            int gz = cz + z * REGION_SIZE;
            std::unique_ptr<u_char8[]> data (wfile->getChunk(gx, gz));
            if (data == nullptr)
                continue;
            if (wfile->getVoxelRegionVersion(x, z) != REGION_FORMAT_VERSION) {
                Chunk::fromOld(data.get());
            }
            if (lut) {
                Chunk::convert(data.get(), lut);
            }
            wfile->put(gx, gz, data.get());
        }
    }
}

void WorldConverter::write() {
    std::cout << "writing world" << std::endl;
    wfile->write(nullptr, content);
}
