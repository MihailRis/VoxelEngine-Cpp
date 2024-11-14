#include "api_lua.hpp"

#include "logic/scripting/scripting_hud.hpp"
#include "graphics/render/WorldRenderer.hpp"
#include "graphics/render/TextsRenderer.hpp"
#include "graphics/render/TextNote.hpp"
#include "engine.hpp"

using namespace scripting;

static int l_spawn(lua::State* L) {
    auto position = lua::tovec3(L, 1);
    auto text = lua::require_wstring(L, 2);
    auto preset = lua::tovalue(L, 3);
    auto extension = lua::tovalue(L, 4);
    
    NotePreset notePreset {};
    notePreset.deserialize(preset);
    if (extension != nullptr) {
        notePreset.deserialize(extension);
    }
    auto note = std::make_unique<TextNote>(text, notePreset, position);
    return lua::pushinteger(L, renderer->texts->add(std::move(note)));
}

const luaL_Reg text3dlib[] = {
    {"spawn", lua::wrap<l_spawn>},
    {NULL, NULL}
};
