#include "WorldRenderer.h"

#include <iostream>
#include <GL/glew.h>
#include <memory>
#include <assert.h>

#include "../window/Window.h"
#include "../window/Camera.h"
#include "../content/Content.h"
#include "../graphics/Mesh.h"
#include "../graphics/Atlas.h"
#include "../graphics/Shader.h"
#include "../graphics/Batch3D.h"
#include "../graphics/Texture.h"
#include "../graphics/LineBatch.h"
#include "../graphics/PostProcessing.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/Block.h"
#include "../world/World.h"
#include "../world/Level.h"
#include "../world/LevelEvents.h"
#include "../objects/Player.h"
#include "../assets/Assets.h"
#include "../logic/PlayerController.h"
#include "../maths/FrustumCulling.h"
#include "../maths/voxmaths.h"
#include "../settings.h"
#include "../engine.h"
#include "../items/ItemDef.h"
#include "../items/ItemStack.h"
#include "../items/Inventory.h"
#include "LevelFrontend.h"
#include "graphics/Skybox.h"
#include "graphics/ChunksRenderer.h"

WorldRenderer::WorldRenderer(Engine* engine, LevelFrontend* frontend, Player* player) 
    : engine(engine), 
      level(frontend->getLevel()),
      player(player)
{
    postProcessing = std::make_unique<PostProcessing>();
    frustumCulling = std::make_unique<Frustum>();
    lineBatch = std::make_unique<LineBatch>();
    renderer = std::make_unique<ChunksRenderer>(
        level,
        frontend->getContentGfxCache(),
        engine->getSettings()
    );
    batch3d = std::make_unique<Batch3D>(4096);

    auto& settings = engine->getSettings();
    level->events->listen(EVT_CHUNK_HIDDEN, 
        [this](lvl_event_type type, Chunk* chunk) {
            renderer->unload(chunk);
        }
    );
    auto assets = engine->getAssets();
    skybox = std::make_unique<Skybox>(
        settings.graphics.skyboxResolution, 
        assets->getShader("skybox_gen")
    );
}

WorldRenderer::~WorldRenderer() {
}

bool WorldRenderer::drawChunk(
    size_t index,
    Camera* camera, 
    Shader* shader, 
    bool culling
){
    auto chunk = level->chunks->chunks[index];
    if (!chunk->isLighted()) {
        return false;
    }
    float distance = glm::distance(
        camera->position, 
        glm::vec3((chunk->x + 0.5f) * CHUNK_W, 
                  camera->position.y, 
                  (chunk->z + 0.5f) * CHUNK_D)
    );
    auto mesh = renderer->getOrRender(chunk, distance < CHUNK_W*1.5f);
    if (mesh == nullptr) {
        return false;
    }
    if (culling){
        glm::vec3 min(
            chunk->x * CHUNK_W, 
            chunk->bottom, 
            chunk->z * CHUNK_D
        );
        glm::vec3 max(
            chunk->x * CHUNK_W + CHUNK_W, 
            chunk->top, 
            chunk->z * CHUNK_D + CHUNK_D
        );

        if (!frustumCulling->IsBoxVisible(min, max)) 
            return false;
    }
    glm::vec3 coord(chunk->x*CHUNK_W+0.5f, 0.5f, chunk->z*CHUNK_D+0.5f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), coord);
    shader->uniformMatrix("u_model", model);
    mesh->draw();
    return true;
}

void WorldRenderer::drawChunks(Chunks* chunks, Camera* camera, Shader* shader) {
    renderer->update();
    std::vector<size_t> indices;
    for (size_t i = 0; i < chunks->volume; i++){
        if (chunks->chunks[i] == nullptr)
            continue;
        indices.push_back(i);
    }
    float px = camera->position.x / (float)CHUNK_W;
    float pz = camera->position.z / (float)CHUNK_D;
    std::sort(indices.begin(), indices.end(), [chunks, px, pz](size_t i, size_t j) {
        auto a = chunks->chunks[i];
        auto b = chunks->chunks[j];
        return ((a->x + 0.5f - px)*(a->x + 0.5f - px) + 
                (a->z + 0.5f - pz)*(a->z + 0.5f - pz)
                >
                (b->x + 0.5f - px)*(b->x + 0.5f - px) + 
                (b->z + 0.5f - pz)*(b->z + 0.5f - pz));
    });

    bool culling = engine->getSettings().graphics.frustumCulling;
    if (culling) {
        frustumCulling->update(camera->getProjView());
    }
    chunks->visible = 0;
    for (size_t i = 0; i < indices.size(); i++){
        chunks->visible += drawChunk(indices[i], camera, shader, culling);
    }
}


