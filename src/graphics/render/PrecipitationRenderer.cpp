#include "PrecipitationRenderer.hpp"

#include "assets/Assets.hpp"
#include "assets/assets_util.hpp"
#include "graphics/core/Shader.hpp"
#include "graphics/core/Texture.hpp"
#include "lighting/Lightmap.hpp"
#include "maths/util.hpp"
#include "window/Camera.hpp"
#include "world/Level.hpp"
#include "voxels/Chunks.hpp"
#include "MainBatch.hpp"
#include "settings.hpp"

PrecipitationRenderer::PrecipitationRenderer(
    const Assets& assets,
    const Level& level,
    const Chunks& chunks,
    const GraphicsSettings* settings
)
    : batch(std::make_unique<MainBatch>(4096)),
      level(level),
      chunks(chunks),
      assets(assets),
      settings(settings) {
}

PrecipitationRenderer::~PrecipitationRenderer() = default;

int PrecipitationRenderer::getHeightAt(int x, int z) {
    int y = CHUNK_H-1;
    while (y > 0) {
        if (auto voxel = chunks.get({x, y, z})) {
            if (voxel->id == 0) {
                y--;
                continue;
            }
        }
        break;
    }
    return y;
}

static inline glm::vec4 light_at(const Chunks& chunks, int x, int y, int z) {
    light_t lightval = chunks.getLight(x, y, z);
    return glm::vec4(
        Lightmap::extract(lightval, 0) / 15.f,
        Lightmap::extract(lightval, 1) / 15.f,
        Lightmap::extract(lightval, 2) / 15.f,
        1
    );
}

void PrecipitationRenderer::render(const Camera& camera, float delta) {
    timer += delta * 0.5f;
    batch->begin();

    util::PseudoRandom random(0);

    int x = glm::floor(camera.position.x);
    int y = glm::floor(camera.position.y);
    int z = glm::floor(camera.position.z);

    auto& texture = assets.require<Texture>("misc/snow");
    texture.setMipMapping(false);
    batch->setTexture(&texture, {});

    const auto& front = camera.front;
    glm::ivec2 size {1, 16};

    float horizontal = 0.5f;

    glm::vec4 light = light_at(chunks, x, y, z);

    int radius = 6;
    int depth = 12;
    float scale = 0.4f;
    int quads = 0;
    float k = 21.41149;
    for (int lx = -radius; lx <= radius; lx++) {
        for (int lz = -depth; lz < 0; lz++) {
            glm::vec4 light = light_at(chunks, x + lx, y, z + lz);
            random.setSeed(lx + x, lz + z);
            float hspeed = (random.randFloat() * 2.0f - 1.0f) * horizontal;
            glm::vec3 pos {
                x + lx + 0.5f,
                glm::max(y - size.y / 2, getHeightAt(x + lx, z + lz)) + size.y / 2 + 1,
                z + lz + 0.5f};
            batch->quad(
                pos,
                {1, 0, 0},
                {0, 1, 0},
                size,
                light,
                glm::vec3(1.0f),
                UVRegion(
                    (lx + x) * scale + timer * hspeed,
                    timer + y * scale + (z + lz) * k,
                    (lx + x + 1) * scale + timer * hspeed,
                    timer + (size.y + y) * scale + (z + lz) * k
                )
            );
        }
    }
    for (int lx = -radius; lx <= radius; lx++) {
        for (int lz = depth; lz > 0; lz--) {
            glm::vec4 light = light_at(chunks, x + lx, y, z + lz);
            random.setSeed(lx + x, lz + z);
            float hspeed = (random.randFloat() * 2.0f - 1.0f) * horizontal;
            glm::vec3 pos {
                x + lx + 0.5f,
                glm::max(y - size.y / 2, getHeightAt(x + lx, z + lz)) + size.y / 2 + 1,
                z + lz + 0.5f};
            batch->quad(
                pos,
                {-1, 0, 0},
                {0, 1, 0},
                size,
                light,
                glm::vec3(1.0f),
                UVRegion(
                    (lx + x) * scale + timer * hspeed,
                    timer + y * scale + (z + lz) * k,
                    (lx + x + 1) * scale + timer * hspeed,
                    timer + (size.y + y) * scale + (z + lz) * k
                )
            );
        }
    }
    for (int lz = -radius; lz <= radius; lz++) {
        for (int lx = -depth; lx < 0; lx++) {
            glm::vec4 light = light_at(chunks, x + lx, y, z + lz);
            random.setSeed(lx + x, lz + z);
            float hspeed = (random.randFloat() * 2.0f - 1.0f) * horizontal;
            glm::vec3 pos {
                x + lx + 0.5f,
                glm::max(y - size.y / 2, getHeightAt(x + lx, z + lz)) + size.y / 2 + 1,
                z + lz + 0.5f};
            batch->quad(
                pos,
                {0, 0, -1},
                {0, 1, 0},
                size,
                light,
                glm::vec3(1.0f),
                UVRegion(
                    (lz + z) * scale + timer * hspeed,
                    timer + y * scale + (x + lx) * k,
                    (lz + z + 1) * scale + timer * hspeed,
                    timer + (size.y + y) * scale + (x + lx) * k
                )
            );
        }
    }
    for (int lz = -radius; lz <= radius; lz++) {
        for (int lx = depth; lx > 0; lx--) {
            glm::vec4 light = light_at(chunks, x + lx, y, z + lz);
            random.setSeed(lx + x, lz + z);
            float hspeed = (random.randFloat() * 2.0f - 1.0f) * horizontal;
            glm::vec3 pos {
                x + lx + 0.5f,
                glm::max(y - size.y / 2, getHeightAt(x + lx, z + lz)) + size.y / 2 + 1,
                z + lz + 0.5f};
            batch->quad(
                pos,
                {0, 0, 1},
                {0, 1, 0},
                size,
                light,
                glm::vec3(1.0f),
                UVRegion(
                    (lz + z) * scale + timer * hspeed,
                    timer + y * scale + (x + lx) * k,
                    (lz + z + 1) * scale + timer * hspeed,
                    timer + (size.y + y) * scale + (x + lx) * k
                )
            );
        }
    }
    batch->flush();
}
