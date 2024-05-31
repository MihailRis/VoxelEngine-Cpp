#ifndef VOXELS_CHUNK_HPP_
#define VOXELS_CHUNK_HPP_

#include <memory>
#include <stdlib.h>
#include <unordered_map>

#include "../constants.hpp"
#include "voxel.hpp"
#include "../lighting/Lightmap.hpp"

struct ChunkFlag {
    static const int MODIFIED = 0x1;
    static const int READY = 0x2;
    static const int LOADED = 0x4;
    static const int LIGHTED = 0x8;
    static const int UNSAVED = 0x10;
    static const int LOADED_LIGHTS = 0x20;
};
inline constexpr int CHUNK_DATA_LEN = CHUNK_VOL*4;

class Lightmap;
class ContentLUT;
class Inventory;

using chunk_inventories_map = std::unordered_map<uint, std::shared_ptr<Inventory>>;

class Chunk {
public:
    int x, z;
    int bottom, top;
    voxel voxels[CHUNK_VOL] {};
    Lightmap lightmap;
    int flags = 0;

    /* Block inventories map where key is index of block in voxels array */
    chunk_inventories_map inventories;

    Chunk(int x, int z);

    bool isEmpty();

    void updateHeights();

    // unused
    std::unique_ptr<Chunk> clone() const;

    // flags getters/setters below
    inline void setFlags(int mask, bool value){
        if (value)
            flags |= mask; 
        else
            flags &= ~(mask);
    }

    /* Creates new block inventory given size
       @return inventory id or 0 if block does not exists */
    void addBlockInventory(std::shared_ptr<Inventory> inventory, 
                           uint x, uint y, uint z);
    void removeBlockInventory(uint x, uint y, uint z);
    void setBlockInventories(chunk_inventories_map map);

    /* @return inventory bound to the given block or nullptr */
    std::shared_ptr<Inventory> getBlockInventory(uint x, uint y, uint z) const;

    inline bool isUnsaved() const {return flags & ChunkFlag::UNSAVED;}

    inline bool isModified() const {return flags & ChunkFlag::MODIFIED;}

    inline bool isLighted() const {return flags & ChunkFlag::LIGHTED;}

    inline bool isLoaded() const {return flags & ChunkFlag::LOADED;}

    inline bool isLoadedLights() const {return flags & ChunkFlag::LOADED_LIGHTS;}

    inline bool isReady() const {return flags & ChunkFlag::READY;}

    inline void setUnsaved(bool newState) {setFlags(ChunkFlag::UNSAVED, newState);}

    inline void setModified(bool newState) {setFlags(ChunkFlag::MODIFIED, newState);}

    inline void setLoaded(bool newState) {setFlags(ChunkFlag::LOADED, newState);}

    inline void setLoadedLights(bool newState) {setFlags(ChunkFlag::LOADED_LIGHTS, newState);}

    inline void setLighted(bool newState) {setFlags(ChunkFlag::LIGHTED, newState);}

    inline void setReady(bool newState) {setFlags(ChunkFlag::READY, newState);}

    std::unique_ptr<ubyte[]> encode() const;

    /**
     * @return true if all is fine
     **/
    bool decode(const ubyte* data);

    static void convert(ubyte* data, const ContentLUT* lut);
};

#endif /* VOXELS_CHUNK_HPP_ */