void WorldRenderer::draw(const GfxContext& pctx, Camera* camera, bool hudVisible){
    EngineSettings& settings = engine->getSettings();
    skybox->refresh(pctx, level->world->daytime, 1.0f+fog*2.0f, 4);

    const Content* content = level->content;
    auto indices = content->getIndices();
    Assets* assets = engine->getAssets();
    Atlas* atlas = assets->getAtlas("blocks");
    Shader* shader = assets->getShader("main");
    Shader* linesShader = assets->getShader("lines");

    const Viewport& viewport = pctx.getViewport();
    int displayWidth = viewport.getWidth();
    int displayHeight = viewport.getHeight();
    
    // World render scope with diegetic HUD included
    {
        GfxContext wctx = pctx.sub();
        postProcessing->use(wctx);

        Window::clearDepth();

        // Drawing background sky plane
        skybox->draw(pctx, camera, assets, level->getWorld()->daytime, fog);
        
        // Actually world render with depth buffer on
        {
            GfxContext ctx = wctx.sub();
            ctx.setDepthTest(true);
            ctx.setCullFace(true);    

            float fogFactor = 15.0f / ((float)settings.chunks.loadDistance-2);

            // Setting up main shader
            shader->use();
            shader->uniformMatrix("u_proj", camera->getProjection());
            shader->uniformMatrix("u_view", camera->getView());
            shader->uniform1f("u_gamma", settings.graphics.gamma);
            shader->uniform1f("u_fogFactor", fogFactor);
            shader->uniform1f("u_fogCurve", settings.graphics.fogCurve);
            shader->uniform1f("u_dayTime", level->world->daytime);
            shader->uniform3f("u_cameraPos", camera->position);
            shader->uniform1i("u_cubemap", 1);

            // Light emission when an emissive item is chosen
            {
                auto inventory = player->getInventory();
                ItemStack& stack = inventory->getSlot(player->getChosenSlot());
                auto item = indices->getItemDef(stack.getItemId());
                float multiplier = 0.5f;
                shader->uniform3f("u_torchlightColor",  
                    item->emission[0] / 15.0f * multiplier,
                    item->emission[1] / 15.0f * multiplier,
                    item->emission[2] / 15.0f * multiplier
                );
                shader->uniform1f("u_torchlightDistance", 6.0f);
            }

            // Binding main shader textures
            skybox->bind();
            atlas->getTexture()->bind();

            drawChunks(level->chunks.get(), camera, shader);

            // Selected block
            if (PlayerController::selectedBlockId != -1 && hudVisible){
                blockid_t id = PlayerController::selectedBlockId;
                auto block = indices->getBlockDef(id);
                const glm::vec3 pos = PlayerController::selectedBlockPosition;
                const glm::vec3 point = PlayerController::selectedPointPosition;
                const glm::vec3 norm = PlayerController::selectedBlockNormal;

                std::vector<AABB>& hitboxes = block->rotatable
                    ? block->rt.hitboxes[PlayerController::selectedBlockStates]
                    : block->hitboxes;

                linesShader->use();
                linesShader->uniformMatrix("u_projview", camera->getProjView());
                lineBatch->lineWidth(2.0f);
                for (auto& hitbox: hitboxes) {
                    const glm::vec3 center = pos + hitbox.center();
                    const glm::vec3 size = hitbox.size();
                    lineBatch->box(center, size + glm::vec3(0.02), glm::vec4(0.f, 0.f, 0.f, 0.5f));
                    if (player->debug) {
                        lineBatch->line(point, point+norm*0.5f, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
                    }
                }
                lineBatch->render();
            }
            skybox->unbind();
        }

        if (hudVisible && player->debug) {
            GfxContext ctx = pctx.sub();
            ctx.setDepthTest(true);

            linesShader->use();

            if (settings.debug.showChunkBorders){
                linesShader->uniformMatrix("u_projview", camera->getProjView());
                glm::vec3 coord = player->camera->position;
                if (coord.x < 0) coord.x--;
                if (coord.z < 0) coord.z--;
                int cx = floordiv((int)coord.x, CHUNK_W);
                int cz = floordiv((int)coord.z, CHUNK_D);

                drawBorders(cx * CHUNK_W, 0, cz * CHUNK_D, 
                            (cx + 1) * CHUNK_W, CHUNK_H, (cz + 1) * CHUNK_D);
            }

            float length = 40.f;
            glm::vec3 tsl(displayWidth/2, displayHeight/2, 0.f);
            glm::mat4 model(glm::translate(glm::mat4(1.f), tsl));
            linesShader->uniformMatrix("u_projview", glm::ortho(
                    0.f, (float)displayWidth, 
                    0.f, (float)displayHeight,
                    -length, length) * model * glm::inverse(camera->rotation));

            ctx.setDepthTest(false);
            lineBatch->lineWidth(4.0f);
            lineBatch->line(0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f);
            lineBatch->line(0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 0.f, 1.f);
            lineBatch->line(0.f, 0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 1.f);
            lineBatch->render();

            ctx.setDepthTest(true);
            lineBatch->lineWidth(2.0f);
            lineBatch->line(0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f);
            lineBatch->line(0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 0.f, 1.f);
            lineBatch->line(0.f, 0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 1.f);
            lineBatch->render();
        }
    }

    {
        GfxContext ctx = pctx.sub();
        ctx.setDepthTest(false);
        
        auto shader = assets->getShader("screen");
        shader->use();
        shader->uniform1f("u_timer", Window::time());
        shader->uniform1f("u_dayTime", level->world->daytime);

        postProcessing->render(ctx, shader);
    }
}

void WorldRenderer::drawBorders(int sx, int sy, int sz, int ex, int ey, int ez) {
    int ww = ex-sx;
    int dd = ez-sz;
    /*corner*/ {
        lineBatch->line(sx, sy, sz,     
                        sx, ey, sz, 0.8f, 0, 0.8f, 1);
        lineBatch->line(sx, sy, ez,     
                        sx, ey, ez, 0.8f, 0, 0.8f, 1);
        lineBatch->line(ex, sy, sz,     
                        ex, ey, sz, 0.8f, 0, 0.8f, 1);
        lineBatch->line(ex, sy, ez,     
                        ex, ey, ez, 0.8f, 0, 0.8f, 1);
    }
    for (int i = 2; i < ww; i+=2) {
        lineBatch->line(sx + i, sy, sz,     
                        sx + i, ey, sz, 0, 0, 0.8f, 1);
        lineBatch->line(sx + i, sy, ez,     
                        sx + i, ey, ez, 0, 0, 0.8f, 1);
    }
    for (int i = 2; i < dd; i+=2) {
        lineBatch->line(sx, sy, sz + i, 
                        sx, ey, sz + i, 0.8f, 0, 0, 1);
        lineBatch->line(ex, sy, sz + i, 
                        ex, ey, sz + i, 0.8f, 0, 0, 1);
    }
    for (int i = sy; i < ey; i+=2){
        lineBatch->line(sx, i, sz,
                        sx, i, ez, 0, 0.8f, 0, 1);
        lineBatch->line(sx, i, ez,
                        ex, i, ez, 0, 0.8f, 0, 1);
        lineBatch->line(ex, i, ez,
                        ex, i, sz, 0, 0.8f, 0, 1);
        lineBatch->line(ex, i, sz,
                        sx, i, sz, 0, 0.8f, 0, 1);
    }
    lineBatch->render();
}

float WorldRenderer::fog = 0.0f;
