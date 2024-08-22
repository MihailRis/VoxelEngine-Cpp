#include "Structure.hpp"

#include <cstring>

#include "data/dynamic.hpp"
#include "data/dynamic_util.hpp"
#include "voxels/ChunksStorage.hpp"
#include "voxels/VoxelsVolume.hpp"
#include "world/Level.hpp"

std::unique_ptr<Structure> Structure::create(
    Level* level, const glm::ivec3& a, const glm::ivec3& b, bool entities
) {
    auto start = glm::min(a, b);
    auto size = glm::abs(a - b);

    VoxelsVolume volume(size.x, size.y, size.z);
    level->chunksStorage->getVoxels(&volume);

    auto volVoxels = volume.getVoxels();
    std::vector<voxel> voxels(size.x*size.y*size.z);
    std::memcpy(voxels.data(), volVoxels, sizeof(voxel) * voxels.size());

    return std::make_unique<Structure>(size, std::move(voxels));
}

std::unique_ptr<dynamic::Map> Structure::serialize() const {
    auto root = std::make_unique<dynamic::Map>();
    root->put("version", STRUCTURE_FORMAT_VERSION);
    root->put("size", dynamic::to_value(size));

    auto& voxelsArr = root->putList("voxels");
    for (size_t i = 0; i < voxels.size(); i++) {
        voxelsArr.put(voxels[i].id);
        voxelsArr.put(blockstate2int(voxels[i].state));
    }
    return root;
}

void Structure::deserialize(dynamic::Map* src) {
    size = glm::ivec3();
    dynamic::get_vec(src, "size", size);
    voxels.resize(size.x*size.y*size.z);

    auto voxelsArr = src->list("voxels");
    for (size_t i = 0; i < size.x*size.y*size.z; i++) {
        voxels[i].id = voxelsArr->integer(i * 2);
        voxels[i].state = int2blockstate(voxelsArr->integer(i * 2 + 1));
    }
}
