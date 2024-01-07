//
// Created by chelovek on 11/29/23.
//

#include "WorldRenderer.h"

#include "LineBatch.h"
#include "ChunksRenderer.h"
#include "Skybox.h"
#include "../engine.h"
#include "../settings.h"
#include "../graphics-common/IShader.h"
#include "../graphics-common/ITexture.h"
#include "../graphics/GfxContext.h"
#include "../graphics/Atlas.h"
#include "../maths/FrustumCulling.h"
#include "../window/Window.h"
#include "../window/Camera.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../world/LevelEvents.h"
#include "../voxels/Block.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../assets/Assets.h"
#include "../content/Content.h"
#include "../logic/PlayerController.h"
#include "../objects/Player.h"
#include "../maths/voxmaths.h"
#include "uniforms/ApplyUniform.h"
#include "uniforms/BackgroundUniform.h"
#include "uniforms/FogUniform.h"
#include "uniforms/ProjectionViewUniform.h"
#include "uniforms/StateUniform.h"

namespace vulkan {
    bool WorldRenderer::drawChunk(size_t index, Camera* camera, IShader* shader, bool culling) {
        std::shared_ptr<Chunk> chunk = m_level->chunks->chunks[index];
        if (!chunk->isLighted())
            return false;
        const std::shared_ptr<Mesh<Vertex3D>> mesh = m_renderer->getOrRender(chunk.get());
        if (mesh == nullptr)
            return false;

        // Simple frustum culling
        if (culling) {
            glm::vec3 min(chunk->x * CHUNK_W, chunk->bottom, chunk->z * CHUNK_D);
            glm::vec3 max(chunk->x * CHUNK_W + CHUNK_W, chunk->top, chunk->z * CHUNK_D + CHUNK_D);

            if (!m_frustumCulling->IsBoxVisible(min, max)) return false;
        }
        const glm::vec3 coord(chunk->x*CHUNK_W+0.5f, 0.5f, chunk->z*CHUNK_D+0.5f);
        const glm::mat4 model = glm::translate(glm::mat4(1.0f), coord);

        DynamicConstants &constant = m_constantses.at(m_constantIndex);
        constant.model = model;

        const glm::vec3 torchLightColor = constant.torchlightColor;
        const float torchLightDistance = constant.torchlightDistance;

        shader->pushConstant(constant);

        mesh->bind();
        mesh->draw({0, 0});

        m_constantses.emplace_back(DynamicConstants{
            glm::mat4(1.0),
            torchLightColor,
            torchLightDistance
        });

        ++m_constantIndex;

        return true;
    }

    void WorldRenderer::drawChunks(Chunks* chunks, Camera* camera, IShader* shader) {
        std::vector<size_t> indices;
        for (size_t i = 0; i < chunks->volume; i++) {
            std::shared_ptr<Chunk> chunk = chunks->chunks[i];
            if (chunk == nullptr)
                continue;
            indices.push_back(i);
        }

        float px = camera->position.x / (float)CHUNK_W;
        float pz = camera->position.z / (float)CHUNK_D;
        std::sort(indices.begin(), indices.end(), [this, chunks, px, pz](size_t i, size_t j) {
            std::shared_ptr<Chunk> a = chunks->chunks[i];
            std::shared_ptr<Chunk> b = chunks->chunks[j];
            return ((a->x + 0.5f - px) * (a->x + 0.5f - px) + (a->z + 0.5f - pz) * (a->z + 0.5f - pz) >
                    (b->x + 0.5f - px) * (b->x + 0.5f - px) + (b->z + 0.5f - pz) * (b->z + 0.5f - pz));
        });

        bool culling = m_engine->getSettings().graphics.frustumCulling;

        if (culling) {
            m_frustumCulling->update(camera->getProjView());
        }
        chunks->visible = 0;
        for (const auto &indice : indices) {
            chunks->visible += drawChunk(indice, camera, shader, culling);
        }
    }

