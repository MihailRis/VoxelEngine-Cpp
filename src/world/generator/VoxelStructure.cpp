#include "VoxelStructure.hpp"

#include <unordered_map>
#include <cstring>

#include "data/dv_util.hpp"
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

dv::value VoxelStructure::serialize() const {
    auto root = dv::object();
    root["version"] = STRUCTURE_FORMAT_VERSION;
    root["size"] = dv::to_value(size);

    auto& blockNamesArr = root.list("block-names");
    for (const auto& name : blockNames) {
        blockNamesArr.add(name);
    }
    auto& voxelsArr = root.list("voxels");
    for (size_t i = 0; i < voxels.size(); i++) {
        voxelsArr.add(voxels[i].id);
        voxelsArr.add(blockstate2int(voxels[i].state));
    }
    return root;
}

void VoxelStructure::deserialize(const dv::value& src) {
    size = glm::ivec3();
    dv::get_vec(src, "size", size);

    auto volume = size.x*size.y*size.z;
    voxels.resize(volume);

    const auto& voxelsArr = src["voxels"];
    for (size_t i = 0; i < volume; i++) {
        voxels[i].id = voxelsArr[i * 2].asInteger();
        voxels[i].state = int2blockstate(voxelsArr[i * 2 + 1].asInteger());
    }
}

void VoxelStructure::prepare(const Content& content) {
    auto volume = size.x*size.y*size.z;
    voxelsRuntime.resize(volume);
    for (size_t i = 0; i < volume; i++) {
        const auto& name = blockNames[voxels[i].id];
        voxelsRuntime[i].id = content.blocks.require(name).rt.id;
        voxelsRuntime[i].state = voxels[i].state;
    }
}
