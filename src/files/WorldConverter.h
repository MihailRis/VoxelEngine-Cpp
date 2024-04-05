#ifndef FILES_WORLD_CONVERTER_H_
#define FILES_WORLD_CONVERTER_H_

#include <queue>
#include <memory>
#include <filesystem>

#include "../typedefs.h"
#include "../delegates.h"
#include "../interfaces/Task.h"

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
    WorldFiles* wfile;
    std::shared_ptr<ContentLUT> const lut;
    const Content* const content;
    std::queue<convert_task> tasks;
    runnable onComplete;
    uint tasksDone = 0;

    void convertPlayer(fs::path file);
    void convertRegion(fs::path file);
public:
    WorldConverter(
        fs::path folder, const Content* content, 
        std::shared_ptr<ContentLUT> lut
    );
    ~WorldConverter();

    void convertNext();

    void setOnComplete(runnable callback) {
        this->onComplete = callback;
    }

    void update() override {
        convertNext();
        if (onComplete && tasks.empty()) {
            onComplete();
        }
    }

    void terminate() override {
        tasks = {};
    }

    void write();

    uint getWorkRemaining() const override;
    uint getWorkDone() const override;
};

#endif // FILES_WORLD_CONVERTER_H_
