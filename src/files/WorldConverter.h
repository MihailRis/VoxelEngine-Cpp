#ifndef FILES_WORLD_CONVERTER_H_
#define FILES_WORLD_CONVERTER_H_

#include <queue>
#include <memory>
#include <filesystem>

#include "../typedefs.h"
#include "../delegates.h"
#include "../interfaces/Task.hpp"

namespace fs = std::filesystem;

class Content;
class ContentLUT;
class WorldFiles;

enum class convert_task_type {
    region, player
};

struct convert_task {
    convert_task_type type;
    fs::path file;
};

class WorldConverter : public Task {
    std::unique_ptr<WorldFiles> wfile;
    std::shared_ptr<ContentLUT> const lut;
    const Content* const content;
    std::queue<convert_task> tasks;
    runnable onComplete;
    uint tasksDone = 0;

    void convertPlayer(fs::path file) const;
    void convertRegion(fs::path file) const;
public:
    WorldConverter(
        fs::path folder, 
        const Content* content, 
        std::shared_ptr<ContentLUT> lut
    );
    ~WorldConverter();

    void convert(convert_task task) const;
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
        fs::path folder, 
        const Content* content, 
        std::shared_ptr<ContentLUT> lut,
        runnable onDone,
        bool multithreading
    );
};

#endif // FILES_WORLD_CONVERTER_H_
