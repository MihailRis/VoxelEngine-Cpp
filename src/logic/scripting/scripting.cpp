#include "scripting.hpp"

#include "lua/lua_engine.hpp"

#include "../../content/ContentPack.hpp"
#include "../../debug/Logger.hpp"
#include "../../engine.hpp"
#include "../../files/engine_paths.hpp"
#include "../../files/files.hpp"
#include "../../frontend/UiDocument.hpp"
#include "../../items/Inventory.hpp"
#include "../../items/ItemDef.hpp"
#include "../../logic/BlocksController.hpp"
#include "../../logic/LevelController.hpp"
#include "../../objects/Player.hpp"
#include "../../util/stringutil.hpp"
#include "../../util/timeutil.hpp"
#include "../../util/timeutil.hpp"
#include "../../voxels/Block.hpp"
#include "../../world/Level.hpp"

#include <iostream>
#include <stdexcept>

using namespace scripting;

static debug::Logger logger("scripting");

Engine* scripting::engine = nullptr;
Level* scripting::level = nullptr;
const Content* scripting::content = nullptr;
const ContentIndices* scripting::indices = nullptr;
BlocksController* scripting::blocks = nullptr;
LevelController* scripting::controller = nullptr;

static void load_script(const fs::path& name) {
    auto paths = scripting::engine->getPaths();
    fs::path file = paths->getResources()/fs::path("scripts")/name;

    std::string src = files::read_string(file);
    lua::execute(lua::get_main_thread(), 0, src, file.u8string());
}

void scripting::initialize(Engine* engine) {
    scripting::engine = engine;
    lua::initialize();

    load_script(fs::path("stdlib.lua"));
    load_script(fs::path("stdcmd.lua"));
}

scriptenv scripting::get_root_environment() {
    return std::make_shared<int>(0);
}

scriptenv scripting::create_pack_environment(const ContentPack& pack) {
    auto L = lua::get_main_thread();
    int id = lua::createEnvironment(L, 0);
    lua::pushenv(L, id);
    lua::pushvalue(L, -1);
    lua::setfield(L, "PACK_ENV");
    lua::pushstring(L, pack.id);
    lua::setfield(L, "PACK_ID");
    lua::pop(L);
    return std::shared_ptr<int>(new int(id), [=](int* id) {
        lua::removeEnvironment(L, *id);
        delete id;
    });
}

scriptenv scripting::create_doc_environment(const scriptenv& parent, const std::string& name) {
    auto L = lua::get_main_thread();
    int id = lua::createEnvironment(L, *parent);
    lua::pushenv(L, id);
    lua::pushvalue(L, -1);
    lua::setfield(L, "DOC_ENV");
    lua::pushstring(L, name);
    lua::setfield(L, "DOC_NAME");
    
    if (lua::getglobal(L, "Document")) {
        if (lua::getfield(L, "new")) {
            lua::pushstring(L, name);
            if (lua::call_nothrow(L, 1)) {
                lua::setfield(L, "document", -3);
            }
        }
        lua::pop(L);
    }
    lua::pop(L);
    return std::shared_ptr<int>(new int(id), [=](int* id) {
        lua::removeEnvironment(L, *id);
        delete id;
    });
}

void scripting::process_post_runnables() {
    auto L = lua::get_main_thread();
    if (lua::getglobal(L, "__process_post_runnables")) {
        lua::call_nothrow(L, 0);
    }
}

void scripting::on_world_load(LevelController* controller) {
    scripting::level = controller->getLevel();
    scripting::content = level->content;
    scripting::indices = level->content->getIndices();
    scripting::blocks = controller->getBlocksController();
    scripting::controller = controller;
    load_script("world.lua");

    auto L = lua::get_main_thread();
    for (auto& pack : scripting::engine->getContentPacks()) {
        lua::emit_event(L, pack.id + ".worldopen");
    }
}

void scripting::on_world_tick() {
    auto L = lua::get_main_thread();
    for (auto& pack : scripting::engine->getContentPacks()) {
        lua::emit_event(L, pack.id + ".worldtick");
    }
}

