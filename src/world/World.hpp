#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "content/ContentPack.hpp"
#include "data/dynamic.hpp"
#include "interfaces/Serializable.hpp"
#include "typedefs.hpp"
#include "util/timeutil.hpp"

class Content;
class WorldFiles;
class Level;
class ContentLUT;
struct EngineSettings;

namespace fs = std::filesystem;

class world_load_error : public std::runtime_error {
public:
    world_load_error(const std::string& message);
};

struct WorldInfo : public Serializable {
    std::string name;
    std::string generator;
    uint64_t seed;
    int64_t nextInventoryId = 0;

    /// @brief Day/night loop timer in range 0..1 where
    /// 0.0 - is midnight and
    /// 0.5 - is noon
    float daytime = timeutil::time_value(10, 00, 00);

    // looking bad
    float daytimeSpeed = 1.0f;

    /// @brief total time passed in the world (not depending on daytimeSpeed)
    double totalTime = 0.0;

    /// @brief will be replaced with weather in future
    float fog = 0.0f;

    entityid_t nextEntityId = 0;

    int major = 0, minor = -1;

    std::unique_ptr<dynamic::Map> serialize() const override;
    void deserialize(dynamic::Map* src) override;
};

/// @brief holds all world data except the level (chunks and objects)
class World {
    WorldInfo info {};

    const Content* const content;
    std::vector<ContentPack> packs;

    int64_t nextInventoryId = 0;

    void writeResources(const Content* content);
public:
    std::shared_ptr<WorldFiles> wfile;

    World(
        WorldInfo info,
        const std::shared_ptr<WorldFiles>& worldFiles,
        const Content* content,
        const std::vector<ContentPack>& packs
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
    static std::shared_ptr<ContentLUT> checkIndices(
        const std::shared_ptr<WorldFiles>& worldFiles, const Content* content
    );

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
    static std::unique_ptr<Level> create(
        const std::string& name,
        const std::string& generator,
        const fs::path& directory,
        uint64_t seed,
        EngineSettings& settings,
        const Content* content,
        const std::vector<ContentPack>& packs
    );

    /// @brief Load an existing world
    /// @param worldFiles world files manager
    /// @param settings current engine settings
    /// @param content current engine Content instance
    /// with all world content-packs applied
    /// @param packs vector of all world content-packs
    /// @return Level instance containing World instance
    /// @throws world_load_error on world.json load error
    static std::unique_ptr<Level> load(
        const std::shared_ptr<WorldFiles>& worldFiles,
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

    WorldInfo& getInfo() {
        return info;
    }

    const WorldInfo& getInfo() const {
        return info;
    }

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
};
