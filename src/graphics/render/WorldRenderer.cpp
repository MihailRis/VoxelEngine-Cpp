#include "WorldRenderer.hpp"

#include <GL/glew.h>
#include <assert.h>

#include <algorithm>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

#include "assets/Assets.hpp"
#include "content/Content.hpp"
#include "engine.hpp"
#include "frontend/LevelFrontend.hpp"
#include "items/Inventory.hpp"
#include "items/ItemDef.hpp"
#include "items/ItemStack.hpp"
#include "logic/PlayerController.hpp"
#include "logic/scripting/scripting_hud.hpp"
#include "maths/FrustumCulling.hpp"
#include "maths/voxmaths.hpp"
#include "objects/Entities.hpp"
#include "objects/Player.hpp"
#include "settings.hpp"
#include "voxels/Block.hpp"
#include "voxels/Chunk.hpp"
#include "voxels/Chunks.hpp"
#include "window/Camera.hpp"
#include "window/Window.hpp"
#include "world/Level.hpp"
#include "world/LevelEvents.hpp"
#include "world/World.hpp"
#include "graphics/core/Atlas.hpp"
#include "graphics/core/Batch3D.hpp"
#include "graphics/core/DrawContext.hpp"
#include "graphics/core/LineBatch.hpp"
#include "graphics/core/Mesh.hpp"
#include "graphics/core/Model.hpp"
#include "graphics/core/PostProcessing.hpp"
#include "graphics/core/Shader.hpp"
#include "graphics/core/Texture.hpp"
#include "ChunksRenderer.hpp"
#include "ModelBatch.hpp"
#include "Skybox.hpp"

bool WorldRenderer::showChunkBorders = false;
bool WorldRenderer::showEntitiesDebug = false;

WorldRenderer::WorldRenderer(
    Engine* engine, LevelFrontend* frontend, Player* player
)
    : engine(engine),
      level(frontend->getLevel()),
      player(player),
      frustumCulling(std::make_unique<Frustum>()),
      lineBatch(std::make_unique<LineBatch>()),
      modelBatch(std::make_unique<ModelBatch>(
          20'000, engine->getAssets(), level->chunks.get(), 
          &engine->getSettings()
      )) {
    renderer = std::make_unique<ChunksRenderer>(
        level, frontend->getContentGfxCache(), &engine->getSettings()
    );
    batch3d = std::make_unique<Batch3D>(4096);

    auto& settings = engine->getSettings();
    level->events->listen(
        EVT_CHUNK_HIDDEN,
        [this](lvl_event_type, Chunk* chunk) { renderer->unload(chunk); }
    );
    auto assets = engine->getAssets();
    skybox = std::make_unique<Skybox>(
        settings.graphics.skyboxResolution.get(),
        assets->get<Shader>("skybox_gen")
    );
}

WorldRenderer::~WorldRenderer() = default;

bool WorldRenderer::drawChunk(
    size_t index, const Camera& camera, Shader* shader, bool culling
) {
    auto chunk = level->chunks->getChunks()[index];
    if (!chunk->flags.lighted) {
        return false;
    }
    float distance = glm::distance(
        camera.position,
        glm::vec3(
            (chunk->x + 0.5f) * CHUNK_W,
            camera.position.y,
            (chunk->z + 0.5f) * CHUNK_D
        )
    );
    auto mesh = renderer->getOrRender(chunk, distance < CHUNK_W * 1.5f);
    if (mesh == nullptr) {
        return false;
    }
    if (culling) {
        glm::vec3 min(chunk->x * CHUNK_W, chunk->bottom, chunk->z * CHUNK_D);
        glm::vec3 max(
            chunk->x * CHUNK_W + CHUNK_W,
            chunk->top,
            chunk->z * CHUNK_D + CHUNK_D
        );

        if (!frustumCulling->isBoxVisible(min, max)) return false;
    }
    glm::vec3 coord(chunk->x * CHUNK_W + 0.5f, 0.5f, chunk->z * CHUNK_D + 0.5f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), coord);
    shader->uniformMatrix("u_model", model);
    mesh->draw();
    return true;
}

