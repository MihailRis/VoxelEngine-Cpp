#include "WorldRenderer.hpp"

#include "ChunksRenderer.hpp"
#include "Skybox.hpp"

#include "../../assets/Assets.hpp"
#include "../../content/Content.hpp"
#include "../../engine.hpp"
#include "../../frontend/LevelFrontend.hpp"
#include "../../items/Inventory.hpp"
#include "../../items/ItemDef.hpp"
#include "../../items/ItemStack.hpp"
#include "../../logic/PlayerController.hpp"
#include "../../maths/FrustumCulling.hpp"
#include "../../maths/voxmaths.hpp"
#include "../../objects/Player.hpp"
#include "../../settings.hpp"
#include "../../voxels/Block.hpp"
#include "../../voxels/Chunk.hpp"
#include "../../voxels/Chunks.hpp"
#include "../../window/Camera.hpp"
#include "../../window/Window.hpp"
#include "../../world/Level.hpp"
#include "../../world/LevelEvents.hpp"
#include "../../world/World.hpp"
#include "../core/Atlas.hpp"
#include "../core/Batch3D.hpp"
#include "../core/DrawContext.hpp"
#include "../core/LineBatch.hpp"
#include "../core/Mesh.hpp"
#include "../core/PostProcessing.hpp"
#include "../core/Shader.hpp"
#include "../core/Texture.hpp"

#include <assert.h>
#include <GL/glew.h>
#include <algorithm>
#include <memory>

#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

bool WorldRenderer::showChunkBorders = false;

WorldRenderer::WorldRenderer(Engine* engine, LevelFrontend* frontend, Player* player) 
    : engine(engine), 
      level(frontend->getLevel()),
      player(player),
      frustumCulling(std::make_unique<Frustum>()),
      lineBatch(std::make_unique<LineBatch>())
{
    renderer = std::make_unique<ChunksRenderer>(
        level,
        frontend->getContentGfxCache(),
        &engine->getSettings()
    );
    batch3d = std::make_unique<Batch3D>(4096);

    auto& settings = engine->getSettings();
    level->events->listen(EVT_CHUNK_HIDDEN, 
        [this](lvl_event_type, Chunk* chunk) {
            renderer->unload(chunk);
        }
    );
    auto assets = engine->getAssets();
    skybox = std::make_unique<Skybox>(
        settings.graphics.skyboxResolution.get(), 
        assets->getShader("skybox_gen")
    );
}

WorldRenderer::~WorldRenderer() = default;