void scripting::on_world_save() {
    auto L = lua::get_main_thread();
    for (auto& pack : scripting::engine->getContentPacks()) {
        lua::emit_event(L, pack.id + ".worldsave");
    }
}

void scripting::on_world_quit() {
    auto L = lua::get_main_thread();
    for (auto& pack : scripting::engine->getContentPacks()) {
        lua::emit_event(L, pack.id + ".worldquit");
    }

    lua::getglobal(L, "pack");
    for (auto& pack : scripting::engine->getContentPacks()) {
        lua::getfield(L, "unload");
        lua::pushstring(L, pack.id);
        lua::call_nothrow(L, 1);   
    }
    lua::pop(L);
    
    if (lua::getglobal(L, "__scripts_cleanup")) {
        lua::call_nothrow(L, 0);
    }
    scripting::level = nullptr;
    scripting::content = nullptr;
    scripting::indices = nullptr;
    scripting::blocks = nullptr;
    scripting::controller = nullptr;
}

void scripting::on_blocks_tick(const Block* block, int tps) {
    std::string name = block->name + ".blockstick";
    lua::emit_event(lua::get_main_thread(), name, [tps] (auto L) {
        return lua::pushinteger(L, tps);
    });
}

void scripting::update_block(const Block* block, int x, int y, int z) {
    std::string name = block->name + ".update";
    lua::emit_event(lua::get_main_thread(), name, [x, y, z] (auto L) {
        return lua::pushivec3(L, x, y, z);
    });
}

void scripting::random_update_block(const Block* block, int x, int y, int z) {
    std::string name = block->name + ".randupdate";
    lua::emit_event(lua::get_main_thread(), name, [x, y, z] (auto L) {
        return lua::pushivec3(L, x, y, z);
    });
}

void scripting::on_block_placed(Player* player, const Block* block, int x, int y, int z) {
    std::string name = block->name + ".placed";
    lua::emit_event(lua::get_main_thread(), name, [x, y, z, player] (auto L) {
        lua::pushivec3(L, x, y, z);
        lua::pushinteger(L, player->getId());
        return 4;
    });
}

void scripting::on_block_broken(Player* player, const Block* block, int x, int y, int z) {
    std::string name = block->name + ".broken";
    lua::emit_event(lua::get_main_thread(), name, [x, y, z, player] (auto L) {
        lua::pushivec3(L, x, y, z);
        lua::pushinteger(L, player->getId());
        return 4;
    });
}

bool scripting::on_block_interact(Player* player, const Block* block, glm::ivec3 pos) {
    std::string name = block->name + ".interact";
    return lua::emit_event(lua::get_main_thread(), name, [pos, player] (auto L) {
        lua::pushivec3(L, pos.x, pos.y, pos.z);
        lua::pushinteger(L, player->getId());
        return 4;
    });
}

bool scripting::on_item_use(Player* player, const ItemDef* item) {
    std::string name = item->name + ".use";
    return lua::emit_event(lua::get_main_thread(), name, [player] (lua::State* L) {
        return lua::pushinteger(L, player->getId());
    });
}

bool scripting::on_item_use_on_block(Player* player, const ItemDef* item, int x, int y, int z) {
    std::string name = item->name + ".useon";
    return lua::emit_event(lua::get_main_thread(), name, [x, y, z, player] (auto L) {
        lua::pushivec3(L, x, y, z);
        lua::pushinteger(L, player->getId());
        return 4;
    });
}

bool scripting::on_item_break_block(Player* player, const ItemDef* item, int x, int y, int z) {
    std::string name = item->name + ".blockbreakby";
    return lua::emit_event(lua::get_main_thread(), name, [x, y, z, player] (auto L) {
        lua::pushivec3(L, x, y, z);
        lua::pushinteger(L, player->getId());
        return 4;
    });
}

void scripting::on_ui_open(
    UiDocument* layout,
    std::vector<dynamic::Value> args
) {
    auto argsptr = std::make_shared<std::vector<dynamic::Value>>(std::move(args));
    std::string name = layout->getId() + ".open";
    lua::emit_event(lua::get_main_thread(), name, [=] (auto L) {
        for (const auto& value : *argsptr) {
            lua::pushvalue(L, value);
        }
        return argsptr->size();
    });
}

