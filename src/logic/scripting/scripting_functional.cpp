#include "scripting_functional.h"

#include "lua/LuaState.hpp"
#include "../../debug/Logger.hpp"
#include "../../util/stringutil.h"

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
    try {
        state->loadbuffer(*env, src, file);
        return state->createRunnable();
    } catch (const lua::luaerror& err) {
        logger.error() << err.what();
        return [](){};
    }
}

static bool processCallback(
    const scriptenv& env,
    const std::string& src,
    const std::string& file
) {
    try {
        return state->eval(*env, src, file) != 0;
    } catch (lua::luaerror& err) {
        std::cerr << err.what() << std::endl;
        return false;
    }
}

wstringconsumer scripting::create_wstring_consumer(
    const scriptenv& env,
    const std::string& src,
    const std::string& file
) {
    return [=](const std::wstring& x){
        if (processCallback(env, src, file)) {
            state->pushstring(util::wstr2str_utf8(x));
            state->callNoThrow(1);
        }
    };
}

wstringsupplier scripting::create_wstring_supplier(
    const scriptenv& env,
    const std::string& src,
    const std::string& file
) {
    return [=](){
        if (processCallback(env, src, file)) {
            if (state->isfunction(-1)) {
                state->callNoThrow(0);
            }
            auto str = state->tostring(-1); state->pop();
            return util::str2wstr_utf8(str);
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
        if (processCallback(env, src, file)) {
            state->pushstring(util::wstr2str_utf8(x));
            if (state->callNoThrow(1))
                return state->toboolean(-1);
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
        if (processCallback(env, src, file)) {
            state->pushboolean(x);
            state->callNoThrow(1);
        }
    };
}

boolsupplier scripting::create_bool_supplier(
    const scriptenv& env,
    const std::string& src,
    const std::string& file
) {
    return [=](){
        if (processCallback(env, src, file)) {
            if (state->isfunction(-1)) {
                state->callNoThrow(0);
            }
            bool x = state->toboolean(-1); state->pop();
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
        if (processCallback(env, src, file)) {
            state->pushnumber(x);
            state->callNoThrow(1);
        }
    };
}

doublesupplier scripting::create_number_supplier(
    const scriptenv& env,
    const std::string& src,
    const std::string& file
) {
    return [=](){
        if (processCallback(env, src, file)) {
            if (state->isfunction(-1)) {
                state->callNoThrow(0);
            }
            lua::luanumber x = state->tonumber(-1); state->pop();
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
    return [=](const int arr[], size_t len){
        if (processCallback(env, src, file)) {
            for (uint i = 0; i < len; i++) {
                state->pushinteger(arr[i]);
            }
            state->callNoThrow(len);
        }
    };
}

vec2supplier scripting::create_vec2_supplier(
    const scriptenv& env,
    const std::string& src, 
    const std::string& file
) {
    return [=](){
        if (processCallback(env, src, file)) {
            if (state->isfunction(-1)) {
                state->callNoThrow(0);
            }
            lua::luanumber y = state->tonumber(-1); state->pop();
            lua::luanumber x = state->tonumber(-1); state->pop();
            return glm::vec2(x, y);
        }
        return glm::vec2(0, 0);
    };
}
