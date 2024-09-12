#include "VoxelStructure.hpp"

#include <unordered_map>
#include <cstring>

#include "data/dynamic.hpp"
#include "data/dynamic_util.hpp"
#include "content/Content.hpp"
#include "voxels/Block.hpp"
#include "voxels/ChunksStorage.hpp"
#include "voxels/VoxelsVolume.hpp"
#include "world/Level.hpp"

std::unique_ptr<VoxelStructure> VoxelStructure::create(
    Level* level, const glm::ivec3& a, const glm::ivec3& b, bool entities
) {
    auto start = glm::min(a, b);
    auto size = glm::abs(a - b);

    VoxelsVolume volume(size.x, size.y, size.z);
    volume.setPosition(start.x, start.y, start.z);
    level->chunksStorage->getVoxels(&volume);

    auto volVoxels = volume.getVoxels();
    std::vector<voxel> voxels(size.x*size.y*size.z);

    std::vector<std::string> blockNames;
    std::unordered_map<blockid_t, blockid_t> blocksRegistered;
    auto contentIndices = level->content->getIndices();
    for (size_t i = 0 ; i < voxels.size(); i++) {
        blockid_t id = volVoxels[i].id;
        blockid_t index;
        
        auto found = blocksRegistered.find(id);
        if (found == blocksRegistered.end()) {
            const auto& def = contentIndices->blocks.require(id);
            index = blockNames.size();
            blockNames.push_back(def.name);
            blocksRegistered[id] = index;
        } else {
            index = found->second;
        }
        voxels[i].id = index;
    }

    return std::make_unique<VoxelStructure>(
        size, std::move(voxels), std::move(blockNames));
}

std::unique_ptr<dynamic::Map> VoxelStructure::serialize() const {
    auto root = std::make_unique<dynamic::Map>();
    root->put("version", STRUCTURE_FORMAT_VERSION);
    root->put("size", dynamic::to_value(size));

    auto& blockNamesArr = root->putList("block-names");
    for (const auto& name : blockNames) {
        blockNamesArr.put(name);
    }
    auto& voxelsArr = root->putList("voxels");
    for (size_t i = 0; i < voxels.size(); i++) {
        voxelsArr.put(static_cast<integer_t>(voxels[i].id));
        voxelsArr.put(static_cast<integer_t>(blockstate2int(voxels[i].state)));
    }
    return root;
}

void VoxelStructure::deserialize(dynamic::Map* src) {
    size = glm::ivec3();
    dynamic::get_vec(src, "size", size);
    voxels.resize(size.x*size.y*size.z);

    auto voxelsArr = src->list("voxels");
    for (size_t i = 0; i < size.x*size.y*size.z; i++) {
        voxels[i].id = voxelsArr->integer(i * 2);
        voxels[i].state = int2blockstate(voxelsArr->integer(i * 2 + 1));
    }
}