void WorldRenderer::drawChunks(
    Chunks* chunks, const Camera& camera, Shader* shader
) {
    auto assets = engine->getAssets();
    auto atlas = assets->get<Atlas>("blocks");

    atlas->getTexture()->bind();
    renderer->update();

    // [warning] this whole method is not thread-safe for chunks

    std::vector<size_t> indices;
    for (size_t i = 0; i < chunks->getVolume(); i++) {
        if (chunks->getChunks()[i] == nullptr) continue;
        indices.emplace_back(i);
    }
    float px = camera.position.x / static_cast<float>(CHUNK_W) - 0.5f;
    float pz = camera.position.z / static_cast<float>(CHUNK_D) - 0.5f;
    std::sort(indices.begin(), indices.end(), [chunks, px, pz](auto i, auto j) {
        const auto& chunksBuffer = chunks->getChunks();
        const auto a = chunksBuffer[i].get();
        const auto b = chunksBuffer[j].get();
        auto adx = (a->x - px);
        auto adz = (a->z - pz);
        auto bdx = (b->x - px);
        auto bdz = (b->z - pz);
        return (adx * adx + adz * adz > bdx * bdx + bdz * bdz);
    });
    bool culling = engine->getSettings().graphics.frustumCulling.get();
    if (culling) {
        frustumCulling->update(camera.getProjView());
    }
    chunks->visible = 0;
    for (size_t i = 0; i < indices.size(); i++) {
        chunks->visible += drawChunk(indices[i], camera, shader, culling);
    }
}

void WorldRenderer::setupWorldShader(
    Shader* shader,
    const Camera& camera,
    const EngineSettings& settings,
    float fogFactor
) {
    shader->use();
    shader->uniformMatrix("u_model", glm::mat4(1.0f));
    shader->uniformMatrix("u_proj", camera.getProjection());
    shader->uniformMatrix("u_view", camera.getView());
    shader->uniform1f("u_timer", timer);
    shader->uniform1f("u_gamma", settings.graphics.gamma.get());
    shader->uniform1f("u_fogFactor", fogFactor);
    shader->uniform1f("u_fogCurve", settings.graphics.fogCurve.get());
    shader->uniform1f("u_dayTime", level->getWorld()->getInfo().daytime);
    shader->uniform2f("u_lightDir", skybox->getLightDir());
    shader->uniform3f("u_cameraPos", camera.position);
    shader->uniform1i("u_cubemap", 1);

    auto indices = level->content->getIndices();
    // Light emission when an emissive item is chosen
    {
        auto inventory = player->getInventory();
        ItemStack& stack = inventory->getSlot(player->getChosenSlot());
        auto& item = indices->items.require(stack.getItemId());
        float multiplier = 0.5f;
        shader->uniform3f(
            "u_torchlightColor",
            item.emission[0] / 15.0f * multiplier,
            item.emission[1] / 15.0f * multiplier,
            item.emission[2] / 15.0f * multiplier
        );
        shader->uniform1f("u_torchlightDistance", 6.0f);
    }
}

void WorldRenderer::renderLevel(
    const DrawContext&,
    const Camera& camera,
    const EngineSettings& settings,
    float delta,
    bool pause
) {
    auto assets = engine->getAssets();

    bool culling = engine->getSettings().graphics.frustumCulling.get();
    float fogFactor = 15.0f / ((float)settings.chunks.loadDistance.get() - 2);

    auto entityShader = assets->get<Shader>("entity");
    setupWorldShader(entityShader, camera, settings, fogFactor);
    skybox->bind();

    level->entities->render(
        assets,
        *modelBatch,
        culling ? frustumCulling.get() : nullptr,
        delta,
        pause
    );
    modelBatch->render();

    auto shader = assets->get<Shader>("main");
    setupWorldShader(shader, camera, settings, fogFactor);

    drawChunks(level->chunks.get(), camera, shader);

    if (!pause) {
        scripting::on_frontend_render();
    }

    skybox->unbind();
}

