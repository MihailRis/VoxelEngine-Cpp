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
#include "../debug/Logger.h"

namespace fs = std::filesystem;

static debug::Logger logger("world-converter");

WorldConverter::WorldConverter(
    fs::path folder, 
    const Content* content, 
    std::shared_ptr<ContentLUT> lut
) : wfile(std::make_unique<WorldFiles>(folder, DebugSettings {})), 
    lut(lut), 
    content(content) 
{
    fs::path regionsFolder = wfile->getRegionsFolder(REGION_LAYER_VOXELS);
    if (!fs::is_directory(regionsFolder)) {
        logger.error() << "nothing to convert";
        return;
    }
    tasks.push(convert_task {convert_task_type::player, wfile->getPlayerFile()});
    for (auto file : fs::directory_iterator(regionsFolder)) {
        tasks.push(convert_task {convert_task_type::region, file.path()});
    }
}

WorldConverter::~WorldConverter() {
}

void WorldConverter::convertRegion(fs::path file) {
    int x, z;
    std::string name = file.stem().string();
    if (!WorldFiles::parseRegionFilename(name, x, z)) {
        logger.error() << "could not parse name " << name;
        return;
    }
    logger.info() << "converting region " << name;
    wfile->processRegionVoxels(x, z, [=](ubyte* data) {
        if (lut) {
            Chunk::convert(data, lut.get());
        }
        return true;
    });
}

void WorldConverter::convertPlayer(fs::path file) {
    logger.info() << "converting player " << file.u8string();
    auto map = files::read_json(file);
    Player::convert(map.get(), lut.get());
    files::write_json(file, map.get());
}

void WorldConverter::convertNext() {
    if (tasks.empty()) {
        throw std::runtime_error("no more regions to convert");
    }
    convert_task task = tasks.front();
    tasks.pop();
    tasksDone++;

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
    logger.info() << "writing world";
    wfile->write(nullptr, content);
}

void WorldConverter::waitForEnd() {
    while (isActive()) {
        update();
    }
}

uint WorldConverter::getWorkRemaining() const {
    return tasks.size();
}

uint WorldConverter::getWorkDone() const {
    return tasksDone;
}
