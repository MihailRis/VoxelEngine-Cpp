#pragma once

#include <unordered_map>
#include <functional>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "typedefs.hpp"
#include "util/AreaMap2D.hpp"

class SurroundMap {
public:
    using LevelCallback = std::function<void(int, int)>;
    struct LevelCallbackWrapper {
        LevelCallback callback;
        bool active = false;
    };
private:
    util::AreaMap2D<int8_t> areaMap;
    std::vector<LevelCallbackWrapper> levelCallbacks;
    int8_t maxLevel;

    void upgrade(int x, int y, int8_t level);
public:
    SurroundMap(int maxLevelRadius, int8_t maxLevel);

    /// @brief Callback called on point level increments
    void setLevelCallback(int8_t level, LevelCallback callback);

    /// @brief Callback called when non-zero value moves out of area
    void setOutCallback(util::AreaMap2D<int8_t>::OutCallback callback);   
    
    /// @brief Upgrade point to maxLevel
    /// @throws std::invalid_argument - upgrade square is not fully inside
    void completeAt(int x, int y);

    /// @brief Set map area center
    void setCenter(int x, int y);

    void resize(int maxLevelRadius);

    /// @brief Get level at position
    /// @throws std::invalid_argument - position is out of area
    int8_t at(int x, int y);

    const util::AreaMap2D<int8_t>& getArea() const {
        return areaMap;
    }
};
