#include "scripting.h"

#include <iostream>
#include <stdexcept>

#include "../../content/ContentPack.h"
#include "../../files/engine_paths.h"
#include "../../files/files.h"
#include "../../util/timeutil.h"
#include "../../world/Level.h"
#include "../../objects/Player.h"
#include "../../voxels/Block.h"
#include "../../items/ItemDef.h"
#include "../../items/Inventory.h"
#include "../../logic/BlocksController.h"
#include "../../frontend/UiDocument.h"
#include "../../engine.h"
#include "lua/LuaState.h"
#include "../../util/stringutil.h"
#include "../../util/timeutil.h"

using namespace scripting;

namespace scripting {
    extern lua::LuaState* state;
}

Engine* scripting::engine = nullptr;
lua::LuaState* scripting::state = nullptr;
Level* scripting::level = nullptr;
const Content* scripting::content = nullptr;
const ContentIndices* scripting::indices = nullptr;
BlocksController* scripting::blocks = nullptr;

Environment::Environment(int env) : env(env) {
}

Environment::~Environment() {
    if (env) {
        state->removeEnvironment(env);
    }
}

int Environment::getId() const {
    return env;
}

void load_script(fs::path name) {
    auto paths = scripting::engine->getPaths();
    fs::path file = paths->getResources()/fs::path("scripts")/name;

    std::string src = files::read_string(file);
    state->execute(0, src, file.u8string());
}

void scripting::initialize(Engine* engine) {
    scripting::engine = engine;

    state = new lua::LuaState();

    load_script(fs::path("stdlib.lua"));
}

std::unique_ptr<Environment> scripting::create_environment(int parent) {
    return std::make_unique<Environment>(state->createEnvironment(parent));
}

std::unique_ptr<Environment> scripting::create_pack_environment(const ContentPack& pack) {
    int id = state->createEnvironment(0);
    state->pushenv(id);
    state->pushvalue(-1);
    state->setfield("PACK_ENV");
    state->pushstring(pack.id);
    state->setfield("PACK_ID");
    state->pop();
    return std::make_unique<Environment>(id);
}

std::unique_ptr<Environment> scripting::create_doc_environment(int parent, const std::string& name) {
    int id = state->createEnvironment(parent);
    state->pushenv(id);
    state->pushvalue(-1);
    state->setfield("DOC_ENV");
    state->pushstring(name.c_str());
    state->setfield("DOC_NAME");
    
    if (state->getglobal("Document")) {
        if (state->getfield("new")) {
            state->pushstring(name.c_str());
            if (state->callNoThrow(1)) {
                state->setfield("document", -3);
            }
        }
        state->pop();
    }
    state->pop();
    return std::make_unique<Environment>(id);
}

void scripting::process_post_runnables() {
    if (state->getglobal("__process_post_runnables")) {
        state->callNoThrow(0);
    }
}

void scripting::on_world_load(Level* level, BlocksController* blocks) {
    scripting::level = level;
    scripting::content = level->content;
    scripting::indices = level->content->getIndices();
    scripting::blocks = blocks;
    load_script("world.lua");

    for (auto& pack : scripting::engine->getContentPacks()) {
        emit_event(pack.id + ".worldopen");
    }
}

void scripting::on_world_tick() {
    for (auto& pack : scripting::engine->getContentPacks()) {
        emit_event(pack.id + ".worldtick");
    }
}

void scripting::on_world_save() {
    for (auto& pack : scripting::engine->getContentPacks()) {
        emit_event(pack.id + ".worldsave");
    }
}

void scripting::on_world_quit() {
    for (auto& pack : scripting::engine->getContentPacks()) {
        emit_event(pack.id + ".worldquit");
    }

    state->getglobal("pack");
    for (auto& pack : scripting::engine->getContentPacks()) {
        state->getfield("unload");
        state->pushstring(pack.id);
        state->callNoThrow(1);   
    }
    state->pop();
    
    if (state->getglobal("__scripts_cleanup")) {
        state->callNoThrow(0);
    }
    scripting::level = nullptr;
    scripting::content = nullptr;
    scripting::indices = nullptr;
}

void scripting::on_blocks_tick(const Block* block, int tps) {
    std::string name = block->name + ".blockstick";
    emit_event(name, [tps] (lua::LuaState* state) {
        state->pushinteger(tps);
        return 1;
    });
}

void scripting::update_block(const Block* block, int x, int y, int z) {
    std::string name = block->name + ".update";
    emit_event(name, [x, y, z] (lua::LuaState* state) {
        state->pushivec3(x, y, z);
        return 3; 
    });
}

void scripting::random_update_block(const Block* block, int x, int y, int z) {
    std::string name = block->name + ".randupdate";
    emit_event(name, [x, y, z] (lua::LuaState* state) {
        state->pushivec3(x, y, z);
        return 3;
    });
}

void scripting::on_block_placed(Player* player, const Block* block, int x, int y, int z) {
    std::string name = block->name + ".placed";
    emit_event(name, [x, y, z, player] (lua::LuaState* state) {
        state->pushivec3(x, y, z);
        state->pushinteger(player->getId());
        return 4; 
    });
}

void scripting::on_block_broken(Player* player, const Block* block, int x, int y, int z) {
    std::string name = block->name + ".broken";
    emit_event(name, [x, y, z, player] (lua::LuaState* state) {
        state->pushivec3(x, y, z);
        state->pushinteger(player->getId());
        return 4;
    });
}

