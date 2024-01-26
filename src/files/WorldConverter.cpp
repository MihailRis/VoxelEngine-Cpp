#include "WorldConverter.h"

#include <memory>
#include <iostream>
#include <stdexcept>
#include "WorldFiles.h"

#include "../data/dynamic.h"
#include "../files/files.h"
#include "../voxels/Chunk.h"
#include "../content/ContentLUT.h"
#include "../objects/Player.h"

namespace fs = std::filesystem;

WorldConverter::WorldConverter(fs::path folder, 
                               const Content* content, 
                               std::shared_ptr<ContentLUT> lut) 
    : lut(lut), content(content) {
    DebugSettings settings;
    wfile = new WorldFiles(folder, settings);

    fs::path regionsFolder = wfile->getRegionsFolder();
    if (!fs::is_directory(regionsFolder)) {
        std::cerr << "nothing to convert" << std::endl;
        return;
    }
    tasks.push(convert_task {convert_task_type::player, wfile->getPlayerFile()});
    for (auto file : fs::directory_iterator(regionsFolder)) {
        tasks.push(convert_task {convert_task_type::region, file.path()});
    }
}

WorldConverter::~WorldConverter() {
    delete wfile;
}

bool WorldConverter::hasNext() const {
    return !tasks.empty();
}

void WorldConverter::convertRegion(fs::path file) {
    int x, z;
    std::string name = file.stem().string();
    if (!WorldFiles::parseRegionFilename(name, x, z)) {
        std::cerr << "could not parse name " << name << std::endl;
        return;
    }
    std::cout << "converting region " << name << std::endl;
    for (uint cz = 0; cz < REGION_SIZE; cz++) {
        for (uint cx = 0; cx < REGION_SIZE; cx++) {
            int gx = cx + x * REGION_SIZE;
            int gz = cz + z * REGION_SIZE;
            std::unique_ptr<ubyte[]> data (wfile->getChunk(gx, gz));
            if (data == nullptr)
                continue;
            if (lut) {
                Chunk::convert(data.get(), lut.get());
            }
            wfile->put(gx, gz, data.get());
        }
    }
}

void WorldConverter::convertPlayer(fs::path file) {
    std::cout << "converting player " << file.u8string() << std::endl;
    auto map = files::read_json(file);
    Player::convert(map.get(), lut.get());
    files::write_json(file, map.get());
}

void WorldConverter::convertNext() {
    if (!hasNext()) {
        throw std::runtime_error("no more regions to convert");
    }
    convert_task task = tasks.front();
    tasks.pop();

    if (!fs::is_regular_file(task.file))
        return;
    switch (task.type) {
        case convert_task_type::region:
            convertRegion(task.file);
            break;
        case convert_task_type::player:
            convertPlayer(task.file);
            break;
    }
}

void WorldConverter::write() {
    std::cout << "writing world" << std::endl;
    wfile->write(nullptr, content);
}