void WorldRenderer::renderBlockSelection() {
    const auto& selection = player->selection;
    auto indices = level->content->getIndices();
    blockid_t id = selection.vox.id;
    auto& block = indices->blocks.require(id);
    const glm::ivec3 pos = player->selection.position;
    const glm::vec3 point = selection.hitPosition;
    const glm::vec3 norm = selection.normal;

    const std::vector<AABB>& hitboxes =
        block.rotatable ? block.rt.hitboxes[selection.vox.state.rotation]
                        : block.hitboxes;

    lineBatch->lineWidth(2.0f);
    for (auto& hitbox : hitboxes) {
        const glm::vec3 center = glm::vec3(pos) + hitbox.center();
        const glm::vec3 size = hitbox.size();
        lineBatch->box(
            center, size + glm::vec3(0.02), glm::vec4(0.f, 0.f, 0.f, 0.5f)
        );
        if (player->debug) {
            lineBatch->line(
                point, point + norm * 0.5f, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)
            );
        }
    }
    lineBatch->flush();
}

void WorldRenderer::renderLines(
    const Camera& camera, Shader* linesShader, const DrawContext& pctx
) {
    linesShader->use();
    linesShader->uniformMatrix("u_projview", camera.getProjView());
    if (player->selection.vox.id != BLOCK_VOID) {
        renderBlockSelection();
    }
    if (player->debug && showEntitiesDebug) {
        auto ctx = pctx.sub(lineBatch.get());
        bool culling = engine->getSettings().graphics.frustumCulling.get();
        level->entities->renderDebug(
            *lineBatch, culling ? frustumCulling.get() : nullptr, ctx
        );
    }
}

void WorldRenderer::renderDebugLines(
    const DrawContext& pctx, const Camera& camera, Shader* linesShader
) {
    DrawContext ctx = pctx.sub(lineBatch.get());
    const auto& viewport = ctx.getViewport();
    uint displayWidth = viewport.getWidth();
    uint displayHeight = viewport.getHeight();

    ctx.setDepthTest(true);

    linesShader->use();

    if (showChunkBorders) {
        linesShader->uniformMatrix("u_projview", camera.getProjView());
        glm::vec3 coord = player->camera->position;
        if (coord.x < 0) coord.x--;
        if (coord.z < 0) coord.z--;
        int cx = floordiv(static_cast<int>(coord.x), CHUNK_W);
        int cz = floordiv(static_cast<int>(coord.z), CHUNK_D);

        drawBorders(
            cx * CHUNK_W,
            0,
            cz * CHUNK_D,
            (cx + 1) * CHUNK_W,
            CHUNK_H,
            (cz + 1) * CHUNK_D
        );
    }

    float length = 40.f;
    glm::vec3 tsl(displayWidth / 2, displayHeight / 2, 0.f);
    glm::mat4 model(glm::translate(glm::mat4(1.f), tsl));
    linesShader->uniformMatrix(
        "u_projview",
        glm::ortho(
            0.f,
            static_cast<float>(displayWidth),
            0.f,
            static_cast<float>(displayHeight),
            -length,
            length
        ) * model *
            glm::inverse(camera.rotation)
    );

    ctx.setDepthTest(false);
    lineBatch->lineWidth(4.0f);
    lineBatch->line(0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f);
    lineBatch->line(0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 0.f, 1.f);
    lineBatch->line(0.f, 0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 1.f);
    lineBatch->flush();

    ctx.setDepthTest(true);
    lineBatch->lineWidth(2.0f);
    lineBatch->line(0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f);
    lineBatch->line(0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 0.f, 1.f);
    lineBatch->line(0.f, 0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 1.f);
}

