#ifndef FILES_WORLD_CONVERTER_H_
#define FILES_WORLD_CONVERTER_H_

#include <queue>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

class Content;
class ContentLUT;
class WorldFiles;

class WorldConverter {
    WorldFiles* wfile;
    std::shared_ptr<ContentLUT> const lut;
    const Content* const content;
    std::queue<fs::path> regions;
public:
    WorldConverter(fs::path folder, const Content* content, 
                   std::shared_ptr<ContentLUT> lut);
    ~WorldConverter();

    bool hasNext() const;
    void convertNext();

    void write();
};

#endif // FILES_WORLD_CONVERTER_H_
