#pragma once

#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "typedefs.hpp"
#include "util/AreaMap2D.hpp"

class SurroundMap {
public:
    using LevelCallback = std::function<void(ubyte)>;
private:
    util::AreaMap2D<ubyte> areaMap;
    std::vector<LevelCallback> levelCallbacks;
    ubyte maxLevel;
public:
    SurroundMap(int loadDistance, ubyte maxLevel);

    void setLevelCallback(int level, LevelCallback callback);
    void setOutCallback(util::AreaMap2D<ubyte>::OutCallback callback);

    void completeAt(int x, int y);

    void setCenter(int x, int y);
};
