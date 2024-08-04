#ifndef VOXELS_CHUNK_HPP_
#define VOXELS_CHUNK_HPP_

#include <stdlib.h>

#include <memory>
#include <unordered_map>

#include "../constants.hpp"
#include "../lighting/Lightmap.hpp"
#include "voxel.hpp"

inline constexpr int CHUNK_DATA_LEN = CHUNK_VOL * 4;

class Lightmap;
class ContentLUT;
class Inventory;

namespace dynamic {
    class Map;
}

using chunk_inventories_map =
    std::unordered_map<uint, std::shared_ptr<Inventory>>;

class Chunk {
public:
    int x, z;
    int bottom, top;
    voxel voxels[CHUNK_VOL] {};
    Lightmap lightmap;
    struct {
        bool modified : 1;
        bool ready : 1;
        bool loaded : 1;
        bool lighted : 1;
        bool unsaved : 1;
        bool loadedLights : 1;
        bool entities : 1;
    } flags {};

    /// @brief Block inventories map where key is index of block in voxels array
    chunk_inventories_map inventories;

    Chunk(int x, int z);

    bool isEmpty();

    void updateHeights();

    // unused
    std::unique_ptr<Chunk> clone() const;

    /// @brief Creates new block inventory given size
    /// @return inventory id or 0 if block does not exists
    void addBlockInventory(
        std::shared_ptr<Inventory> inventory, uint x, uint y, uint z
    );
    void removeBlockInventory(uint x, uint y, uint z);
    void setBlockInventories(chunk_inventories_map map);

    /// @return inventory bound to the given block or nullptr
    std::shared_ptr<Inventory> getBlockInventory(uint x, uint y, uint z) const;

    inline void setModifiedAndUnsaved() {
        flags.modified = true;
        flags.unsaved = true;
    }

    std::unique_ptr<ubyte[]> encode() const;

    /// @return true if all is fine
    bool decode(const ubyte* data);

    static void convert(ubyte* data, const ContentLUT* lut);
};

#endif /* VOXELS_CHUNK_HPP_ */