void scripting::on_ui_progress(UiDocument* layout, int workDone, int workTotal) {
    std::string name = layout->getId() + ".progress";
    lua::emit_event(lua::get_main_thread(), name, [=] (auto L) {
        lua::pushinteger(L, workDone);
        lua::pushinteger(L, workTotal);
        return 2;
    });
}

void scripting::on_ui_close(UiDocument* layout, Inventory* inventory) {
    std::string name = layout->getId() + ".close";
    lua::emit_event(lua::get_main_thread(), name, [inventory] (auto L) {
        return lua::pushinteger(L, inventory ? inventory->getId() : 0);
    });
}

bool scripting::register_event(int env, const std::string& name, const std::string& id) {
    auto L = lua::get_main_thread();
    if (lua::pushenv(L, env) == 0) {
        lua::pushglobals(L);
    }
    if (lua::getfield(L, name)) {
        lua::pop(L);
        lua::getglobal(L, "events");
        lua::getfield(L, "on");
        lua::pushstring(L, id);
        lua::getfield(L, name, -4);
        lua::call_nothrow(L, 2);
        lua::pop(L);

        // remove previous name
        lua::pushnil(L);
        lua::setfield(L, name);
        return true;
    }
    return false;
}

void scripting::load_block_script(const scriptenv& senv, const std::string& prefix, const fs::path& file, block_funcs_set& funcsset) {
    int env = *senv;
    std::string src = files::read_string(file);
    logger.info() << "script (block) " << file.u8string();
    lua::execute(lua::get_main_thread(), env, src, file.u8string());
    funcsset.init = register_event(env, "init", prefix+".init");
    funcsset.update = register_event(env, "on_update", prefix+".update");
    funcsset.randupdate = register_event(env, "on_random_update", prefix+".randupdate");
    funcsset.onbroken = register_event(env, "on_broken", prefix+".broken");
    funcsset.onplaced = register_event(env, "on_placed", prefix+".placed");
    funcsset.oninteract = register_event(env, "on_interact", prefix+".interact");
    funcsset.onblockstick = register_event(env, "on_blocks_tick", prefix+".blockstick");
}

void scripting::load_item_script(const scriptenv& senv, const std::string& prefix, const fs::path& file, item_funcs_set& funcsset) {
    int env = *senv;
    std::string src = files::read_string(file);
    logger.info() << "script (item) " << file.u8string();
    lua::execute(lua::get_main_thread(), env, src, file.u8string());

    funcsset.init = register_event(env, "init", prefix+".init");
    funcsset.on_use = register_event(env, "on_use", prefix+".use");
    funcsset.on_use_on_block = register_event(env, "on_use_on_block", prefix+".useon");
    funcsset.on_block_break_by = register_event(env, "on_block_break_by", prefix+".blockbreakby");
}

void scripting::load_world_script(const scriptenv& senv, const std::string& prefix, const fs::path& file) {
    int env = *senv;

    std::string src = files::read_string(file);
    logger.info() << "loading world script for " << prefix;
    lua::execute(lua::get_main_thread(), env, src, file.u8string());

    register_event(env, "init", prefix+".init");
    register_event(env, "on_world_open", prefix+".worldopen");
    register_event(env, "on_world_tick", prefix+".worldtick");
    register_event(env, "on_world_save", prefix+".worldsave");
    register_event(env, "on_world_quit", prefix+".worldquit");
}

void scripting::load_layout_script(const scriptenv& senv, const std::string& prefix, const fs::path& file, uidocscript& script) {
    int env = *senv;

    std::string src = files::read_string(file);
    logger.info() << "loading script " << file.u8string();

    lua::execute(lua::get_main_thread(), env, src, file.u8string());
    script.onopen = register_event(env, "on_open", prefix+".open");
    script.onprogress = register_event(env, "on_progress", prefix+".progress");
    script.onclose = register_event(env, "on_close", prefix+".close");
}

void scripting::close() {
    lua::finalize();
    content = nullptr;
    indices = nullptr;
    level = nullptr;
}
