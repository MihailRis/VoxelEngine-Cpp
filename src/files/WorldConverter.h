#ifndef FILES_WORLD_CONVERTER_H_
#define FILES_WORLD_CONVERTER_H_

#include <queue>
#include <memory>
#include <filesystem>

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

class WorldConverter {
    WorldFiles* wfile;
    std::shared_ptr<ContentLUT> const lut;
    const Content* const content;
    std::queue<convert_task> tasks;

    void convertPlayer(fs::path file);
    void convertRegion(fs::path file);
public:
    WorldConverter(fs::path folder, const Content* content, 
                   std::shared_ptr<ContentLUT> lut);
    ~WorldConverter();

    bool hasNext() const;
    void convertNext();

    void write();
};

#endif // FILES_WORLD_CONVERTER_H_
