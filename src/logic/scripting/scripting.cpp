#include "scripting.h"

#include <iostream>
#include <stdexcept>

#include "../../content/ContentPack.h"
#include "../../files/engine_paths.h"
#include "../../files/files.h"
#include "../../util/timeutil.h"
#include "../../world/Level.h"
#include "../../voxels/Block.h"
#include "../../items/ItemDef.h"
#include "../../logic/BlocksController.h"
#include "../../engine.h"
#include "LuaState.h"
#include "../../util/stringutil.h"

using namespace scripting;

namespace scripting {
    extern lua::LuaState* state;
}

Engine* scripting::engine = nullptr;
lua::LuaState* scripting::state = nullptr;
Level* scripting::level = nullptr;
const Content* scripting::content = nullptr;
BlocksController* scripting::blocks = nullptr;

void load_script(fs::path name) {
    auto paths = scripting::engine->getPaths();
    fs::path file = paths->getResources()/fs::path("scripts")/name;

    std::string src = files::read_string(file);
    state->execute(src, file.u8string());
}

void scripting::initialize(Engine* engine) {
    scripting::engine = engine;

    state = new lua::LuaState();

    load_script(fs::path("stdlib.lua"));
}

// todo: luaL state check
runnable scripting::create_runnable(
    const std::string& file,
    const std::string& src
) {
    return [=](){
        state->execute(src, file);
    };
}

wstringconsumer scripting::create_wstring_consumer(
    const std::string& src,
    const std::string& file
) {
    try {
        if (state->eval(src, file) == 0)
            return [](const std::wstring& _) {};
    } catch (const lua::luaerror& err) {
        std::cerr << err.what() << std::endl;
        return [](const std::wstring& _) {};
    }

    auto funcName = state->storeAnonymous();
    return [=](const std::wstring& x){
        if (state->getglobal(funcName)) {
            state->pushstring(util::wstr2str_utf8(x));
            state->callNoThrow(1);
        }
    };
}

void scripting::on_world_load(Level* level, BlocksController* blocks) {
    scripting::level = level;
    scripting::content = level->content;
    scripting::blocks = blocks;
    load_script("world.lua");

    for (auto& pack : scripting::engine->getContentPacks()) {
        if (state->getglobal(pack.id+".worldopen")) {
            state->callNoThrow(0);
        }
    }
}

void scripting::on_world_save() {
    for (auto& pack : scripting::engine->getContentPacks()) {
        if (state->getglobal(pack.id+".worldsave")) {
            state->callNoThrow(0);
        }
    }
}

void scripting::on_world_quit() {
    for (auto& pack : scripting::engine->getContentPacks()) {
        if (state->getglobal(pack.id+".worldquit")) {
            state->callNoThrow(0);
        }
    }
    scripting::level = nullptr;
    scripting::content = nullptr;
}

void scripting::on_blocks_tick(const Block* block, int tps) {
    std::string name = block->name+".blockstick";
    if (state->getglobal(name)) {
        state->pushinteger(tps);
        state->callNoThrow(1);
    }
}

void scripting::update_block(const Block* block, int x, int y, int z) {
    std::string name = block->name+".update";
    if (state->getglobal(name)) {
        state->pushivec3(x, y, z);
        state->callNoThrow(3);
    }
}

void scripting::random_update_block(const Block* block, int x, int y, int z) {
    std::string name = block->name+".randupdate";
    if (state->getglobal(name)) {
        state->pushivec3(x, y, z);
        state->callNoThrow(3);
    }
}

void scripting::on_block_placed(Player* player, const Block* block, int x, int y, int z) {
    std::string name = block->name+".placed";
    if (state->getglobal(name)) {
        state->pushivec3(x, y, z);
        state->pushinteger(1); // playerid placeholder
        state->callNoThrow(4);
    }
}

void scripting::on_block_broken(Player* player, const Block* block, int x, int y, int z) {
    std::string name = block->name+".broken";
    if (state->getglobal(name)) {
        state->pushivec3(x, y, z);
        state->pushinteger(1); // playerid placeholder
        state->callNoThrow(4);
    }
}

bool scripting::on_block_interact(Player* player, const Block* block, int x, int y, int z) {
    std::string name = block->name+".oninteract";
    if (state->getglobal(name)) {
        state->pushivec3(x, y, z);
        state->pushinteger(1); // playerid placeholder
        if (state->callNoThrow(4)) {
            return state->toboolean(-1);
        }
    }
    return false;
}

bool scripting::on_item_use_on_block(Player* player, const ItemDef* item, int x, int y, int z) {
    std::string name = item->name+".useon";
    if (state->getglobal(name)) {
        state->pushivec3(x, y, z);
        state->pushinteger(1); // playerid placeholder
        if (state->callNoThrow(4)) {
            return state->toboolean(-1);
        }
    }
    return false;
}

bool scripting::on_item_break_block(Player* player, const ItemDef* item, int x, int y, int z) {
    std::string name = item->name+".blockbreakby";
    if (state->getglobal(name)) {
        state->pushivec3(x, y, z);
        state->pushinteger(1); // playerid placeholder
        if (state->callNoThrow(4)) {
            return state->toboolean(-1);
        }
    }
    return false;
}

void scripting::load_block_script(std::string prefix, fs::path file, block_funcs_set* funcsset) {
    std::string src = files::read_string(file);
    std::cout << "loading script " << file.u8string() << std::endl;
    state->execute(src, file.u8string());

    funcsset->init=state->rename("init", prefix+".init");
    funcsset->update=state->rename("on_update", prefix+".update");
    funcsset->randupdate=state->rename("on_random_update", prefix+".randupdate");
    funcsset->onbroken=state->rename("on_broken", prefix+".broken");
    funcsset->onplaced=state->rename("on_placed", prefix+".placed");
    funcsset->oninteract=state->rename("on_interact", prefix+".oninteract");
    funcsset->onblockstick=state->rename("on_blocks_tick", prefix+".blockstick");
}

void scripting::load_item_script(std::string prefix, fs::path file, item_funcs_set* funcsset) {
    std::string src = files::read_string(file);
    std::cout << "loading script " << file.u8string() << std::endl;
    state->execute(src, file.u8string());
    funcsset->init=state->rename("init", prefix+".init");
    funcsset->on_use_on_block=state->rename("on_use_on_block", prefix+".useon");
    funcsset->on_block_break_by=state->rename("on_block_break_by", prefix+".blockbreakby");
}

void scripting::load_world_script(std::string prefix, fs::path file) {
    std::string src = files::read_string(file);
    std::cout << "loading script " << file.u8string() << std::endl;

    state->loadbuffer(src, file.u8string());
    state->callNoThrow(0);

    state->rename("init", prefix+".init");
    state->rename("on_world_open", prefix+".worldopen");
    state->rename("on_world_save", prefix+".worldsave");
    state->rename("on_world_quit", prefix+".worldquit");
}

void scripting::close() {
    delete state;

    state = nullptr;
    content = nullptr;
    level = nullptr;
}
