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
#include "../../objects/EntityDef.hpp"
#include "../../objects/Entities.hpp"
#include "../../util/stringutil.hpp"
#include "../../util/timeutil.hpp"
#include "../../util/timeutil.hpp"
#include "../../voxels/Block.hpp"
#include "../../world/Level.hpp"

#include <iostream>
#include <stdexcept>

using namespace scripting;

static debug::Logger logger("scripting");

static inline const std::string STDCOMP = "stdcomp";

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

[[nodiscard]]
scriptenv scripting::get_root_environment() {
    return std::make_shared<int>(0);
}

[[nodiscard]]
scriptenv scripting::create_pack_environment(const ContentPack& pack) {
    auto L = lua::get_main_thread();
    int id = lua::create_environment(L, 0);
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

[[nodiscard]]
scriptenv scripting::create_doc_environment(const scriptenv& parent, const std::string& name) {
    auto L = lua::get_main_thread();
    int id = lua::create_environment(L, *parent);
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

[[nodiscard]]
static scriptenv create_component_environment(const scriptenv& parent, 
                                              int entityIdx, 
                                              const std::string& name) {
    auto L = lua::get_main_thread();
    int id = lua::create_environment(L, *parent);

    lua::pushvalue(L, entityIdx);

    lua::pushenv(L, id);

    lua::pushvalue(L, -1);
    lua::setfield(L, "this");

    lua::pushvalue(L, -2);
    lua::setfield(L, "entity");

    lua::pop(L);
    if (lua::getfield(L, "components")) {
        lua::pushenv(L, id);
        lua::setfield(L, name);
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

dynamic::Value scripting::get_component_value(const scriptenv& env, const std::string& name) {
    auto L = lua::get_main_thread();
    lua::pushenv(L, *env);
    if (lua::getfield(L, name)) {
        return lua::tovalue(L, -1);
    }
    return dynamic::NONE;
}

void scripting::on_entity_spawn(
    const EntityDef& def, 
    entityid_t eid, 
    const std::vector<std::unique_ptr<UserComponent>>& components,
    dynamic::Value args,
    dynamic::Map_sptr saved
) {
    auto L = lua::get_main_thread();
    lua::requireglobal(L, STDCOMP);
    if (lua::getfield(L, "new_Entity")) {
        lua::pushinteger(L, eid);
        lua::call(L, 1);
    }
    for (size_t i = 0; i < components.size()-1; i++) {
        lua::pushvalue(L, -1);
    }
    for (auto& component : components) {
        auto compenv = create_component_environment(get_root_environment(), -1, 
                                                    component->name);
        lua::get_from(L, lua::CHUNKS_TABLE, component->name, true);
        lua::pushenv(L, *compenv);

        lua::pushvalue(L, args);
        lua::setfield(L, "ARGS");

        if (saved == nullptr) {
            lua::createtable(L, 0, 0);
        } else {
            if (auto datamap = saved->map(component->name)) {
                lua::pushvalue(L, datamap);
            } else {
                lua::createtable(L, 0, 0);
            }
        }
        lua::setfield(L, "SAVED_DATA");

        lua::setfenv(L);
        lua::call_nothrow(L, 0, 0);

        lua::pushenv(L, *compenv);
        auto& funcsset = component->funcsset;
        funcsset.on_grounded = lua::hasfield(L, "on_grounded");
        funcsset.on_fall = lua::hasfield(L, "on_fall");
        funcsset.on_despawn = lua::hasfield(L, "on_despawn");
        funcsset.on_trigger_enter = lua::hasfield(L, "on_trigger_enter");
        funcsset.on_trigger_exit = lua::hasfield(L, "on_trigger_exit");
        funcsset.on_save = lua::hasfield(L, "on_save");
        lua::pop(L, 2);

        component->env = compenv;
    }
}

static bool process_entity_callback(
    const scriptenv& env, 
    const std::string& name, 
    std::function<int(lua::State*)> args
) {
    auto L = lua::get_main_thread();
    lua::pushenv(L, *env);
    if (lua::getfield(L, name)) {
        if (args) {
            lua::call_nothrow(L, args(L), 0);
        } else {
            lua::call_nothrow(L, 0, 0);
        }
    }
    lua::pop(L);
    return true;
}

bool scripting::on_entity_despawn(const EntityDef& def, const Entity& entity) {
    const auto& script = entity.getScripting();
    for (auto& component : script.components) {
        if (component->funcsset.on_despawn) {
            process_entity_callback(component->env, "on_despawn", nullptr);
        }
    }
    auto L = lua::get_main_thread();
    lua::get_from(L, "stdcomp", "remove_Entity", true);
    lua::pushinteger(L, entity.getUID());
    lua::call(L, 1, 0);
    return true;
}

bool scripting::on_entity_grounded(const Entity& entity, float force) {
    const auto& script = entity.getScripting();
    for (auto& component : script.components) {
        if (component->funcsset.on_grounded) {
            process_entity_callback(component->env, "on_grounded", [force](auto L){
                return lua::pushnumber(L, force);
            });
        }
    }
    return true;
}

bool scripting::on_entity_fall(const Entity& entity) {
    const auto& script = entity.getScripting();
    for (auto& component : script.components) {
        if (component->funcsset.on_fall) {
            process_entity_callback(component->env, "on_fall", nullptr);
        }
    }
    return true;
}

bool scripting::on_entity_save(const Entity& entity) {
    const auto& script = entity.getScripting();
    for (auto& component : script.components) {
        if (component->funcsset.on_save) {
            process_entity_callback(component->env, "on_save", nullptr);
        }
    }
    return true;
}

void scripting::on_trigger_enter(const Entity& entity, size_t index, entityid_t oid) {
    const auto& script = entity.getScripting();
    for (auto& component : script.components) {
        if (component->funcsset.on_trigger_enter) {
            process_entity_callback(component->env, "on_trigger_enter", [index, oid](auto L) {
                lua::pushinteger(L, index);
                lua::pushinteger(L, oid);
                return 2;
            });
        }
    }
}

void scripting::on_trigger_exit(const Entity& entity, size_t index, entityid_t oid) {
    const auto& script = entity.getScripting();
    for (auto& component : script.components) {
        if (component->funcsset.on_trigger_exit) {
            process_entity_callback(component->env, "on_trigger_exit", [index, oid](auto L) {
                lua::pushinteger(L, index);
                lua::pushinteger(L, oid);
                return 2;
            });
        }
    }
}

void scripting::on_entities_update() {
    auto L = lua::get_main_thread();
    lua::get_from(L, STDCOMP, "update", true);
    lua::call_nothrow(L, 0, 0);
    lua::pop(L);
}

void scripting::on_entities_render() {
    auto L = lua::get_main_thread();
    lua::get_from(L, STDCOMP, "render", true);
    lua::call_nothrow(L, 0, 0);
    lua::pop(L);
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

int scripting::get_values_on_stack() {
    return lua::gettop(lua::get_main_thread());
}

static void load_script(int env, const std::string& type, const fs::path& file) {
    std::string src = files::read_string(file);
    logger.info() << "script (" << type << ") " << file.u8string();
    lua::execute(lua::get_main_thread(), env, src, file.u8string());
}

void scripting::load_block_script(const scriptenv& senv, const std::string& prefix, const fs::path& file, block_funcs_set& funcsset) {
    int env = *senv;
    load_script(env, "block", file);
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
    load_script(env, "item", file);
    funcsset.init = register_event(env, "init", prefix+".init");
    funcsset.on_use = register_event(env, "on_use", prefix+".use");
    funcsset.on_use_on_block = register_event(env, "on_use_on_block", prefix+".useon");
    funcsset.on_block_break_by = register_event(env, "on_block_break_by", prefix+".blockbreakby");
}

void scripting::load_entity_component(const scriptenv& penv, const std::string& name, const fs::path& file) {
    auto L = lua::get_main_thread();
    std::string src = files::read_string(file);
    logger.info() << "script (component) " << file.u8string();
    lua::loadbuffer(L, *penv, src, file.u8string());
    lua::store_in(L, lua::CHUNKS_TABLE, name);
}

void scripting::load_world_script(const scriptenv& senv, const std::string& prefix, const fs::path& file) {
    int env = *senv;
    load_script(env, "world", file);
    register_event(env, "init", prefix+".init");
    register_event(env, "on_world_open", prefix+".worldopen");
    register_event(env, "on_world_tick", prefix+".worldtick");
    register_event(env, "on_world_save", prefix+".worldsave");
    register_event(env, "on_world_quit", prefix+".worldquit");
}

void scripting::load_layout_script(const scriptenv& senv, const std::string& prefix, const fs::path& file, uidocscript& script) {
    int env = *senv;
    load_script(env, "layout", file);
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