void WorldRenderer::renderHands(const Camera& camera, const Assets& assets) {
    auto entityShader = assets.get<Shader>("entity");
    auto indices = level->content->getIndices();

    // get current chosen item
    const auto& inventory = player->getInventory();
    int slot = player->getChosenSlot();
    const ItemStack& stack = inventory->getSlot(slot);
    const auto& def = indices->items.require(stack.getItemId());

    // prepare modified HUD camera
    Camera hudcam = camera;
    hudcam.far = 100.0f;
    hudcam.setFov(1.2f);
    hudcam.position = {};

    // configure model matrix
    const glm::vec3 itemOffset(0.08f, 0.035f, -0.1);

    static glm::mat4 prevRotation(1.0f);

    const float speed = 24.0f;
    glm::mat4 matrix = glm::translate(glm::mat4(1.0f), itemOffset);
    matrix = glm::scale(matrix, glm::vec3(0.1f));
    glm::mat4 rotation = camera.rotation;
    glm::quat rot0 = glm::quat_cast(prevRotation);
    glm::quat rot1 = glm::quat_cast(rotation);
    glm::quat finalRot =
        glm::slerp(rot0, rot1, static_cast<float>(engine->getDelta() * speed));
    rotation = glm::mat4_cast(finalRot);
    matrix = rotation * matrix *
             glm::rotate(
                 glm::mat4(1.0f), -glm::pi<float>() * 0.5f, glm::vec3(0, 1, 0)
             );
    prevRotation = rotation;
    auto offset = -(camera.position - player->getPosition());
    float angle = glm::radians(player->cam.x - 90);
    float cos = glm::cos(angle);
    float sin = glm::sin(angle);

    float newX = offset.x * cos - offset.z * sin;
    float newZ = offset.x * sin + offset.z * cos;
    offset = glm::vec3(newX, offset.y, newZ);
    matrix = matrix * glm::translate(glm::mat4(1.0f), offset);

    // render
    texture_names_map map = {};
    modelBatch->setLightsOffset(camera.position);
    modelBatch->draw(
        matrix,
        glm::vec3(1.0f),
        assets.get<model::Model>(def.modelName),
        &map
    );
    Window::clearDepth();
    setupWorldShader(entityShader, hudcam, engine->getSettings(), 0.0f);
    skybox->bind();
    modelBatch->render();
    modelBatch->setLightsOffset(glm::vec3());
    skybox->unbind();
}

void WorldRenderer::draw(
    const DrawContext& pctx,
    Camera& camera,
    bool hudVisible,
    bool pause,
    float delta,
    PostProcessing* postProcessing
) {
    timer += delta * !pause;
    auto world = level->getWorld();
    const Viewport& vp = pctx.getViewport();
    camera.aspect = vp.getWidth() / static_cast<float>(vp.getHeight());

    const auto& settings = engine->getSettings();
    const auto& worldInfo = world->getInfo();

    skybox->refresh(pctx, worldInfo.daytime, 1.0f + worldInfo.fog * 2.0f, 4);

    const auto& assets = *engine->getAssets();
    auto linesShader = assets.get<Shader>("lines");

    // World render scope with diegetic HUD included
    {
        DrawContext wctx = pctx.sub();
        postProcessing->use(wctx);

        Window::clearDepth();

        // Drawing background sky plane
        skybox->draw(pctx, camera, assets, worldInfo.daytime, worldInfo.fog);

        // Actually world render with depth buffer on
        {
            DrawContext ctx = wctx.sub();
            ctx.setDepthTest(true);
            ctx.setCullFace(true);
            renderLevel(ctx, camera, settings, delta, pause);
            // Debug lines
            if (hudVisible) {
                renderLines(camera, linesShader, ctx);
                renderHands(camera, assets);
            }
        }
        if (hudVisible && player->debug) {
            renderDebugLines(wctx, camera, linesShader);
        }
        renderBlockOverlay(wctx, assets);
    }

    // Rendering fullscreen quad with
    auto screenShader = assets.get<Shader>("screen");
    screenShader->use();
    screenShader->uniform1f("u_timer", timer);
    screenShader->uniform1f("u_dayTime", worldInfo.daytime);
    postProcessing->render(pctx, screenShader);
}

