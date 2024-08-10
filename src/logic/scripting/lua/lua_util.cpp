#include "lua_util.hpp"

#include <iomanip>
#include <iostream>

#include "util/stringutil.hpp"

using namespace lua;

static int nextEnvironment = 1;

std::unordered_map<std::type_index, std::string> lua::usertypeNames;

int lua::userdata_destructor(lua::State* L) {
    if (auto obj = touserdata<Userdata>(L, 1)) {
        obj->~Userdata();
    }
    return 0;
}

std::string lua::env_name(int env) {
    return "_ENV" + util::mangleid(env);
}

int lua::pushvalue(State* L, const dynamic::Value& value) {
    using namespace dynamic;

    if (auto* flag = std::get_if<bool>(&value)) {
        pushboolean(L, *flag);
    } else if (auto* num = std::get_if<integer_t>(&value)) {
        pushinteger(L, *num);
    } else if (auto* num = std::get_if<number_t>(&value)) {
        pushnumber(L, *num);
    } else if (auto* str = std::get_if<std::string>(&value)) {
        pushstring(L, *str);
    } else if (auto listptr = std::get_if<List_sptr>(&value)) {
        auto list = *listptr;
        createtable(L, list->size(), 0);
        for (size_t i = 0; i < list->size(); i++) {
            pushvalue(L, list->get(i));
            rawseti(L, i + 1);
        }
    } else if (auto mapptr = std::get_if<Map_sptr>(&value)) {
        auto map = *mapptr;
        createtable(L, 0, map->size());
        for (auto& entry : map->values) {
            pushvalue(L, entry.second);
            setfield(L, entry.first);
        }
    } else {
        pushnil(L);
    }
    return 1;
}

std::wstring lua::require_wstring(State* L, int idx) {
    return util::str2wstr_utf8(require_string(L, idx));
}

int lua::pushwstring(State* L, const std::wstring& str) {
    return pushstring(L, util::wstr2str_utf8(str));
}

dynamic::Value lua::tovalue(State* L, int idx) {
    using namespace dynamic;
    auto type = lua::type(L, idx);
    switch (type) {
        case LUA_TNIL:
        case LUA_TNONE:
            return dynamic::NONE;
        case LUA_TBOOLEAN:
            return toboolean(L, idx) == 1;
        case LUA_TNUMBER: {
            auto number = tonumber(L, idx);
            auto integer = tointeger(L, idx);
            if (number == static_cast<Number>(integer)) {
                return integer;
            } else {
                return number;
            }
        }
        case LUA_TFUNCTION:
            return "<function " +
                   std::to_string(
                       reinterpret_cast<ptrdiff_t>(lua_topointer(L, idx))
                   ) +
                   ">";
        case LUA_TSTRING:
            return std::string(tostring(L, idx));
        case LUA_TTABLE: {
            int len = objlen(L, idx);
            if (len) {
                // array
                auto list = create_list();
                for (int i = 1; i <= len; i++) {
                    rawgeti(L, i, idx);
                    list->put(tovalue(L, -1));
                    pop(L);
                }
                return list;
            } else {
                // table
                auto map = create_map();
                pushvalue(L, idx);
                pushnil(L);
                while (next(L, -2)) {
                    pushvalue(L, -2);
                    auto key = tostring(L, -1);
                    map->put(key, tovalue(L, -2));
                    pop(L, 2);
                }
                pop(L);
                return map;
            }
        }
        default:
            throw std::runtime_error(
                "lua type " + std::string(lua_typename(L, type)) +
                " is not supported"
            );
    }
}

static int l_error_handler(lua_State* L) {
    if (!isstring(L, 1)) {  // 'message' not a string?
        return 1;           // keep it intact
    }
    if (get_from(L, "debug", "traceback")) {
        lua_pushvalue(L, 1);    // pass error message
        lua_pushinteger(L, 2);  // skip this function and traceback
        lua_call(L, 2, 1);      // call debug.traceback
    }
    return 1;
}

int lua::call(State* L, int argc, int nresults) {
    int handler_pos = gettop(L) - argc;
    pushcfunction(L, l_error_handler);
    insert(L, handler_pos);
    if (lua_pcall(L, argc, nresults, handler_pos)) {
        std::string log = tostring(L, -1);
        pop(L);
        remove(L, handler_pos);
        throw luaerror(log);
    }
    remove(L, handler_pos);
    return nresults == -1 ? 1 : nresults;
}

int lua::call_nothrow(State* L, int argc, int nresults) {
    int handler_pos = gettop(L) - argc;
    pushcfunction(L, l_error_handler);
    insert(L, handler_pos);
    if (lua_pcall(L, argc, LUA_MULTRET, handler_pos)) {
        log_error(tostring(L, -1));
        pop(L);
        remove(L, handler_pos);
        return 0;
    }
    remove(L, handler_pos);
    return nresults == -1 ? 1 : nresults;
}

void lua::dump_stack(State* L) {
    int top = gettop(L);
    for (int i = 1; i <= top; i++) {
        std::cout << std::setw(3) << i << std::setw(20) << luaL_typename(L, i)
                  << std::setw(30);
        switch (lua::type(L, i)) {
            case LUA_TNUMBER:
                std::cout << tonumber(L, i);
                break;
            case LUA_TSTRING:
                std::cout << tostring(L, i);
                break;
            case LUA_TBOOLEAN:
                std::cout << (toboolean(L, i) ? "true" : "false");
                break;
            case LUA_TNIL:
                std::cout << "nil";
                break;
            default:
                std::cout << topointer(L, i);
                break;
        }
        std::cout << std::endl;
    }
}

static std::shared_ptr<std::string> create_lambda_handler(State* L) {
    auto ptr = reinterpret_cast<ptrdiff_t>(topointer(L, -1));
    auto name = util::mangleid(ptr);
    getglobal(L, LAMBDAS_TABLE);
    pushvalue(L, -2);
    setfield(L, name);
    pop(L, 2);

    return std::shared_ptr<std::string>(
        new std::string(name),
        [=](std::string* name) {
            getglobal(L, LAMBDAS_TABLE);
            pushnil(L);
            setfield(L, *name);
            pop(L);
            delete name;
        }
    );
}

runnable lua::create_runnable(State* L) {
    auto funcptr = create_lambda_handler(L);
    return [=]() {
        getglobal(L, LAMBDAS_TABLE);
        getfield(L, *funcptr);
        call_nothrow(L, 0);
        pop(L);
    };
}

scripting::common_func lua::create_lambda(State* L) {
    auto funcptr = create_lambda_handler(L);
    return [=](const std::vector<dynamic::Value>& args) {
        getglobal(L, LAMBDAS_TABLE);
        getfield(L, *funcptr);
        for (const auto& arg : args) {
            pushvalue(L, arg);
        }
        if (call(L, args.size(), 1)) {
            auto result = tovalue(L, -1);
            pop(L);
            return result;
        }
        return dynamic::Value(dynamic::NONE);
    };
}

int lua::create_environment(State* L, int parent) {
    int id = nextEnvironment++;

    // local env = {}
    createtable(L, 0, 1);

    // setmetatable(env, {__index=_G})
    createtable(L, 0, 1);
    if (parent == 0) {
        pushglobals(L);
    } else {
        if (pushenv(L, parent) == 0) {
            pushglobals(L);
        }
    }
    setfield(L, "__index");
    setmetatable(L);

    // envname = env
    setglobal(L, env_name(id));
    return id;
}

void lua::removeEnvironment(State* L, int id) {
    if (id == 0) {
        return;
    }
    pushnil(L);
    setglobal(L, env_name(id));
}
