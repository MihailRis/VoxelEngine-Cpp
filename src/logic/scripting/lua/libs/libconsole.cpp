#include "coders/commons.hpp"
#include "engine/Engine.hpp"
#include "logic/CommandsInterpreter.hpp"
#include "api_lua.hpp"

using namespace scripting;

static int l_add_command(lua::State* L) {
    if (!lua::isfunction(L, 3)) {
        throw std::runtime_error("invalid callback");
    }
    auto scheme = lua::require_string(L, 1);
    auto description = lua::require_string(L, 2);
    lua::pushvalue(L, 3);
    auto func = lua::create_lambda(L);
    try {
        engine->getCommandsInterpreter()->getRepository()->add(
            scheme,
            description,
            [func](auto, auto args, auto kwargs) {
                return func({args, kwargs});
            }
        );
    } catch (const parsing_error& err) {
        throw std::runtime_error(
            ("command scheme error:\n" + err.errorLog()).c_str()
        );
    }
    return 0;
}

static int l_execute(lua::State* L) {
    auto prompt = lua::require_string(L, 1);
    try {
        auto result = engine->getCommandsInterpreter()->execute(prompt);
        lua::pushvalue(L, result);
        return 1;
    } catch (const parsing_error& err) {
        if (std::string(err.what()).find("unknown command ") == 0) {
            throw;
        }
        throw std::runtime_error(err.errorLog());
    }
}

static int l_set(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto value = lua::tovalue(L, 2);
    (*engine->getCommandsInterpreter())[name] = value;
    return 0;
}

static int l_get_commands_list(lua::State* L) {
    auto interpreter = engine->getCommandsInterpreter();
    auto repo = interpreter->getRepository();
    const auto& commands = repo->getCommands();

    lua::createtable(L, commands.size(), 0);
    size_t index = 1;
    for (const auto& entry : commands) {
        lua::pushstring(L, entry.first);
        lua::rawseti(L, index++);
    }
    return 1;
}

static int l_get_command_info(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto interpreter = engine->getCommandsInterpreter();
    auto repo = interpreter->getRepository();
    auto command = repo->get(name);
    if (command == nullptr) {
        return 0;
    }
    const auto& args = command->getArgs();
    const auto& kwargs = command->getKwArgs();

    lua::createtable(L, 0, 4);

    lua::pushstring(L, name);
    lua::setfield(L, "name");

    lua::pushstring(L, command->getDescription());
    lua::setfield(L, "description");

    lua::createtable(L, args.size(), 0);
    for (size_t i = 0; i < args.size(); i++) {
        auto& arg = args[i];
        lua::createtable(L, 0, 2);

        lua::pushstring(L, arg.name);
        lua::setfield(L, "name");

        lua::pushstring(L, cmd::argtype_name(arg.type));
        lua::setfield(L, "type");

        if (arg.optional) {
            lua::pushboolean(L, true);
            lua::setfield(L, "optional");
        }
        lua::rawseti(L, i + 1);
    }
    lua::setfield(L, "args");

    lua::createtable(L, 0, kwargs.size());
    for (auto& entry : kwargs) {
        auto& arg = entry.second;
        lua::createtable(L, 0, 1);

        lua::pushstring(L, cmd::argtype_name(arg.type));
        lua::setfield(L, "type");

        lua::setfield(L, arg.name);
    }
    lua::setfield(L, "kwargs");
    return 1;
}

const luaL_Reg consolelib[] = {
    {"add_command", lua::wrap<l_add_command>},
    {"execute", lua::wrap<l_execute>},
    {"set", lua::wrap<l_set>},
    {"get_commands_list", lua::wrap<l_get_commands_list>},
    {"get_command_info", lua::wrap<l_get_command_info>},
    {NULL, NULL}};
