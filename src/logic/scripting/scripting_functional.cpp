#include "scripting_functional.hpp"

#include "lua/lua_util.hpp"
#include "lua/LuaState.hpp"
#include "../../debug/Logger.hpp"
#include "../../util/stringutil.hpp"

namespace scripting {
    extern lua::LuaState* state;
}

static debug::Logger logger("scripting_func");

using namespace scripting;

runnable scripting::create_runnable(
    const scriptenv& env,
    const std::string& src,
    const std::string& file
) {
    auto L = state->getMainThread();
    try {
        state->loadbuffer(L, *env, src, file);
        return state->createRunnable(L);
    } catch (const lua::luaerror& err) {
        logger.error() << err.what();
        return [](){};
    }
}

static lua_State* processCallback(
    const scriptenv& env,
    const std::string& src,
    const std::string& file
) {
    auto L = state->getMainThread();
    try {
        if (state->eval(L, *env, src, file) != 0) {
            return L;
        }
    } catch (lua::luaerror& err) {
        logger.error() << err.what();
    }
    return nullptr;
}

wstringconsumer scripting::create_wstring_consumer(
    const scriptenv& env,
    const std::string& src,
    const std::string& file
) {
    return [=](const std::wstring& x){
        if (auto L = processCallback(env, src, file)) {
            lua::pushwstring(L, x);
            lua::callNoThrow(L, 1);
        }
    };
}

wstringsupplier scripting::create_wstring_supplier(
    const scriptenv& env,
    const std::string& src,
    const std::string& file
) {
    return [=](){
        if (auto L = processCallback(env, src, file)) {
            if (lua_isfunction(L, -1)) {
                lua::callNoThrow(L, 0);
            }
            auto str = lua::require_wstring(L, -1); lua_pop(L, 1);
            return str;
        }
        return std::wstring(L"");
    };
}

wstringchecker scripting::create_wstring_validator(
    const scriptenv& env,
    const std::string& src,
    const std::string& file
) {
    return [=](const std::wstring& x){
        if (auto L = processCallback(env, src, file)) {
            lua::pushwstring(L, x);
            if (lua::callNoThrow(L, 1))
                return lua::toboolean(L, -1);
        }
        return false;
    };
}

boolconsumer scripting::create_bool_consumer(
    const scriptenv& env,
    const std::string& src,
    const std::string& file
) {
    return [=](bool x){
        if (auto L = processCallback(env, src, file)) {
            lua::pushboolean(L, x);
            lua::callNoThrow(L, 1);
        }
    };
}

boolsupplier scripting::create_bool_supplier(
    const scriptenv& env,
    const std::string& src,
    const std::string& file
) {
    return [=](){
        if (auto L = processCallback(env, src, file)) {
            if (lua_isfunction(L, -1)) {
                lua::callNoThrow(L, 0);
            }
            bool x = lua::toboolean(L,-1); lua_pop(L, 1);
            return x;
        }
        return false;
    };
}

doubleconsumer scripting::create_number_consumer(
    const scriptenv& env,
    const std::string& src,
    const std::string& file
) {
    return [=](double x){
        if (auto L = processCallback(env, src, file)) {
            lua::pushnumber(L, x);
            lua::callNoThrow(L, 1);
        }
    };
}

doublesupplier scripting::create_number_supplier(
    const scriptenv& env,
    const std::string& src,
    const std::string& file
) {
    return [=](){
        if (auto L = processCallback(env, src, file)) {
            if (lua_isfunction(L, -1)) {
                lua::callNoThrow(L, 0);
            }
            auto x = lua_tonumber(L, -1); 
            lua::pop(L);
            return x;
        }
        return 0.0;
    };
}

int_array_consumer scripting::create_int_array_consumer(
    const scriptenv& env,
    const std::string& src, 
    const std::string& file
) {
    return [=](const int arr[], size_t len) {
        if (auto L = processCallback(env, src, file)) {
            for (uint i = 0; i < len; i++) {
                lua::pushinteger(L, arr[i]);
            }
            lua::callNoThrow(L, len);
        }
    };
}

vec2supplier scripting::create_vec2_supplier(
    const scriptenv& env,
    const std::string& src, 
    const std::string& file
) {
    return [=]() {
        if (auto L = processCallback(env, src, file)) {
            if (lua_isfunction(L, -1)) {
                lua::callNoThrow(L, 0);
            }
            auto y = lua_tonumber(L, -1); lua::pop(L);
            auto x = lua_tonumber(L, -1); lua::pop(L);
            return glm::vec2(x, y);
        }
        return glm::vec2(0, 0);
    };
}
