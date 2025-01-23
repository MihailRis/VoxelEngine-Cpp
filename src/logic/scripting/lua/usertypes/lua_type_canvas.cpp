#include <unordered_map>

#include "graphics/core/ImageData.hpp"
#include "graphics/core/Texture.hpp"
#include "logic/scripting/lua/lua_custom_types.hpp"
#include "logic/scripting/lua/lua_util.hpp"

using namespace lua;

LuaCanvas::LuaCanvas(std::shared_ptr<Texture> inTexture)
    : mTexture(std::move(inTexture)) {
    mData = mTexture->readData();
}

union RGBA {
    uint8_t rgba[4];
    uint32_t raw;
};

static RGBA* get_at(const ImageData& data, uint index) {
    if (index >= data.getWidth() * data.getHeight()) {
        return nullptr;
    }
    return reinterpret_cast<RGBA*>(data.getData() + index * sizeof(RGBA));
}

static RGBA* get_at(const ImageData& data, uint x, uint y) {
    return get_at(data, y * data.getWidth() + x);
}

static RGBA* get_at(State* L, uint x, uint y) {
    if (auto texture = touserdata<LuaCanvas>(L, 1)) {
        return get_at(texture->data(), x, y);
    }
    return nullptr;
}

static int l_at(State* L) {
    auto x = static_cast<uint>(tointeger(L, 2));
    auto y = static_cast<uint>(tointeger(L, 3));

    if (auto pixel = get_at(L, x, y)) {
        return pushinteger(L, pixel->raw);
    }

    return 0;
}

static int l_set(State* L) {
    auto x = static_cast<uint>(tointeger(L, 2));
    auto y = static_cast<uint>(tointeger(L, 3));

    if (auto pixel = get_at(L, x, y)) {
        switch (gettop(L)) {
            case 4:
                pixel->raw = static_cast<uint>(tointeger(L, 4));
                return 1;
            case 6:
                pixel->rgba[0] = static_cast<ubyte>(tointeger(L, 4));
                pixel->rgba[1] = static_cast<ubyte>(tointeger(L, 5));
                pixel->rgba[2] = static_cast<ubyte>(tointeger(L, 6));
                pixel->rgba[3] = 255;
                return 1;
            case 7:
                pixel->rgba[0] = static_cast<ubyte>(tointeger(L, 4));
                pixel->rgba[1] = static_cast<ubyte>(tointeger(L, 5));
                pixel->rgba[2] = static_cast<ubyte>(tointeger(L, 6));
                pixel->rgba[3] = static_cast<ubyte>(tointeger(L, 7));
                return 1;
            default:
                return 0;
        }
    }

    return 0;
}

static int l_update(State* L) {
    if (auto texture = touserdata<LuaCanvas>(L, 1)) {
        texture->texture().reload(texture->data());
    }
    return 0;
}

static std::unordered_map<std::string, lua_CFunction> methods {
    {"at", lua::wrap<l_at>},
    {"set", lua::wrap<l_set>},
    {"update", lua::wrap<l_update>}
};

static int l_meta_index(State* L) {
    auto texture = touserdata<LuaCanvas>(L, 1);
    if (texture == nullptr) {
        return 0;
    }
    auto& data = texture->data();
    if (isnumber(L, 2)) {
        if (auto pixel = get_at(data, static_cast<uint>(tointeger(L, 2)))) {
            return pushinteger(L, pixel->raw);
        }
    }
    if (isstring(L, 2)) {
        auto name = tostring(L, 2);
        if (!strcmp(name, "width")) {
            return pushinteger(L, data.getWidth());
        }
        if (!strcmp(name, "height")) {
            return pushinteger(L, data.getHeight());
        }
        if (auto func = methods.find(tostring(L, 2)); func != methods.end()) {
            return pushcfunction(L, func->second);
        }
    }
    return 0;
}

static int l_meta_newindex(State* L) {
    auto texture = touserdata<LuaCanvas>(L, 1);
    if (texture == nullptr) {
        return 0;
    }
    auto& data = texture->data();
    if (isnumber(L, 2) && isnumber(L, 3)) {
        if (auto pixel = get_at(data, static_cast<uint>(tointeger(L, 2)))) {
            pixel->raw = static_cast<uint>(tointeger(L, 3));
            return 1;
        }
        return 1;
    }
    return 0;
}

int LuaCanvas::createMetatable(State* L) {
    createtable(L, 0, 3);
    pushcfunction(L, lua::wrap<l_meta_index>);
    setfield(L, "__index");
    pushcfunction(L, lua::wrap<l_meta_newindex>);
    setfield(L, "__newindex");
    return 1;
}
