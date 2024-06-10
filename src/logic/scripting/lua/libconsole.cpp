#include "api_lua.hpp"
#include "lua_commons.hpp"
#include "lua_util.hpp"
#include "LuaState.hpp"

#include "../scripting.hpp"
#include "../../CommandsInterpreter.hpp"
#include "../../../engine.hpp"
#include "../../../coders/commons.hpp"

namespace scripting {
    extern lua::LuaState* state;
}

using namespace scripting;

static int l_add_command(lua_State* L) {
    if (!lua_isfunction(L, 3)) {
        throw std::runtime_error("invalid callback");
    }
    auto scheme = lua::require_string(L, 1);
    auto description = lua::require_string(L, 2);
    lua_pushvalue(L, 3);
    auto func = state->createLambda(L);
    try {
        engine->getCommandsInterpreter()->getRepository()->add(
            scheme, description, [func](auto, auto args, auto kwargs) {
                return func({args, kwargs});
            }
        );
    } catch (const parsing_error& err) {
        throw std::runtime_error(("command scheme error:\n"+err.errorLog()).c_str());
    }
    return 0;
}

static int l_execute(lua_State* L) {
    auto prompt = lua::require_string(L, 1);
    auto result = engine->getCommandsInterpreter()->execute(prompt);
    lua::pushvalue(L, result);
    return 1;
}

static int l_set(lua_State* L) {
    auto name = lua::require_string(L, 1);
    auto value = lua::tovalue(L, 2);
    (*engine->getCommandsInterpreter())[name] = value;
    return 0;
}

static int l_get_commands_list(lua_State* L) {
    auto interpreter = engine->getCommandsInterpreter();
    auto repo = interpreter->getRepository();
    const auto& commands = repo->getCommands();

    lua_createtable(L, commands.size(), 0);
    size_t index = 1;
    for (const auto& entry : commands) {
        lua_pushstring(L, entry.first.c_str());
        lua_rawseti(L, -2, index++);
    }
    return 1;
}

static int l_get_command_info(lua_State* L) {
    auto name = lua::require_string(L, 1);
    auto interpreter = engine->getCommandsInterpreter();
    auto repo = interpreter->getRepository();
    auto command = repo->get(name);
    if (command == nullptr) {
        return 0;
    }
    const auto& args = command->getArgs();
    const auto& kwargs = command->getKwArgs();
    
    lua_createtable(L, 0, 4);

    lua_pushstring(L, name);
    lua_setfield(L, -2, "name");

    lua_pushstring(L, command->getDescription().c_str());
    lua_setfield(L, -2, "description");
    
    lua_createtable(L, args.size(), 0);
    for (size_t i = 0; i < args.size(); i++) {
        auto& arg = args.at(i);
        lua_createtable(L, 0, 2);
        
        lua_pushstring(L, arg.name.c_str());
        lua_setfield(L, -2, "name");
        
        lua_pushstring(L, cmd::argtype_name(arg.type).c_str());
        lua_setfield(L, -2, "type");

        if (arg.optional) {
            lua_pushboolean(L, true);
            lua_setfield(L, -2, "optional");
        }
        lua_rawseti(L, -2, i+1);
    }
    lua_setfield(L, -2, "args");

    lua_createtable(L, 0, kwargs.size());
    for (auto& entry : kwargs) {
        auto& arg = entry.second;
        lua_createtable(L, 0, 1);

        lua_pushstring(L, cmd::argtype_name(arg.type).c_str());
        lua_setfield(L, -2, "type");

        lua_setfield(L, -2, arg.name.c_str());
    }
    lua_setfield(L, -2, "kwargs");
    return 1;
}

const luaL_Reg consolelib [] = {
    {"add_command", lua_wrap_errors<l_add_command>},
    {"execute", lua_wrap_errors<l_execute>},
    {"set", lua_wrap_errors<l_set>},
    {"get_commands_list", lua_wrap_errors<l_get_commands_list>},
    {"get_command_info", lua_wrap_errors<l_get_command_info>},
    {NULL, NULL}
};
