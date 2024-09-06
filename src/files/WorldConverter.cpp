#include "WorldConverter.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

#include "content/ContentReport.hpp"
#include "files/compatibility.hpp"
#include "data/dynamic.hpp"
#include "debug/Logger.hpp"
#include "files/files.hpp"
#include "objects/Player.hpp"
#include "util/ThreadPool.hpp"
#include "voxels/Chunk.hpp"
#include "items/Inventory.hpp"
#include "WorldFiles.hpp"

namespace fs = std::filesystem;

static debug::Logger logger("world-converter");

class ConverterWorker : public util::Worker<ConvertTask, int> {
    std::shared_ptr<WorldConverter> converter;
public:
    ConverterWorker(std::shared_ptr<WorldConverter> converter)
        : converter(std::move(converter)) {
    }

    int operator()(const std::shared_ptr<ConvertTask>& task) override {
        converter->convert(*task);
        return 0;
    }
};

void WorldConverter::addRegionsTasks(
    RegionLayerIndex layerid,
    ConvertTaskType taskType
) {
    const auto& regions = wfile->getRegions();
    auto regionsFolder = regions.getRegionsFolder(layerid);
    if (!fs::is_directory(regionsFolder)) {
        return;
    }
    for (const auto& file : fs::directory_iterator(regionsFolder)) {
        int x, z;
        std::string name = file.path().stem().string();
        if (!WorldRegions::parseRegionFilename(name, x, z)) {
            logger.error() << "could not parse region name " << name;
            continue;
        }
        tasks.push(ConvertTask {taskType, file.path(), x, z, layerid});
    }
}

void WorldConverter::createUpgradeTasks() {
    const auto& regions = wfile->getRegions();
    for (auto& issue : report->getIssues()) {
        if (issue.issueType != ContentIssueType::REGION_FORMAT_UPDATE) {
            continue;
        }
        addRegionsTasks(issue.regionLayer, ConvertTaskType::UPGRADE_REGION);
    }
}

void WorldConverter::createConvertTasks() {
    auto handleReorder = [=](ContentType contentType) {
        switch (contentType) {
            case ContentType::BLOCK:
                addRegionsTasks(
                    REGION_LAYER_VOXELS,
                    ConvertTaskType::VOXELS
                );
                break;
            case ContentType::ITEM:
                addRegionsTasks(
                    REGION_LAYER_INVENTORIES,
                    ConvertTaskType::INVENTORIES
                );
                break;
            default:
                break;
        }
    };

    const auto& regions = wfile->getRegions();
    for (auto& issue : report->getIssues()) {
        switch (issue.issueType) {
            case ContentIssueType::REGION_FORMAT_UPDATE:
                break;
            case ContentIssueType::MISSING:
                throw std::runtime_error("issue can't be resolved");
            case ContentIssueType::REORDER:
                handleReorder(issue.contentType);
                break;
        }
    }

    tasks.push(ConvertTask {ConvertTaskType::PLAYER, wfile->getPlayerFile()});
}

WorldConverter::WorldConverter(
    const std::shared_ptr<WorldFiles>& worldFiles,
    const Content* content,
    std::shared_ptr<ContentReport> reportPtr,
    bool upgradeMode
)
    : wfile(worldFiles),
      report(std::move(reportPtr)),
      content(content),
      upgradeMode(upgradeMode)
{
    if (upgradeMode) {
        createUpgradeTasks();
    } else {
        createConvertTasks();
    }
}

WorldConverter::~WorldConverter() {
}

std::shared_ptr<Task> WorldConverter::startTask(
    const std::shared_ptr<WorldFiles>& worldFiles,
    const Content* content,
    const std::shared_ptr<ContentReport>& report,
    const runnable& onDone,
    bool upgradeMode,
    bool multithreading
) {
    auto converter = std::make_shared<WorldConverter>(
        worldFiles, content, report, upgradeMode);
    if (!multithreading) {
        converter->setOnComplete([=]() {
            converter->write();
            onDone();
        });
        return converter;
    }
    auto pool = std::make_shared<util::ThreadPool<ConvertTask, int>>(
        "converter-pool",
        [=]() { return std::make_shared<ConverterWorker>(converter); },
        [=](int&) {}
    );
    auto& converterTasks = converter->tasks;
    while (!converterTasks.empty()) {
        const ConvertTask& task = converterTasks.front();
        auto ptr = std::make_shared<ConvertTask>(task);
        pool->enqueueJob(ptr);
        converterTasks.pop();
    }
    pool->setOnComplete([=]() {
        converter->write();
        onDone();
    });
    return pool;
}

void WorldConverter::upgradeRegion(
    const fs::path& file, int x, int z, RegionLayerIndex layer
) const {
    auto path = wfile->getRegions().getRegionFilePath(layer, x, z);
    auto bytes = files::read_bytes_buffer(path);
    auto buffer = compatibility::convert_region_2to3(bytes, layer);
    files::write_bytes(path, buffer.data(), buffer.size());
}

void WorldConverter::convertVoxels(const fs::path& file, int x, int z) const {
    logger.info() << "converting voxels region " << x << "_" << z;
    wfile->getRegions().processRegion(x, z, REGION_LAYER_VOXELS,
    [=](std::unique_ptr<ubyte[]> data, uint32_t*) {
        Chunk::convert(data.get(), report.get());
        return data;
    });
}

void WorldConverter::convertInventories(const fs::path& file, int x, int z) const {
    logger.info() << "converting inventories region " << x << "_" << z;
    wfile->getRegions().processInventories(x, z, [=](Inventory* inventory) {
        inventory->convert(report.get());
    });
}

void WorldConverter::convertPlayer(const fs::path& file) const {
    logger.info() << "converting player " << file.u8string();
    auto map = files::read_json(file);
    Player::convert(map.get(), report.get());
    files::write_json(file, map.get());
}

void WorldConverter::convert(const ConvertTask& task) const {
    if (!fs::is_regular_file(task.file)) return;

    switch (task.type) {
        case ConvertTaskType::UPGRADE_REGION:
            upgradeRegion(task.file, task.x, task.z, task.layer);
            break;
        case ConvertTaskType::VOXELS:
            convertVoxels(task.file, task.x, task.z);
            break;
        case ConvertTaskType::INVENTORIES:
            convertInventories(task.file, task.x, task.z);
            break;
        case ConvertTaskType::PLAYER:
            convertPlayer(task.file);
            break;
    }
}

void WorldConverter::convertNext() {
    if (tasks.empty()) {
        throw std::runtime_error("no more regions to convert");
    }
    ConvertTask task = tasks.front();
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
    if (upgradeMode) {
        logger.info() << "refreshing version";
        wfile->patchIndicesVersion("region-version", REGION_FORMAT_VERSION);
    } else {
        logger.info() << "writing world";
        wfile->write(nullptr, content);
    }
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
