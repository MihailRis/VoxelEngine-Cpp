#pragma once

#include <memory>
#include <queue>

#include "delegates.hpp"
#include "interfaces/Task.hpp"
#include "io/io.hpp"
#include "world/files/world_regions_fwd.hpp"
#include "typedefs.hpp"

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
    /// @brief convert blocks data to updated layouts
    CONVERT_BLOCKS_DATA,
};

struct ConvertTask {
    ConvertTaskType type;
    io::path file;

    /// @brief region coords
    int x, z;
    RegionLayerIndex layer;
};

enum class ConvertMode {
    UPGRADE,
    REINDEX,
    BLOCK_FIELDS,
};

class WorldConverter : public Task {
    std::shared_ptr<WorldFiles> wfile;
    std::shared_ptr<ContentReport> const report;
    const Content* const content;
    std::queue<ConvertTask> tasks;
    runnable onComplete;
    uint tasksDone = 0;
    ConvertMode mode;

    void upgradeRegion(
        const io::path& file, int x, int z, RegionLayerIndex layer) const;
    void convertPlayer(const io::path& file) const;
    void convertVoxels(const io::path& file, int x, int z) const;
    void convertInventories(const io::path& file, int x, int z) const;
    void convertBlocksData(int x, int z, const ContentReport& report) const;

    void addRegionsTasks(
        RegionLayerIndex layerid,
        ConvertTaskType taskType
    );

    void createUpgradeTasks();
    void createConvertTasks();
    void createBlockFieldsConvertTasks();
public:
    WorldConverter(
        const std::shared_ptr<WorldFiles>& worldFiles,
        const Content* content,
        std::shared_ptr<ContentReport> report,
        ConvertMode mode
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
        ConvertMode mode,
        bool multithreading
    );
};

