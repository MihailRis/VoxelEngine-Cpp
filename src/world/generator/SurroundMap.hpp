#pragma once

#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

class SurroundMap {
    struct Entry {
        /// @brief Level is increased when all surrounding (8) entries having
        // greather or equal confirmed level
        int level = 0;
        /// @brief bits storing surrounding entries having greather or equal
        /// confirmed level.
        /// 0 - is x=-1,y=-1 offset, 1 is x=0,y=-1, ... 7 is x=1,y=1
        /// (Prevents extra access to the entries hashmap)
        uint8_t surrounding = 0x0;
        /// @brief level confirmed status (entry is ready to expand)
        bool confirmed = false;
        /// @brief mark used on sweep event (extra isles garbage collection)
        bool marked = false;
    };
    std::unordered_map<glm::ivec2, Entry> entries;
public:
    /// @brief Reset all isles marks
    void resetMarks();

    /// @brief Mark all connected entries
    /// @param origin origin point
    void markIsle(const glm::ivec2& origin);

    /// @brief Erase all non-marked isles
    /// @return erased entries positions
    std::vector<glm::ivec2> sweep();

    /// @brief Attempt to upgrade all confirmed entries with specified level
    /// @param level target entries level
    /// @return All upgraded entries positions 
    std::vector<glm::ivec2> upgrade();
    
    /// @brief Create entry if does not exist
    /// @param origin entry position
    /// @return true if new entry has been created
    bool createEntry(const glm::ivec2& origin);

    void getLevels(unsigned char* out, int width, int height, int ox, int oy) const;
};
