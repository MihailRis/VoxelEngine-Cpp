#include "scripting_functional.hpp"

#include "coders/json.hpp"
#include "debug/Logger.hpp"
#include "util/stringutil.hpp"
#include "lua/lua_engine.hpp"

using namespace scripting;

static debug::Logger logger("scripting_func");

runnable scripting::create_runnable(
    const scriptenv& env, const std::string& src, const std::string& file
) {
    auto L = lua::get_main_thread();
    try {
        lua::loadbuffer(L, *env, src, file);
        return lua::create_runnable(L);
    } catch (const lua::luaerror& err) {
        logger.error() << err.what();
        return []() {};
    }
}

static lua::State* process_callback(
    const scriptenv& env, const std::string& src, const std::string& file
) {
    auto L = lua::get_main_thread();
    try {
        if (lua::eval(L, *env, src, file) != 0) {
            return L;
        }
    } catch (lua::luaerror& err) {
        logger.error() << err.what();
    }
    return nullptr;
}

wstringconsumer scripting::create_wstring_consumer(
    const scriptenv& env, const std::string& src, const std::string& file
) {
    return [=](const std::wstring& x) {
        if (auto L = process_callback(env, src, file)) {
            lua::pushwstring(L, x);
            lua::call_nothrow(L, 1);
        }
    };
}

wstringsupplier scripting::create_wstring_supplier(
    const scriptenv& env, const std::string& src, const std::string& file
) {
    return [=]() {
        if (auto L = process_callback(env, src, file)) {
            if (lua::isfunction(L, -1)) {
                lua::call_nothrow(L, 0);
            }
            auto str = lua::require_wstring(L, -1);
            lua::pop(L);
            return str;
        }
        return std::wstring();
    };
}

wstringchecker scripting::create_wstring_validator(
    const scriptenv& env, const std::string& src, const std::string& file
) {
    return [=](const std::wstring& x) {
        if (auto L = process_callback(env, src, file)) {
            lua::pushwstring(L, x);
            if (lua::call_nothrow(L, 1)) return lua::toboolean(L, -1);
        }
        return false;
    };
}

boolconsumer scripting::create_bool_consumer(
    const scriptenv& env, const std::string& src, const std::string& file
) {
    return [=](bool x) {
        if (auto L = process_callback(env, src, file)) {
            lua::pushboolean(L, x);
            lua::call_nothrow(L, 1);
        }
    };
}

boolsupplier scripting::create_bool_supplier(
    const scriptenv& env, const std::string& src, const std::string& file
) {
    return [=]() {
        if (auto L = process_callback(env, src, file)) {
            if (lua::isfunction(L, -1)) {
                lua::call_nothrow(L, 0);
            }
            bool x = lua::toboolean(L, -1);
            lua::pop(L);
            return x;
        }
        return false;
    };
}

doubleconsumer scripting::create_number_consumer(
    const scriptenv& env, const std::string& src, const std::string& file
) {
    return [=](double x) {
        if (auto L = process_callback(env, src, file)) {
            lua::pushnumber(L, x);
            lua::call_nothrow(L, 1);
        }
    };
}

doublesupplier scripting::create_number_supplier(
    const scriptenv& env, const std::string& src, const std::string& file
) {
    return [=]() {
        if (auto L = process_callback(env, src, file)) {
            if (lua::isfunction(L, -1)) {
                lua::call_nothrow(L, 0);
            }
            auto x = lua::tonumber(L, -1);
            lua::pop(L);
            return x;
        }
        return 0.0;
    };
}

int_array_consumer scripting::create_int_array_consumer(
    const scriptenv& env, const std::string& src, const std::string& file
) {
    return [=](const int arr[], size_t len) {
        if (auto L = process_callback(env, src, file)) {
            for (uint i = 0; i < len; i++) {
                lua::pushinteger(L, arr[i]);
            }
            lua::call_nothrow(L, len);
        }
    };
}

vec2supplier scripting::create_vec2_supplier(
    const scriptenv& env, const std::string& src, const std::string& file
) {
    return [=]() {
        if (auto L = process_callback(env, src, file)) {
            if (lua::isfunction(L, -1)) {
                lua::call_nothrow(L, 0);
            }
            auto y = lua::tonumber(L, -1);
            lua::pop(L);
            auto x = lua::tonumber(L, -1);
            lua::pop(L);
            return glm::vec2(x, y);
        }
        return glm::vec2(0, 0);
    };
}

dynamic::to_string_func scripting::create_tostring(
    const scriptenv& env, const std::string& src, const std::string& file
) {
    auto L = lua::get_main_thread();
    try {
        lua::loadbuffer(L, *env, src, file);
        lua::call(L, 0, 1);
        auto func = lua::create_lambda(L);
        return [func](const dynamic::Value& value) {
            auto result = func({value});
            return json::stringify(result, true, "  ");
        };
    } catch (const lua::luaerror& err) {
        logger.error() << err.what();
        return [](const auto& value) {
            return json::stringify(value, true, "  ");
        };
    }
}
