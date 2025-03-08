#include <unordered_map>

#include "graphics/core/ImageData.hpp"
#include "graphics/core/Texture.hpp"
#include "logic/scripting/lua/lua_custom_types.hpp"
#include "logic/scripting/lua/lua_util.hpp"

using namespace lua;

LuaCanvas::LuaCanvas(
    std::shared_ptr<Texture> inTexture, std::shared_ptr<ImageData> inData
)
    : mTexture(std::move(inTexture)), mData(std::move(inData)) {
}

union RGBA {
    struct {
        uint8_t r, g, b, a;
    };
    uint8_t arr[4];
    uint32_t rgba;
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
    if (auto canvas = touserdata<LuaCanvas>(L, 1)) {
        return get_at(canvas->data(), x, y);
    }
    return nullptr;
}

static int l_at(State* L) {
    auto x = static_cast<uint>(tointeger(L, 2));
    auto y = static_cast<uint>(tointeger(L, 3));

    if (auto pixel = get_at(L, x, y)) {
        return pushinteger(L, pixel->rgba);
    }

    return 0;
}

static RGBA get_rgba(State* L, int first) {
    RGBA rgba {};
    rgba.a = 255;
    switch (gettop(L) - first) {
        case 0:
            rgba.rgba = static_cast<uint>(tointeger(L, first));
            break;
        case 3:
            rgba.a = static_cast<ubyte>(tointeger(L, first + 3));
            [[fallthrough]];
        case 2:
            rgba.r = static_cast<ubyte>(tointeger(L, first));
            rgba.g = static_cast<ubyte>(tointeger(L, first + 1));
            rgba.b = static_cast<ubyte>(tointeger(L, first + 2));
            break;
    }
    return rgba;
}

static int l_set(State* L) {
    auto x = static_cast<uint>(tointeger(L, 2));
    auto y = static_cast<uint>(tointeger(L, 3));

    if (auto pixel = get_at(L, x, y)) {
        *pixel = get_rgba(L, 4);
    }
    return 0;
}

static int l_clear(State* L) {
    auto canvas = touserdata<LuaCanvas>(L, 1);
    if (canvas == nullptr) {
        throw std::runtime_error("used canvas.clear instead of canvas:clear");
    }
    auto& image = canvas->data();
    ubyte* data = image.getData();
    RGBA rgba {};
    if (gettop(L) == 1) {
        std::fill(data, data + image.getDataSize(), 0);
        return 0;
    }
    rgba = get_rgba(L, 2);
    size_t pixels = image.getWidth() * image.getHeight();
    const size_t channels = 4;
    for (size_t i = 0; i < pixels * channels; i++) {
        data[i] = rgba.arr[i % channels];
    }
    return 0;
}

static int l_line(State* L) {
    int x1 = tointeger(L, 2);
    int y1 = tointeger(L, 3);

    int x2 = tointeger(L, 4);
    int y2 = tointeger(L, 5);

    RGBA rgba = get_rgba(L, 6);
    if (auto canvas = touserdata<LuaCanvas>(L, 1)) {
        auto& image = canvas->data();
        image.drawLine(
            x1, y1, x2, y2, glm::ivec4 {rgba.r, rgba.g, rgba.b, rgba.a}
        );
    }
    return 0;
}

static int l_update(State* L) {
    if (auto canvas = touserdata<LuaCanvas>(L, 1)) {
        canvas->texture().reload(canvas->data());
    }
    return 0;
}

static std::unordered_map<std::string, lua_CFunction> methods {
    {"at", lua::wrap<l_at>},
    {"set", lua::wrap<l_set>},
    {"line", lua::wrap<l_line>},
    {"clear", lua::wrap<l_clear>},
    {"update", lua::wrap<l_update>}};

static int l_meta_index(State* L) {
    auto texture = touserdata<LuaCanvas>(L, 1);
    if (texture == nullptr) {
        return 0;
    }
    auto& data = texture->data();
    if (isnumber(L, 2)) {
        if (auto pixel = get_at(data, static_cast<uint>(tointeger(L, 2)))) {
            return pushinteger(L, pixel->rgba);
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
            pixel->rgba = static_cast<uint>(tointeger(L, 3));
        }
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
