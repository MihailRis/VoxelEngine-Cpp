#pragma once

#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

class SurroundMap {
    struct Entry {
        /// @brief Level is increased when all surrounding (8) entries having
        // greather or equal confirmed level
        int level = 0;
        /// @brief number of surrounding entries having greather or equal
        /// confirmed level
        int surrounding = 0;
        /// @brief level confirmed status (entry is ready to expand)
        bool confirmed = true;
        /// @brief mark used on sweep event (extra isles garbage collection)
        bool marked = false;
    };
    std::unordered_map<glm::ivec2, Entry> entries;
public:
    /// @brief Reset all isles marks
    void resetMarks();

    /// @brief Mark all connected entries
    /// @param point origin point
    void markIsle(const glm::ivec2& point);

    /// @brief Erase all non-marked isles
    /// @return erased entries positions
    std::vector<glm::ivec2> sweep();

    /// @brief Expand all confirmed entries with specified level
    /// @param level target entries level
    /// @return All upgraded entries positions 
    std::vector<glm::ivec2> expand(int level);
};