bool scripting::on_block_interact(Player* player, const Block* block, int x, int y, int z) {
    std::string name = block->name + ".interact";
    return emit_event(name, [x, y, z, player] (lua::LuaState* state) {
        state->pushivec3(x, y, z);
        state->pushinteger(player->getId());
        return 4;
    });
}

bool scripting::on_item_use(Player* player, const ItemDef* item) {
    std::string name = item->name + ".use";
    return emit_event(name, [player] (lua::LuaState* state) {
        state->pushinteger(player->getId());
        return 1;
    });
}

bool scripting::on_item_use_on_block(Player* player, const ItemDef* item, int x, int y, int z) {
    std::string name = item->name + ".useon";
    return emit_event(name, [x, y, z, player] (lua::LuaState* state) {
        state->pushivec3(x, y, z);
        state->pushinteger(player->getId());
        return 4;
    });
}

bool scripting::on_item_break_block(Player* player, const ItemDef* item, int x, int y, int z) {
    std::string name = item->name + ".blockbreakby";
    return emit_event(name, [x, y, z, player] (lua::LuaState* state) {
        state->pushivec3(x, y, z);
        state->pushinteger(player->getId());
        return 4;
    });
}

void scripting::on_ui_open(UiDocument* layout, Inventory* inventory, glm::ivec3 blockcoord) {
    std::string name = layout->getId() + ".open";
    emit_event(name, [inventory, blockcoord] (lua::LuaState* state) {
        state->pushinteger(inventory == nullptr ? 0 : inventory->getId());
        state->pushivec3(blockcoord.x, blockcoord.y, blockcoord.z);
        return 4;
    });
}

void scripting::on_ui_close(UiDocument* layout, Inventory* inventory) {
    std::string name = layout->getId() + ".close";
    emit_event(name, [inventory] (lua::LuaState* state) {
        state->pushinteger(inventory == nullptr ? 0 : inventory->getId());
        return 1;
    });
}

bool scripting::register_event(int env, const std::string& name, const std::string& id) {
    if (state->pushenv(env) == 0) {
        state->pushglobals();
    }
    if (state->getfield(name)) {
        state->pop();
        state->getglobal("events");
        state->getfield("on");
        state->pushstring(id);
        state->getfield(name, -4);
        state->callNoThrow(2);
        state->pop();

        // remove previous name
        state->pushnil();
        state->setfield(name);
        return true;
    }
    return false;
}

bool scripting::emit_event(const std::string &name, std::function<int(lua::LuaState *)> args) {
    state->getglobal("events");
    state->getfield("emit");
    state->pushstring(name);
    state->callNoThrow(args(state) + 1);
    bool result = state->toboolean(-1);
    state->pop(2);
    return result;
}

void scripting::load_block_script(int env, std::string prefix, fs::path file, block_funcs_set& funcsset) {
    std::string src = files::read_string(file);
    std::cout << "loading script " << file.u8string() << std::endl;
    state->execute(env, src, file.u8string());
    funcsset.init = register_event(env, "init", prefix+".init");
    funcsset.update = register_event(env, "on_update", prefix+".update");
    funcsset.randupdate = register_event(env, "on_random_update", prefix+".randupdate");
    funcsset.onbroken = register_event(env, "on_broken", prefix+".broken");
    funcsset.onplaced = register_event(env, "on_placed", prefix+".placed");
    funcsset.oninteract = register_event(env, "on_interact", prefix+".interact");
    funcsset.onblockstick = register_event(env, "on_blocks_tick", prefix+".blockstick");
}

void scripting::load_item_script(int env, std::string prefix, fs::path file, item_funcs_set& funcsset) {
    std::string src = files::read_string(file);
    std::cout << "loading script " << file.u8string() << std::endl;
    state->execute(env, src, file.u8string());

    funcsset.init = register_event(env, "init", prefix+".init");
    funcsset.on_use = register_event(env, "on_use", prefix+".use");
    funcsset.on_use_on_block = register_event(env, "on_use_on_block", prefix+".useon");
    funcsset.on_block_break_by = register_event(env, "on_block_break_by", prefix+".blockbreakby");
}

void scripting::load_world_script(int env, std::string prefix, fs::path file) {
    std::string src = files::read_string(file);
    std::cout << "loading script " << file.u8string() << std::endl;

    state->loadbuffer(env, src, file.u8string());
    state->callNoThrow(0);

    register_event(env, "init", prefix+".init");
    register_event(env, "on_world_open", prefix+".worldopen");
    register_event(env, "on_world_tick", prefix+".worldtick");
    register_event(env, "on_world_save", prefix+".worldsave");
    register_event(env, "on_world_quit", prefix+".worldquit");
}

void scripting::load_layout_script(int env, std::string prefix, fs::path file, uidocscript& script) {
    std::string src = files::read_string(file);
    std::cout << "loading script " << file.u8string() << std::endl;

    script.environment = env;
    state->loadbuffer(env, src, file.u8string());
    state->callNoThrow(0);
    script.onopen = register_event(env, "on_open", prefix+".open");
    script.onclose = register_event(env, "on_close", prefix+".close");
}

void scripting::close() {
    delete state;

    state = nullptr;
    content = nullptr;
    indices = nullptr;
    level = nullptr;
}