void WorldRenderer::renderBlockOverlay(const DrawContext& wctx, const Assets& assets) {
    int x = std::floor(player->camera->position.x);
    int y = std::floor(player->camera->position.y);
    int z = std::floor(player->camera->position.z);
    auto block = level->chunks->get(x, y, z);
    if (block && block->id) {
        const auto& def =
            level->content->getIndices()->blocks.require(block->id);
        if (def.overlayTexture.empty()) {
            return;
        }
        DrawContext ctx = wctx.sub();
        ctx.setDepthTest(false);
        ctx.setCullFace(false);
        
        auto& shader = assets.require<Shader>("ui3d");
        auto& atlas = assets.require<Atlas>("blocks");
        shader.use();
        batch3d->begin();
        shader.uniformMatrix("u_projview", glm::mat4(1.0f));
        shader.uniformMatrix("u_apply", glm::mat4(1.0f));
        auto light = level->chunks->getLight(x, y, z);
        float s = Lightmap::extract(light, 3) / 15.0f;
        glm::vec4 tint(
            glm::min(1.0f, Lightmap::extract(light, 0) / 15.0f + s),
            glm::min(1.0f, Lightmap::extract(light, 1) / 15.0f + s),
            glm::min(1.0f, Lightmap::extract(light, 2) / 15.0f + s),
            1.0f
        );
        batch3d->texture(atlas.getTexture());
        batch3d->sprite(
            glm::vec3(),
            glm::vec3(0, 1, 0),
            glm::vec3(1, 0, 0),
            2,
            2,
            atlas.get(def.overlayTexture),
            tint
        );
        batch3d->flush();
    }
}

void WorldRenderer::drawBorders(
    int sx, int sy, int sz, int ex, int ey, int ez
) {
    int ww = ex - sx;
    int dd = ez - sz;
    /*corner*/ {
        lineBatch->line(sx, sy, sz, sx, ey, sz, 0.8f, 0, 0.8f, 1);
        lineBatch->line(sx, sy, ez, sx, ey, ez, 0.8f, 0, 0.8f, 1);
        lineBatch->line(ex, sy, sz, ex, ey, sz, 0.8f, 0, 0.8f, 1);
        lineBatch->line(ex, sy, ez, ex, ey, ez, 0.8f, 0, 0.8f, 1);
    }
    for (int i = 2; i < ww; i += 2) {
        lineBatch->line(sx + i, sy, sz, sx + i, ey, sz, 0, 0, 0.8f, 1);
        lineBatch->line(sx + i, sy, ez, sx + i, ey, ez, 0, 0, 0.8f, 1);
    }
    for (int i = 2; i < dd; i += 2) {
        lineBatch->line(sx, sy, sz + i, sx, ey, sz + i, 0.8f, 0, 0, 1);
        lineBatch->line(ex, sy, sz + i, ex, ey, sz + i, 0.8f, 0, 0, 1);
    }
    for (int i = sy; i < ey; i += 2) {
        lineBatch->line(sx, i, sz, sx, i, ez, 0, 0.8f, 0, 1);
        lineBatch->line(sx, i, ez, ex, i, ez, 0, 0.8f, 0, 1);
        lineBatch->line(ex, i, ez, ex, i, sz, 0, 0.8f, 0, 1);
        lineBatch->line(ex, i, sz, sx, i, sz, 0, 0.8f, 0, 1);
    }
    lineBatch->flush();
}

void WorldRenderer::clear() {
    renderer->clear();
}
