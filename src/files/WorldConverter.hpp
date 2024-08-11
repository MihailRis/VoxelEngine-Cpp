#pragma once

#include <filesystem>
#include <memory>
#include <queue>

#include "delegates.hpp"
#include "interfaces/Task.hpp"
#include "typedefs.hpp"

namespace fs = std::filesystem;

class Content;
class ContentLUT;
class WorldFiles;

enum class convert_task_type { region, player };

struct convert_task {
    convert_task_type type;
    fs::path file;
};

class WorldConverter : public Task {
    std::shared_ptr<WorldFiles> wfile;
    std::shared_ptr<ContentLUT> const lut;
    const Content* const content;
    std::queue<convert_task> tasks;
    runnable onComplete;
    uint tasksDone = 0;

    void convertPlayer(const fs::path& file) const;
    void convertRegion(const fs::path& file) const;
public:
    WorldConverter(
        const std::shared_ptr<WorldFiles>& worldFiles,
        const Content* content,
        std::shared_ptr<ContentLUT> lut
    );
    ~WorldConverter();

    void convert(const convert_task& task) const;
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
        const std::shared_ptr<ContentLUT>& lut,
        const runnable& onDone,
        bool multithreading
    );
};