    WorldRenderer::WorldRenderer(Engine* engine, Level* level, const ContentGfxCache* cache)
        : m_engine(engine),
          m_level(level),
          m_frustumCulling(new Frustum()),
          m_lineBatch(new LineBatch(4096)),
          m_renderer(new ChunksRenderer(level, cache, engine->getSettings())){

        const EngineSettings& settings = engine->getSettings();
        level->events->listen(EVT_CHUNK_HIDDEN, [this](lvl_event_type type, Chunk* chunk) {
            m_renderer->unload(chunk);
        });
        const Assets *assets = engine->getAssets();
        m_skybox = new Skybox(settings.graphics.skyboxResolution, assets->getShader("skybox_gen"));
    }

    WorldRenderer::~WorldRenderer() {
        delete m_skybox;
        delete m_lineBatch;
        delete m_renderer;
        delete m_frustumCulling;
    }

    void WorldRenderer::draw(const GfxContext&context, Camera* camera) {
        auto &vkContext = VulkanContext::get();
        m_constantses.clear();
        m_constantIndex = 0;
        const EngineSettings &settings = m_engine->getSettings();
        m_skybox->refresh(m_level->world->daytime,
                    fmaxf(1.0f, 10.0f / (static_cast<float>(settings.chunks.loadDistance) - 2)) + fog * 2.0f, 4);

        const Content* content = m_level->content;
        const ContentIndices* contentIds = content->indices;
        Assets* assets = m_engine->getAssets();
        Atlas* atlas = assets->getAtlas("blocks");
        IShader* shader = assets->getShader("main");
        IShader* linesShader = assets->getShader("lines");

        const Viewport &viewport = context.getViewport();
        u32 displayWidth = viewport.getWidth();
        u32 displayHeight = viewport.getHeight();

        VkCommandBuffer commandBuffer = vkContext.immediateBeginDraw(0, 0, 0, VK_ATTACHMENT_LOAD_OP_CLEAR);

        IShader* backShader = assets->getShader("background");
        backShader->use(commandBuffer, vkContext.getSwapchain().getExtent());

        BackgroundUniform backgroundUniform = {
            camera->getView(false),
            camera->zoom,
            static_cast<float>(displayWidth) / static_cast<float>(displayHeight)
        };
        backShader->uniform(backgroundUniform);
        m_skybox->draw(backShader, commandBuffer);

        vkContext.immediateEndDraw(commandBuffer);

        {
            GfxContext ctx = context.sub();
            ctx.depthTest(true);
            ctx.cullFace(true);
            const float fogFactor = 18.0f / static_cast<float>(settings.chunks.loadDistance);

            shader->use();
            m_skybox->bind();

            StateUniform stateUniform = {
                camera->getProjection(),
                camera->getView(),
                camera->position,
                1.0f
            };

            FogUniform fogUniform = {
                fogFactor,
                settings.graphics.fogCurve
            };

            shader->uniform(stateUniform);
            shader->uniform(fogUniform);

            Block* cblock = contentIds->getBlockDef(m_level->player->chosenBlock);
            assert(cblock != nullptr);
            float multiplier = 0.5f;
            const glm::vec3 torchlightColor{
                static_cast<float>(cblock->emission[0]) / 15.0f * multiplier,
                static_cast<float>(cblock->emission[1]) / 15.0f * multiplier,
                static_cast<float>(cblock->emission[2]) / 15.0f * multiplier
            };

            m_constantses.emplace_back(DynamicConstants{
                glm::mat4(1.0),
                torchlightColor,
                6.0f
            });

            atlas->getTexture()->bind();

            Chunks* chunks = m_level->chunks;
            drawChunks(chunks, camera, shader);

            if (PlayerController::selectedBlockId != -1){
                blockid_t id = PlayerController::selectedBlockId;
                Block* block = contentIds->getBlockDef(id);
                assert(block != nullptr);
                const glm::vec3 pos = PlayerController::selectedBlockPosition;
                const glm::vec3 point = PlayerController::selectedPointPosition;
                const glm::vec3 norm = PlayerController::selectedBlockNormal;
                AABB hitbox = block->hitbox;
                if (block->rotatable) {
                    int states = PlayerController::selectedBlockStates;
                    block->rotations.variants[states].transform(hitbox);
                }

                const glm::vec3 center = pos + hitbox.center();
                const glm::vec3 size = hitbox.size();
                const ProjectionViewUniform projectionViewUniform{ camera->getProjView() };
                linesShader->use();
                linesShader->uniform(projectionViewUniform);
                m_lineBatch->lineWidth(2.0f);
                m_lineBatch->begin();
                m_lineBatch->box(center, size + glm::vec3(0.02), glm::vec4(0.f, 0.f, 0.f, 0.5f));
                if (m_level->player->debug)
                    m_lineBatch->line(point, point+norm*0.5f, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
                m_lineBatch->render();
            }
        }

        if (m_level->player->debug) {
            GfxContext ctx = context.sub();
            ctx.depthTest(true);

            linesShader->use();

            if (settings.debug.showChunkBorders){
                const ProjectionViewUniform projectionViewUniform{camera->getProjView()};
                linesShader->uniform(projectionViewUniform);
                glm::vec3 coord = m_level->player->camera->position;
                if (coord.x < 0) coord.x--;
                if (coord.z < 0) coord.z--;
                int cx = floordiv(static_cast<int>(coord.x), CHUNK_W);
                int cz = floordiv(static_cast<int>(coord.z), CHUNK_D);

                drawBorders(cx * CHUNK_W, 0, cz * CHUNK_D,
                            (cx + 1) * CHUNK_W, CHUNK_H, (cz + 1) * CHUNK_D);
            }

            float length = 40.f;
            glm::vec3 tsl(displayWidth/2, displayHeight/2, -length * 0.5f);
            glm::mat4 model(glm::translate(glm::mat4(1.f), tsl));
            const ProjectionViewUniform projectionViewUniform{
                glm::ortho(
                    0.f, static_cast<float>(displayWidth),
                    0.f, static_cast<float>(displayHeight),
                    -length, length) * model * glm::inverse(camera->rotation)
            };
            linesShader->uniform(projectionViewUniform);

            m_lineBatch->begin();

            m_lineBatch->lineWidth(4.0f);
            m_lineBatch->line(0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f);
            m_lineBatch->line(0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 0.f, 1.f);
            m_lineBatch->line(0.f, 0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 1.f);
            m_lineBatch->render();

        }

        m_lineBatch->end();
    }

    void WorldRenderer::drawDebug(const GfxContext&context, Camera* camera) {
    }

    void WorldRenderer::drawBorders(int sx, int sy, int sz, int ex, int ey, int ez) {
        int ww = ex-sx;
        int dd = ez-sz;
        m_lineBatch->lineWidth(1.0f);
        m_lineBatch->begin();
        /*corner*/ {
            m_lineBatch->line(sx, sy, sz,
                            sx, ey, sz, 0.8f, 0, 0.8f, 1);
            m_lineBatch->line(sx, sy, ez,
                            sx, ey, ez, 0.8f, 0, 0.8f, 1);
            m_lineBatch->line(ex, sy, sz,
                            ex, ey, sz, 0.8f, 0, 0.8f, 1);
            m_lineBatch->line(ex, sy, ez,
                            ex, ey, ez, 0.8f, 0, 0.8f, 1);
        }
        for (int i = 2; i < ww; i+=2) {
            m_lineBatch->line(sx + i, sy, sz,
                            sx + i, ey, sz, 0, 0, 0.8f, 1);
            m_lineBatch->line(sx + i, sy, ez,
                            sx + i, ey, ez, 0, 0, 0.8f, 1);
        }
        for (int i = 2; i < dd; i+=2) {
            m_lineBatch->line(sx, sy, sz + i,
                            sx, ey, sz + i, 0.8f, 0, 0, 1);
            m_lineBatch->line(ex, sy, sz + i,
                            ex, ey, sz + i, 0.8f, 0, 0, 1);
        }
        for (int i = sy; i < ey; i+=2){
            m_lineBatch->line(sx, i, sz,
                            sx, i, ez, 0, 0.8f, 0, 1);
            m_lineBatch->line(sx, i, ez,
                            ex, i, ez, 0, 0.8f, 0, 1);
            m_lineBatch->line(ex, i, ez,
                            ex, i, sz, 0, 0.8f, 0, 1);
            m_lineBatch->line(ex, i, sz,
                            sx, i, sz, 0, 0.8f, 0, 1);
        }
        m_lineBatch->render();
    }

    float WorldRenderer::fog = 0.0f;
} // vulkan