bool WorldRenderer::drawChunk(
    size_t index,
    Camera* camera, 
    Shader* shader, 
    bool culling
){
    auto chunk = level->chunks->chunks[index];
    if (!chunk->flags.lighted) {
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

    // [warning] this whole method is not thread-safe for chunks

    std::vector<size_t> indices;
    for (size_t i = 0; i < chunks->volume; i++){
        if (chunks->chunks[i] == nullptr)
            continue;
        indices.emplace_back(i);
    }
    float px = camera->position.x / static_cast<float>(CHUNK_W) - 0.5f;
    float pz = camera->position.z / static_cast<float>(CHUNK_D) - 0.5f;
    std::sort(indices.begin(), indices.end(), [chunks, px, pz](auto i, auto j) {
        const auto a = chunks->chunks[i].get();
        const auto b = chunks->chunks[j].get();
        auto adx = (a->x - px);
        auto adz = (a->z - pz);
        auto bdx = (b->x - px);
        auto bdz = (b->z - pz);
        return (adx*adx + adz*adz > bdx*bdx + bdz*bdz);
    });
    bool culling = engine->getSettings().graphics.frustumCulling.get();
    if (culling) {
        frustumCulling->update(camera->getProjView());
    }
    chunks->visible = 0;
    for (unsigned long long indice : indices){
        chunks->visible += drawChunk(indice, camera, shader, culling);
    }
}

void WorldRenderer::renderLevel(
    const DrawContext&,
    Camera* camera, 
    const EngineSettings& settings
) {
    Assets* assets = engine->getAssets();
    Atlas* atlas = assets->getAtlas("blocks");
    Shader* shader = assets->getShader("main");

    auto indices = level->content->getIndices();

    float fogFactor = 15.0f / ((float)settings.chunks.loadDistance.get()-2);

    // Setting up main shader
    shader->use();
    shader->uniformMatrix("u_proj", camera->getProjection());
    shader->uniformMatrix("u_view", camera->getView());
    shader->uniform1f("u_timer", Window::time());
    shader->uniform1f("u_gamma", settings.graphics.gamma.get());
    shader->uniform1f("u_fogFactor", fogFactor);
    shader->uniform1f("u_fogCurve", settings.graphics.fogCurve.get());
    shader->uniform1f("u_dayTime", level->getWorld()->daytime);
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

    skybox->unbind();
}

void WorldRenderer::renderBlockSelection(Camera* camera, Shader* linesShader) {
    auto indices = level->content->getIndices();
    blockid_t id = PlayerController::selectedBlockId;
    auto block = indices->getBlockDef(id);
    const glm::ivec3 pos = player->selectedBlockPosition;
    const glm::vec3 point = PlayerController::selectedPointPosition;
    const glm::vec3 norm = PlayerController::selectedBlockNormal;

    const std::vector<AABB>& hitboxes = block->rotatable
        ? block->rt.hitboxes[PlayerController::selectedBlockRotation]
        : block->hitboxes;

    linesShader->use();
    linesShader->uniformMatrix("u_projview", camera->getProjView());
    lineBatch->lineWidth(2.0f);
    for (auto& hitbox: hitboxes) {
        const glm::vec3 center = glm::vec3(pos) + hitbox.center();
        const glm::vec3 size = hitbox.size();
        lineBatch->box(center, size + glm::vec3(0.02), glm::vec4(0.f, 0.f, 0.f, 0.5f));
        if (player->debug) {
            lineBatch->line(point, point+norm*0.5f, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
        }
    }
    lineBatch->render();
}

void WorldRenderer::renderDebugLines(
    const DrawContext& pctx, 
    Camera* camera,
    Shader* linesShader
) {
    DrawContext ctx = pctx.sub();
    const auto& viewport = ctx.getViewport();
    uint displayWidth = viewport.getWidth();
    uint displayHeight = viewport.getHeight();
    
    ctx.setDepthTest(true);

    linesShader->use();

    if (showChunkBorders){
        linesShader->uniformMatrix("u_projview", camera->getProjView());
        glm::vec3 coord = player->camera->position;
        if (coord.x < 0) coord.x--;
        if (coord.z < 0) coord.z--;
        int cx = floordiv((int)coord.x, CHUNK_W);
        int cz = floordiv((int)coord.z, CHUNK_D);

        drawBorders(
            cx * CHUNK_W, 0, cz * CHUNK_D, 
            (cx + 1) * CHUNK_W, CHUNK_H, (cz + 1) * CHUNK_D
        );
    }

    float length = 40.f;
    glm::vec3 tsl(displayWidth/2, displayHeight/2, 0.f);
    glm::mat4 model(glm::translate(glm::mat4(1.f), tsl));
    linesShader->uniformMatrix("u_projview", glm::ortho(
        0.f, (float)displayWidth, 
        0.f, (float)displayHeight,
        -length, length) * model * glm::inverse(camera->rotation)
    );

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

void WorldRenderer::draw(
    const DrawContext& pctx, 
    Camera* camera, 
    bool hudVisible, 
    PostProcessing* postProcessing
){
    auto world = level->getWorld();
    const Viewport& vp = pctx.getViewport();
    camera->aspect = vp.getWidth() / static_cast<float>(vp.getHeight());

    const EngineSettings& settings = engine->getSettings();
    skybox->refresh(pctx, world->daytime, 1.0f+world->fog*2.0f, 4);

    Assets* assets = engine->getAssets();
    Shader* linesShader = assets->getShader("lines");
    
    // World render scope with diegetic HUD included
    {
        DrawContext wctx = pctx.sub();
        postProcessing->use(wctx);

        Window::clearDepth();

        // Drawing background sky plane
        skybox->draw(pctx, camera, assets, world->daytime, world->fog);
        
        // Actually world render with depth buffer on
        {
            DrawContext ctx = wctx.sub();
            ctx.setDepthTest(true);
            ctx.setCullFace(true);
            renderLevel(ctx, camera, settings);
            // Selected block
            if (PlayerController::selectedBlockId != -1 && hudVisible){
                renderBlockSelection(camera, linesShader);
            }
        }

        if (hudVisible && player->debug) {
            renderDebugLines(wctx, camera, linesShader);
        }
    }

    // Rendering fullscreen quad with 
    auto screenShader = assets->getShader("screen");
    screenShader->use();
    screenShader->uniform1f("u_timer", Window::time());
    screenShader->uniform1f("u_dayTime", level->getWorld()->daytime);
    postProcessing->render(pctx, screenShader);
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
