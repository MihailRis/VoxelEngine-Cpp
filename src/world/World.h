#ifndef WORLD_WORLD_H_
#define WORLD_WORLD_H_

#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include "../typedefs.h"
#include "../settings.h"
#include "../util/timeutil.h"

#include "../content/ContentPack.h"

class Content;
class WorldFiles;
class Chunks;
class Level;
class Player;
class ContentLUT;

namespace fs = std::filesystem;

class world_load_error : public std::runtime_error {
public:
    world_load_error(std::string message);
};

class World {
    std::string name;
    uint64_t seed;
    EngineSettings& settings;
    const Content* const content;
    std::vector<ContentPack> packs;
public:
    WorldFiles* wfile;

    /* Day/night loop timer in range 0..1 
       0.0 - is midnight
       0.5 - is noon
    */
    float daytime = timeutil::time_value(10, 00, 00);
    float daytimeSpeed = 1.0f/60.0f/24.0f;
    double totalTime = 0.0;

    World(std::string name, 
          fs::path directory, 
          uint64_t seed, 
          EngineSettings& settings,
          const Content* content,
          std::vector<ContentPack> packs);
    ~World();

    void updateTimers(float delta);
    void write(Level* level);

    static ContentLUT* checkIndices(const fs::path& directory,
                                         const Content* content);

    static Level* create(std::string name, 
                         fs::path directory, 
                         uint64_t seed, 
                         EngineSettings& settings, 
                         const Content* content,
                         const std::vector<ContentPack>& packs);
    static Level* load(fs::path directory,
                       EngineSettings& settings,
                       const Content* content,
                       const std::vector<ContentPack>& packs);

    void setName(const std::string& name);
    void setSeed(uint64_t seed);
    
    bool hasPack(const std::string& id) const;
    std::string getName() const;
    uint64_t getSeed() const;
    const std::vector<ContentPack>& getPacks() const;
};

#endif /* WORLD_WORLD_H_ */
