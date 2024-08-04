#include "WorldConverter.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

#include <content/ContentLUT.hpp>
#include "../data/dynamic.hpp"
#include <debug/Logger.hpp>
#include <files/files.hpp>
#include "../objects/Player.hpp"
#include "../util/ThreadPool.hpp"
#include "../voxels/Chunk.hpp"
#include "WorldFiles.hpp"

namespace fs = std::filesystem;

static debug::Logger logger("world-converter");

class ConverterWorker : public util::Worker<convert_task, int> {
    std::shared_ptr<WorldConverter> converter;
public:
    ConverterWorker(std::shared_ptr<WorldConverter> converter)
        : converter(std::move(converter)) {
    }

    int operator()(const std::shared_ptr<convert_task>& task) override {
        converter->convert(*task);
        return 0;
    }
};

WorldConverter::WorldConverter(
    const fs::path& folder,
    const Content* content,
    std::shared_ptr<ContentLUT> lut
)
    : wfile(std::make_unique<WorldFiles>(folder)),
      lut(std::move(lut)),
      content(content) {
    fs::path regionsFolder =
        wfile->getRegions().getRegionsFolder(REGION_LAYER_VOXELS);
    if (!fs::is_directory(regionsFolder)) {
        logger.error() << "nothing to convert";
        return;
    }
    tasks.push(convert_task {convert_task_type::player, wfile->getPlayerFile()}
    );
    for (const auto& file : fs::directory_iterator(regionsFolder)) {
        tasks.push(convert_task {convert_task_type::region, file.path()});
    }
}

WorldConverter::~WorldConverter() {
}

std::shared_ptr<Task> WorldConverter::startTask(
    const fs::path& folder,
    const Content* content,
    const std::shared_ptr<ContentLUT>& lut,
    const runnable& onDone,
    bool multithreading
) {
    auto converter = std::make_shared<WorldConverter>(folder, content, lut);
    if (!multithreading) {
        converter->setOnComplete([=]() {
            converter->write();
            onDone();
        });
        return converter;
    }
    auto pool = std::make_shared<util::ThreadPool<convert_task, int>>(
        "converter-pool",
        [=]() { return std::make_shared<ConverterWorker>(converter); },
        [=](int&) {}
    );
    auto& converterTasks = converter->tasks;
    while (!converterTasks.empty()) {
        const convert_task& task = converterTasks.front();
        auto ptr = std::make_shared<convert_task>(task);
        pool->enqueueJob(ptr);
        converterTasks.pop();
    }
    pool->setOnComplete([=]() {
        converter->write();
        onDone();
    });
    return pool;
}

void WorldConverter::convertRegion(const fs::path& file) const {
    int x, z;
    std::string name = file.stem().string();
    if (!WorldRegions::parseRegionFilename(name, x, z)) {
        logger.error() << "could not parse name " << name;
        return;
    }
    logger.info() << "converting region " << name;
    wfile->getRegions().processRegionVoxels(x, z, [=](ubyte* data) {
        if (lut) {
            Chunk::convert(data, lut.get());
        }
        return true;
    });
}

void WorldConverter::convertPlayer(const fs::path& file) const {
    logger.info() << "converting player " << file.u8string();
    auto map = files::read_json(file);
    Player::convert(map.get(), lut.get());
    files::write_json(file, map.get());
}

void WorldConverter::convert(const convert_task& task) const {
    if (!fs::is_regular_file(task.file)) return;

    switch (task.type) {
        case convert_task_type::region:
            convertRegion(task.file);
            break;
        case convert_task_type::player:
            convertPlayer(task.file);
            break;
    }
}

void WorldConverter::convertNext() {
    if (tasks.empty()) {
        throw std::runtime_error("no more regions to convert");
    }
    convert_task task = tasks.front();
    tasks.pop();
    tasksDone++;

    convert(task);
}

void WorldConverter::setOnComplete(runnable callback) {
    this->onComplete = std::move(callback);
}

void WorldConverter::update() {
    convertNext();
    if (onComplete && tasks.empty()) {
        onComplete();
    }
}

void WorldConverter::terminate() {
    tasks = {};
}

bool WorldConverter::isActive() const {
    return !tasks.empty();
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

uint WorldConverter::getWorkTotal() const {
    return tasks.size() + tasksDone;
}

uint WorldConverter::getWorkDone() const {
    return tasksDone;
}
