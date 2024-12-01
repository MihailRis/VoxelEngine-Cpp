#include "BlockWrapsRenderer.hpp"

#include "assets/Assets.hpp"
#include "assets/assets_util.hpp"
#include "constants.hpp"
#include "content/Content.hpp"
#include "graphics/core/Atlas.hpp"
#include "graphics/core/Shader.hpp"
#include "graphics/core/DrawContext.hpp"
#include "graphics/render/MainBatch.hpp"
#include "objects/Player.hpp"
#include "voxels/Block.hpp"
#include "voxels/Chunks.hpp"
#include "window/Window.hpp"
#include "world/Level.hpp"
#include "world/LevelEvents.hpp"

BlockWrapsRenderer::BlockWrapsRenderer(const Assets& assets, const Level& level)
    : assets(assets), level(level), batch(std::make_unique<MainBatch>(1024)) {
    this->level.events->listen(
        EVT_BLOCK_SET,
        [this](lvl_event_type, void* pos) {
            if (const u64id_t existingId = get_id_by_pos(*static_cast<glm::ivec3*>(pos))) {
                remove(existingId);
            }
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
    if (const u64id_t existingId = get_id_by_pos(position)) {
        remove(existingId);
    }

    u64id_t id = nextWrapper++;
    wrappers[id] = std::make_unique<BlockWrapper>(
        BlockWrapper {position, texture}
    );
    positionIndex[position] = id;
    return id;
}

BlockWrapper* BlockWrapsRenderer::get(u64id_t id) const {
    const auto& found = wrappers.find(id);
    return (found != wrappers.end()) ? found->second.get() : nullptr;
}

u64id_t BlockWrapsRenderer::get_id_by_pos(const glm::ivec3& position) const {
    const auto& found = positionIndex.find(position);
    return (found != positionIndex.end()) ? found->second : 0;
}

void BlockWrapsRenderer::remove(u64id_t id) {
    if (const auto& found = wrappers.find(id); found != wrappers.end()) {
        positionIndex.erase(found->second->position);
        wrappers.erase(found);
    }
}