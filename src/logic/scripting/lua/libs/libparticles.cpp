#include "api_lua.hpp"

#include "logic/scripting/scripting_hud.hpp"
#include "graphics/render/WorldRenderer.hpp"
#include "graphics/render/ParticlesRenderer.hpp"
#include "graphics/render/Emitter.hpp"
#include "assets/assets_util.hpp"
#include "engine.hpp"

using namespace scripting;

static int l_emit(lua::State* L) {
    EmitterOrigin origin;
    if (lua::istable(L, 1)) {
        origin = lua::tovec3(L, 1);
    } else {
        origin = static_cast<entityid_t>(lua::tointeger(L, 1));
    }
    int count = lua::tointeger(L, 2);
    auto preset = lua::tovalue(L, 3);
    auto extension = lua::tovalue(L, 4);
    
    ParticlesPreset particlesPreset {};
    particlesPreset.deserialize(preset);
    if (extension != nullptr) {
        particlesPreset.deserialize(extension);
    }
    auto& assets = *engine->getAssets();
    auto region = util::get_texture_region(assets, particlesPreset.texture, "");
    auto emitter = std::make_unique<Emitter>(
        *level,
        std::move(origin),
        std::move(particlesPreset),
        region.texture,
        region.region,
        count
    );
    return lua::pushinteger(L, renderer->particles->add(std::move(emitter)));
}

static int l_stop(lua::State* L) {
    u64id_t id = lua::touinteger(L, 1);
    if (auto emitter = renderer->particles->getEmitter(id)) {
        emitter->stop();
    }
    return 0;
}

const luaL_Reg particleslib[] = {
    {"emit", lua::wrap<l_emit>},
    {"stop", lua::wrap<l_stop>},
    {NULL, NULL}
};