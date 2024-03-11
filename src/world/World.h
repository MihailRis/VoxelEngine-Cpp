#ifndef WORLD_WORLD_H_
#define WORLD_WORLD_H_

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <stdexcept>
#include "../typedefs.h"
#include "../settings.h"
#include "../util/timeutil.h"
#include "../data/dynamic.h"
#include "../interfaces/Serializable.h"
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

/// @brief holds all world data except the level (chunks and objects)
class World : Serializable {
    std::string name;
    std::string generator;
    uint64_t seed;
    EngineSettings& settings;
    const Content* const content;
    std::vector<ContentPack> packs;

    int64_t nextInventoryId = 0;
public:
    std::unique_ptr<WorldFiles> wfile;

    /// @brief Day/night loop timer in range 0..1 where 
    /// 0.0 - is midnight and
    /// 0.5 - is noon
    float daytime = timeutil::time_value(10, 00, 00);

    // looking bad
    float daytimeSpeed = 1.0f/60.0f/24.0f;
    
    /// @brief total time passed in the world (not depending on daytimeSpeed)
    double totalTime = 0.0;

    World(
        std::string name, 
        std::string generator,
        fs::path directory, 
        uint64_t seed, 
        EngineSettings& settings,
        const Content* content,
        std::vector<ContentPack> packs
    );

    ~World();

    /// @brief Update world day-time and total time
    /// @param delta delta-time
    void updateTimers(float delta);
    
    /// @brief Write all unsaved level data to the world directory
    void write(Level* level);

    /// @brief Check world indices and generate ContentLUT if convert required 
    /// @param directory world directory
    /// @param content current Content instance
    /// @return ContentLUT if world convert required else nullptr 
    static ContentLUT* checkIndices(const fs::path& directory, const Content* content);

    /// @brief Create new world
    /// @param name internal world name
    /// @param directory root world directory
    /// @param type of the world
    /// @param seed world generation seed
    /// @param settings current engine settings
    /// @param content current engine Content instance 
    /// with all world content-packs applied
    /// @param packs vector of all world content-packs
    /// @return Level instance containing World instance
    static Level* create(
        std::string name, 
        std::string generator,
        fs::path directory, 
        uint64_t seed, 
        EngineSettings& settings, 
        const Content* content,
        const std::vector<ContentPack>& packs
    );

    /// @brief Load an existing world
    /// @param directory root world directory 
    /// @param settings current engine settings
    /// @param content current engine Content instance 
    /// with all world content-packs applied
    /// @param packs vector of all world content-packs
    /// @return Level instance containing World instance
    /// @throws world_load_error on world.json load error
    static Level* load(
        fs::path directory,
        EngineSettings& settings,
        const Content* content,
        const std::vector<ContentPack>& packs
    );

    void setName(const std::string& name);
    void setSeed(uint64_t seed);
    void setGenerator(const std::string& generator);

    /// @brief Check if world has content-pack installed 
    /// @param id content-pack id
    bool hasPack(const std::string& id) const;

    /// @brief Get internal world name (not the folder name)
    /// @return name stored in world.json
    std::string getName() const;

    /// @brief Get world generation seed
    uint64_t getSeed() const;

    /// @brief Get world generator id
    std::string getGenerator() const;

    /// @brief Get vector of all content-packs installed in world
    const std::vector<ContentPack>& getPacks() const;
    
    /// @brief Get next inventory id and increment it's counter
    /// @return integer >= 1
    int64_t getNextInventoryId() {
        return nextInventoryId++;
    }

    /// @brief Get current world Content instance
    const Content* getContent() const {
        return content;
    }

    std::unique_ptr<dynamic::Map> serialize() const override;
    void deserialize(dynamic::Map *src) override;
};

#endif /* WORLD_WORLD_H_ */
