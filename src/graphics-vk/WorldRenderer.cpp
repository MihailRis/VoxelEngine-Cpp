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
#include "../world/LevelEvents.h"
#include "../voxels/Block.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../assets/Assets.h"
#include "../content/Content.h"
#include "../logic/PlayerController.h"
#include "../objects/Player.h"
#include "../maths/voxmaths.h"

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
        const glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(chunk->x * CHUNK_W, 0.0f, chunk->z * CHUNK_D + 1));

        auto &constant = m_constantses.at(m_constantIndex);
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

        level->events->listen(EVT_CHUNK_HIDDEN, [this](lvl_event_type type, Chunk* chunk) {
            m_renderer->unload(chunk);
        });
        const auto *assets = engine->getAssets();
        m_skybox = new Skybox(64, assets->getShader("skybox_gen"));
    }

    WorldRenderer::~WorldRenderer() {
        delete m_skybox;
        delete m_lineBatch;
        delete m_renderer;
        delete m_frustumCulling;
    }

    void WorldRenderer::draw(const GfxContext&context, Camera* camera) {
        m_constantses.clear();
        m_constantIndex = 0;
        const EngineSettings &settings = m_engine->getSettings();
        // skybox->refresh(m_level->world->daytime,
        // 				fmax(1.0f, 18.0f/settings.chunks.loadDistance), 4);

        const Content* content = m_level->content;
        const ContentIndices* contentIds = content->indices;
        Assets* assets = m_engine->getAssets();
        Atlas* atlas = assets->getAtlas("blocks");
        IShader* shader = assets->getShader("main");
        IShader* linesShader = assets->getShader("lines");

        const Viewport&viewport = context.getViewport();
        int displayWidth = viewport.getWidth();
        int displayHeight = viewport.getHeight();

        // IShader* backShader = assets->getShader("background");
        // backShader->use();
        // backShader->uniformMatrix("u_view", camera->getView(false));
        // backShader->uniform1f("u_zoom", camera->zoom);
        // backShader->uniform1f("u_ar", (float)Window::width / (float)Window::height);
        // skybox->draw(backShader);
        {
            GfxContext ctx = context.sub();
            ctx.depthTest(true);
            ctx.cullFace(true);
            const float fogFactor = 18.0f / static_cast<float>(settings.chunks.loadDistance);

            shader->use();
            m_skybox->bind();
            shader->uniformMatrix("u_proj", camera->getProjection());
            shader->uniformMatrix("u_view", camera->getView());
            shader->uniform1f("u_gamma", 1.0f);
            shader->uniform1f("u_fogFactor", fogFactor);
            shader->uniform1f("u_fogCurve", settings.graphics.fogCurve);
            shader->uniform3f("u_cameraPos", camera->position);

            Block* cblock = contentIds->getBlockDef(m_level->player->choosenBlock);
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
                    auto states = PlayerController::selectedBlockStates;
                    block->rotations.variants[states].transform(hitbox);
                }

                const glm::vec3 center = pos + hitbox.center();
                const glm::vec3 size = hitbox.size();
                linesShader->use();
                linesShader->uniformMatrix("u_projview", camera->getProjView());
                m_lineBatch->lineWidth(2.0f);
                m_lineBatch->box(center, size + glm::vec3(0.02), glm::vec4(0.f, 0.f, 0.f, 0.5f));
                if (m_level->player->debug)
                    m_lineBatch->line(point, point+norm*0.5f, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
                m_lineBatch->render();
            }
            //  skybox->unbind();
        }

        // if (m_level->player->debug) {
        //     GfxContext ctx = context.sub();
        //     ctx.depthTest(true);
        //
        //     linesShader->use();
        //     if (settings.debug.showChunkBorders) {
        //         linesShader->uniformMatrix("u_projview", camera->getProjView());
        //         glm::vec3 coord = m_level->player->camera->position;
        //         if (coord.x < 0) coord.x--;
        //         if (coord.z < 0) coord.z--;
        //         int cx = floordiv((int)coord.x, CHUNK_W);
        //         int cz = floordiv((int)coord.z, CHUNK_D);
        //         /*corner*/ {
        //             m_lineBatch->line(cx * CHUNK_W, 0, cz * CHUNK_D,
        //                               cx * CHUNK_W, CHUNK_H, cz * CHUNK_D, 0.8f, 0, 0.8f, 1);
        //             m_lineBatch->line(cx * CHUNK_W, 0, (cz + 1) * CHUNK_D,
        //                               cx * CHUNK_W, CHUNK_H, (cz + 1) * CHUNK_D, 0.8f, 0, 0.8f, 1);
        //             m_lineBatch->line((cx + 1) * CHUNK_W, 0, cz * CHUNK_D,
        //                               (cx + 1) * CHUNK_W, CHUNK_H, cz * CHUNK_D, 0.8f, 0, 0.8f, 1);
        //             m_lineBatch->line((cx + 1) * CHUNK_W, 0, (cz + 1) * CHUNK_D,
        //                               (cx + 1) * CHUNK_W, CHUNK_H, (cz + 1) * CHUNK_D, 0.8f, 0, 0.8f, 1);
        //         }
        //         for (int i = 2; i < CHUNK_W; i += 2) {
        //             m_lineBatch->line(cx * CHUNK_W + i, 0, cz * CHUNK_D,
        //                               cx * CHUNK_W + i, CHUNK_H, cz * CHUNK_D, 0, 0, 0.8f, 1);
        //             m_lineBatch->line(cx * CHUNK_W + i, 0, (cz + 1) * CHUNK_D,
        //                               cx * CHUNK_W + i, CHUNK_H, (cz + 1) * CHUNK_D, 0, 0, 0.8f, 1);
        //         }
        //         for (int i = 2; i < CHUNK_D; i += 2) {
        //             m_lineBatch->line(cx * CHUNK_W, 0, cz * CHUNK_D + i,
        //                               cx * CHUNK_W, CHUNK_H, cz * CHUNK_D + i, 0.8f, 0, 0, 1);
        //             m_lineBatch->line((cx + 1) * CHUNK_W, 0, cz * CHUNK_D + i,
        //                               (cx + 1) * CHUNK_W, CHUNK_H, cz * CHUNK_D + i, 0.8f, 0, 0, 1);
        //         }
        //         for (int i = 0; i < CHUNK_H; i += 2) {
        //             m_lineBatch->line(cx * CHUNK_W, i, cz * CHUNK_D,
        //                               cx * CHUNK_W, i, (cz + 1) * CHUNK_D, 0, 0.8f, 0, 1);
        //             m_lineBatch->line(cx * CHUNK_W, i, (cz + 1) * CHUNK_D,
        //                               (cx + 1) * CHUNK_W, i, (cz + 1) * CHUNK_D, 0, 0.8f, 0, 1);
        //             m_lineBatch->line((cx + 1) * CHUNK_W, i, (cz + 1) * CHUNK_D,
        //                               (cx + 1) * CHUNK_W, i, cz * CHUNK_D, 0, 0.8f, 0, 1);
        //             m_lineBatch->line((cx + 1) * CHUNK_W, i, cz * CHUNK_D,
        //                               cx * CHUNK_W, i, cz * CHUNK_D, 0, 0.8f, 0, 1);
        //         }
        //         m_lineBatch->render();
        //     }
        //
        //     float length = 40.f;
        //     // top-right: glm::vec3 tsl = glm::vec3(displayWidth - length - 4, -length - 4, 0.f);
        //     glm::vec3 tsl = glm::vec3(displayWidth / 2, displayHeight / 2, 0.f);
        //     glm::mat4 model(glm::translate(glm::mat4(1.f), tsl));
        //     linesShader->uniformMatrix("u_projview", glm::ortho(
        //                                                  0.f, (float)displayWidth,
        //                                                  0.f, (float)displayHeight,
        //                                                  -length, length) * model * glm::inverse(camera->rotation));
        //
        //     ctx.depthTest(false);
        //     m_lineBatch->lineWidth(4.0f);
        //     m_lineBatch->line(0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f);
        //     m_lineBatch->line(0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 0.f, 1.f);
        //     m_lineBatch->line(0.f, 0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 1.f);
        //     m_lineBatch->render();
        //
        //     ctx.depthTest(true);
        //     m_lineBatch->lineWidth(2.0f);
        //     m_lineBatch->line(0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f);
        //     m_lineBatch->line(0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 0.f, 1.f);
        //     m_lineBatch->line(0.f, 0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 1.f);
        //     m_lineBatch->render();
        // }
    }

    void WorldRenderer::drawDebug(const GfxContext&context, Camera* camera) {
    }
} // vulkan
