#pragma once

#include <filesystem>
#include <memory>
#include <queue>

#include "delegates.hpp"
#include "interfaces/Task.hpp"
#include "files/world_regions_fwd.hpp"
#include "typedefs.hpp"

namespace fs = std::filesystem;

class Content;
class ContentReport;
class WorldFiles;

enum class ConvertTaskType {
    /// @brief rewrite voxels region indices
    VOXELS,
    /// @brief rewrite inventories region indices
    INVENTORIES,
    /// @brief rewrite player
    PLAYER,
    /// @brief refresh region file version
    UPGRADE_REGION,
};

struct ConvertTask {
    ConvertTaskType type;
    fs::path file;

    /// @brief region coords
    int x, z;
    RegionLayerIndex layer;
};

class WorldConverter : public Task {
    std::shared_ptr<WorldFiles> wfile;
    std::shared_ptr<ContentReport> const report;
    const Content* const content;
    std::queue<ConvertTask> tasks;
    runnable onComplete;
    uint tasksDone = 0;
    bool upgradeMode;

    void upgradeRegion(
        const fs::path& file, int x, int z, RegionLayerIndex layer) const;
    void convertPlayer(const fs::path& file) const;
    void convertVoxels(const fs::path& file, int x, int z) const;
    void convertInventories(const fs::path& file, int x, int z) const;

    void addRegionsTasks(
        RegionLayerIndex layerid,
        ConvertTaskType taskType
    );

    void createUpgradeTasks();
    void createConvertTasks();
public:
    WorldConverter(
        const std::shared_ptr<WorldFiles>& worldFiles,
        const Content* content,
        std::shared_ptr<ContentReport> report,
        bool upgradeMode
    );
    ~WorldConverter();

    void convert(const ConvertTask& task) const;
    void convertNext();
    void setOnComplete(runnable callback);
    void write();

    void update() override;
    void terminate() override;
    bool isActive() const override;
    void waitForEnd() override;
    uint getWorkTotal() const override;
    uint getWorkDone() const override;

    static std::shared_ptr<Task> startTask(
        const std::shared_ptr<WorldFiles>& worldFiles,
        const Content* content,
        const std::shared_ptr<ContentReport>& report,
        const runnable& onDone,
        bool upgradeMode,
        bool multithreading
    );
};

