#include "BlockWrapsRenderer.hpp"

#include "assets/Assets.hpp"
#include "assets/assets_util.hpp"
#include "constants.hpp"
#include "content/Content.hpp"
#include "graphics/core/Shader.hpp"
#include "graphics/core/DrawContext.hpp"
#include "graphics/render/MainBatch.hpp"
#include "voxels/Block.hpp"
#include "voxels/Chunks.hpp"
#include "world/Level.hpp"
#include "world/LevelEvents.hpp"

BlockWrapsRenderer::BlockWrapsRenderer(const Assets& assets, const Level& level)
    : assets(assets), level(level), batch(std::make_unique<MainBatch>(1024)) {
    this->level.events->listen(
        EVT_BLOCK_CHANGED,
        [this](lvl_event_type, void* pos) {
            remove_by_position(*static_cast<glm::ivec3*>(pos));
        }
    );
}

BlockWrapsRenderer::~BlockWrapsRenderer() = default;

void BlockWrapsRenderer::draw(const BlockWrapper& wrapper) {
    const auto& chunks = *level.chunks;

    auto textureRegion = util::get_texture_region(assets, wrapper.texture, "");

    auto& shader = assets.require<Shader>("entity");
    shader.use();
    shader.uniform1i("u_alphaClip", false);

    const UVRegion& cracksRegion = textureRegion.region;
    UVRegion regions[6] {
        cracksRegion, cracksRegion, cracksRegion,
        cracksRegion, cracksRegion, cracksRegion
    };
    batch->setTexture(textureRegion.texture);

    const voxel* vox = chunks.get(wrapper.position);
    if (vox == nullptr) {
        return;
    }
    if (vox->id != BLOCK_VOID) {
        const auto& def =
            level.content->getIndices()->blocks.require(vox->id);
        switch (def.model) {
            case BlockModel::block:
                batch->cube(
                    glm::vec3(wrapper.position) + glm::vec3(0.5f),
                    glm::vec3(1.01f),
                    regions,
                    glm::vec4(1,1,1,0),
                    false
                );
                break;
            case BlockModel::aabb: {
                const auto& aabb = def.rt.hitboxes[vox->state.rotation].at(0);
                const auto& size = aabb.size();
                regions[0].scale(size.z, size.y);
                regions[1].scale(size.z, size.y);
                regions[2].scale(size.x, size.z);
                regions[3].scale(size.x, size.z);
                regions[4].scale(size.x, size.y);
                regions[5].scale(size.x, size.y);
                batch->cube(
                    glm::vec3(wrapper.position) + aabb.center(),
                    size * glm::vec3(1.01f),
                    regions,
                    glm::vec4(1,1,1,0),
                    false
                );
                break;
            }
            default:
                break;
        }
    }
}

void BlockWrapsRenderer::draw(const DrawContext& pctx, const Player& player) {
    auto ctx = pctx.sub();
    for (const auto& [_, wrapper] : wrappers) {
        draw(*wrapper);
    }
    batch->flush();
}

u64id_t BlockWrapsRenderer::add(
    const glm::ivec3& position, const std::string& texture
) {
    u64id_t id = nextWrapper++;
    wrappers[id] = std::make_unique<BlockWrapper>(
        BlockWrapper {position, texture}
    );
    positionIndex[position].insert(id);
    return id;
}

BlockWrapper* BlockWrapsRenderer::get(u64id_t id) const {
    const auto& found = wrappers.find(id);
    if (found == wrappers.end()) {
        return nullptr;
    }
    return found->second.get();
}

const std::unordered_set<u64id_t>* BlockWrapsRenderer::get_ids_by_position(const glm::ivec3& position) const {
    const auto found = positionIndex.find(position);
    return (found != positionIndex.end()) ? &found->second : nullptr;
}

void BlockWrapsRenderer::remove(u64id_t id) {
    if (const auto& found = wrappers.find(id); found != wrappers.end()) {
        const glm::ivec3& position = found->second->position;
        if (const auto pos = positionIndex.find(position); pos != positionIndex.end()) {
            pos->second.erase(id);
            if (pos->second.empty()) {
                positionIndex.erase(pos);
            }
        }
        wrappers.erase(found);
    }
}

void BlockWrapsRenderer::remove_by_position(const glm::ivec3& position) {
    if (const auto& found = positionIndex.find(position); found != positionIndex.end()) {
        for (const auto& id : found->second) {
            wrappers.erase(id);
        }
        positionIndex.erase(found);
    }
}