#ifndef LOGIC_SCRIPTING_LUA_UTIL_HPP_
#define LOGIC_SCRIPTING_LUA_UTIL_HPP_

#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "lua_commons.hpp"
#include "lua_custom_types.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

// NOTE: const std::string& used instead of string_view because c_str() needed
namespace lua {
    inline std::string LAMBDAS_TABLE = "$L";  // lambdas storage
    inline std::string CHUNKS_TABLE = "$C";   // precompiled lua chunks
    extern std::unordered_map<std::type_index, std::string> usertypeNames;
    int userdata_destructor(lua::State* L);

    std::string env_name(int env);

    template <lua_CFunction func>
    int wrap(lua_State* L) {
        int result = 0;
        try {
            result = func(L);
        }
        // transform exception with description into lua_error
        catch (std::exception& e) {
            luaL_error(L, e.what());
        }
        // Rethrow any other exception (lua error for example)
        catch (...) {
            throw;
        }
        return result;
    }

    inline void pop(lua::State* L, int n = 1) {
        lua_pop(L, n);
    }
    inline void insert(lua::State* L, int idx) {
        lua_insert(L, idx);
    }
    inline void remove(lua::State* L, int idx) {
        lua_remove(L, idx);
    }
    inline int gettop(lua::State* L) {
        return lua_gettop(L);
    }
    inline size_t objlen(lua::State* L, int idx) {
        return lua_objlen(L, idx);
    }
    inline int next(lua::State* L, int idx) {
        return lua_next(L, idx);
    }
    inline int type(lua::State* L, int idx) {
        return lua_type(L, idx);
    }
    inline const char* type_name(lua::State* L, int idx) {
        return lua_typename(L, idx);
    }
    inline int rawget(lua::State* L, int idx = -2) {
        lua_rawget(L, idx);
        return 1;
    }
    inline int rawgeti(lua::State* L, int n, int idx = -1) {
        lua_rawgeti(L, idx, n);
        return 1;
    }
    inline void rawseti(lua::State* L, int n, int idx = -2) {
        lua_rawseti(L, idx, n);
    }

    inline int createtable(lua::State* L, int narr, int nrec) {
        lua_createtable(L, narr, nrec);
        return 1;
    }

    inline bool isnil(lua::State* L, int idx) {
        return lua_isnil(L, idx);
    }

    inline bool getglobal(lua::State* L, const std::string& name) {
        lua_getglobal(L, name.c_str());
        if (isnil(L, -1)) {
            pop(L);
            return false;
        }
        return true;
    }

    inline int requireglobal(lua::State* L, const std::string& name) {
        if (getglobal(L, name)) {
            return 1;
        } else {
            throw std::runtime_error("global name " + name + " not found");
        }
    }

    inline bool hasglobal(lua::State* L, const std::string& name) {
        lua_getglobal(L, name.c_str());
        if (isnil(L, -1)) {
            pop(L);
            return false;
        }
        pop(L);
        return true;
    }

    // function wrappers with number of pushed values as return value

    inline int pushnil(lua::State* L) {
        lua_pushnil(L);
        return 1;
    }

    inline int pushinteger(lua::State* L, lua::Integer x) {
        lua_pushinteger(L, x);
        return 1;
    }

    inline int pushnumber(lua::State* L, lua::Number x) {
        lua_pushnumber(L, x);
        return 1;
    }

    template <int n>
    inline int pushvec(lua::State* L, glm::vec<n, float> vec) {
        createtable(L, n, 0);
        for (int i = 0; i < n; i++) {
            pushnumber(L, vec[i]);
            rawseti(L, i + 1);
        }
        return 1;
    }

    template <int n>
    inline int pushivec(lua::State* L, glm::vec<n, int> vec) {
        createtable(L, n, 0);
        for (int i = 0; i < n; i++) {
            pushinteger(L, vec[i]);
            rawseti(L, i + 1);
        }
        return 1;
    }

    inline int pushivec3_stack(
        lua::State* L, lua::Integer x, lua::Integer y, lua::Integer z
    ) {
        pushinteger(L, x);
        pushinteger(L, y);
        pushinteger(L, z);
        return 3;
    }

    inline int pushivec3_stack(lua::State* L, glm::ivec3 vec) {
        pushinteger(L, vec.x);
        pushinteger(L, vec.y);
        pushinteger(L, vec.z);
        return 3;
    }

    inline int pushvec3_stack(lua::State* L, glm::vec3 vec) {
        pushnumber(L, vec.x);
        pushnumber(L, vec.y);
        pushnumber(L, vec.z);
        return 3;
    }
    inline int pushvec4_stack(lua::State* L, glm::vec4 vec) {
        pushnumber(L, vec.x);
        pushnumber(L, vec.y);
        pushnumber(L, vec.z);
        pushnumber(L, vec.w);
        return 4;
    }

    inline void setmetatable(lua::State* L, int idx = -2) {
        lua_setmetatable(L, idx);
    }
    inline int pushvalue(lua::State* L, int idx) {
        lua_pushvalue(L, idx);
        return 1;
    }
    inline int pushvec2(lua::State* L, glm::vec2 vec) {
        return pushvec(L, vec);
    }

    inline int pushvec3(lua::State* L, glm::vec3 vec) {
        return pushvec(L, vec);
    }

    inline int pushvec4(lua::State* L, glm::vec4 vec) {
        return pushvec(L, vec);
    }
    inline int pushcolor(lua::State* L, glm::vec4 vec) {
        return pushivec(L, glm::ivec4(vec * 255.0f));
    }

    inline int pushquat(lua::State* L, glm::quat quat) {
        createtable(L, 4, 0);
        for (size_t i = 0; i < 4; i++) {
            pushnumber(L, quat[i]);
            rawseti(L, i + 1);
        }
        return 1;
    }
    inline int setquat(lua::State* L, int idx, glm::quat quat) {
        pushvalue(L, idx);
        for (int i = 0; i < 4; i++) {
            pushnumber(L, quat[i]);
            rawseti(L, i + 1);
        }
        return 1;
    }
    inline int pushmat4(lua::State* L, glm::mat4 matrix) {
        createtable(L, 16, 0);
        for (uint y = 0; y < 4; y++) {
            for (uint x = 0; x < 4; x++) {
                uint i = y * 4 + x;
                pushnumber(L, matrix[y][x]);
                rawseti(L, i + 1);
            }
        }
        return 1;
    }
    /// @brief pushes matrix table to the stack and updates it with glm matrix
    inline int setmat4(lua::State* L, int idx, glm::mat4 matrix) {
        pushvalue(L, idx);
        for (uint y = 0; y < 4; y++) {
            for (uint x = 0; x < 4; x++) {
                uint i = y * 4 + x;
                pushnumber(L, matrix[y][x]);
                rawseti(L, i + 1);
            }
        }
        return 1;
    }
    template <int n>
    inline int setvec(lua::State* L, int idx, glm::vec<n, float> vec) {
        pushvalue(L, idx);
        for (int i = 0; i < n; i++) {
            pushnumber(L, vec[i]);
            rawseti(L, i + 1);
        }
        return 1;
    }
    inline int pushcfunction(lua::State* L, lua_CFunction func) {
        lua_pushcfunction(L, func);
        return 1;
    }
    inline int pushstring(lua::State* L, const std::string& str) {
        lua_pushstring(L, str.c_str());
        return 1;
    }

    template <typename... Args>
    inline int pushfstring(lua_State* L, const char* fmt, Args... args) {
        lua_pushfstring(L, fmt, args...);
        return 1;
    }

    int pushwstring(lua::State* L, const std::wstring& str);

    inline int pushboolean(lua::State* L, bool value) {
        lua_pushboolean(L, value);
        return 1;
    }
    inline int pushglobals(lua::State* L) {
        return pushvalue(L, LUA_GLOBALSINDEX);
    }
    inline bool isnoneornil(lua::State* L, int idx) {
        return lua_isnoneornil(L, idx);
    }
    inline bool isboolean(lua::State* L, int idx) {
        return lua_isboolean(L, idx);
    }
    inline bool isnumber(lua::State* L, int idx) {
        return lua_isnumber(L, idx);
    }
    inline bool isstring(lua::State* L, int idx) {
        return lua_isstring(L, idx);
    }
    inline bool istable(lua::State* L, int idx) {
        return lua_istable(L, idx);
    }
    inline bool isfunction(lua::State* L, int idx) {
        return lua_isfunction(L, idx);
    }
    inline bool isuserdata(lua::State* L, int idx) {
        return lua_isuserdata(L, idx);
    }
    inline void setfield(lua::State* L, const std::string& name, int idx = -2) {
        lua_setfield(L, idx, name.c_str());
    }
    inline bool toboolean(lua::State* L, int idx) {
        return lua_toboolean(L, idx);
    }
    inline lua::Integer tointeger(lua::State* L, int idx) {
        return lua_tointeger(L, idx);
    }
    inline lua::Number tonumber(lua::State* L, int idx) {
        return lua_tonumber(L, idx);
    }
    inline const char* tostring(lua::State* L, int idx) {
        return lua_tostring(L, idx);
    }
    inline const void* topointer(lua::State* L, int idx) {
        return lua_topointer(L, idx);
    }
    inline void setglobal(lua::State* L, const std::string& name) {
        lua_setglobal(L, name.c_str());
    }
    template <class T>
    inline T* touserdata(lua::State* L, int idx) {
        if (void* rawptr = lua_touserdata(L, idx)) {
            return static_cast<T*>(rawptr);
        }
        return nullptr;
    }
    template <class T, typename... Args>
    inline int newuserdata(lua::State* L, Args&&... args) {
        const auto& found = usertypeNames.find(typeid(T));
        void* ptr = lua_newuserdata(L, sizeof(T));
        new (ptr) T(args...);

        if (found == usertypeNames.end()) {
            log_error(
                "usertype is not registred: " + std::string(typeid(T).name())
            );
        } else if (getglobal(L, found->second)) {
            setmetatable(L);
        }
        return 1;
    }

    template <class T, lua_CFunction func>
    inline void newusertype(lua::State* L, const std::string& name) {
        usertypeNames[typeid(T)] = name;
        func(L);

        pushcfunction(L, userdata_destructor);
        setfield(L, "__gc");

        setglobal(L, name);
    }

    template <int n>
    inline glm::vec<n, float> tovec(lua::State* L, int idx) {
        pushvalue(L, idx);
        if (!istable(L, idx) || objlen(L, idx) < n) {
            throw std::runtime_error(
                "value must be an array of " + std::to_string(n) + " numbers"
            );
        }
        glm::vec<n, float> vec;
        for (int i = 0; i < n; i++) {
            rawgeti(L, i + 1);
            vec[i] = tonumber(L, -1);
            pop(L);
        }
        pop(L);
        return vec;
    }

    inline glm::vec2 tovec2(lua::State* L, int idx) {
        pushvalue(L, idx);
        if (!istable(L, idx) || objlen(L, idx) < 2) {
            throw std::runtime_error("value must be an array of two numbers");
        }
        rawgeti(L, 1);
        auto x = tonumber(L, -1);
        pop(L);
        rawgeti(L, 2);
        auto y = tonumber(L, -1);
        pop(L);
        pop(L);
        return glm::vec2(x, y);
    }
    inline glm::vec3 tovec3(lua::State* L, int idx) {
        pushvalue(L, idx);
        if (!istable(L, idx) || objlen(L, idx) < 3) {
            throw std::runtime_error("value must be an array of three numbers");
        }
        rawgeti(L, 1);
        auto x = tonumber(L, -1);
        pop(L);
        rawgeti(L, 2);
        auto y = tonumber(L, -1);
        pop(L);
        rawgeti(L, 3);
        auto z = tonumber(L, -1);
        pop(L);
        pop(L);
        return glm::vec3(x, y, z);
    }
    inline glm::vec4 tovec4(lua::State* L, int idx) {
        pushvalue(L, idx);
        if (!istable(L, idx) || objlen(L, idx) < 4) {
            throw std::runtime_error("value must be an array of four numbers");
        }
        rawgeti(L, 1);
        auto x = tonumber(L, -1);
        pop(L);
        rawgeti(L, 2);
        auto y = tonumber(L, -1);
        pop(L);
        rawgeti(L, 3);
        auto z = tonumber(L, -1);
        pop(L);
        rawgeti(L, 4);
        auto w = tonumber(L, -1);
        pop(L);
        pop(L);
        return glm::vec4(x, y, z, w);
    }

    inline glm::quat toquat(lua::State* L, int idx) {
        pushvalue(L, idx);
        if (!istable(L, idx) || objlen(L, idx) < 4) {
            throw std::runtime_error("value must be an array of four numbers");
        }
        rawgeti(L, 1);
        auto x = tonumber(L, -1);
        pop(L);
        rawgeti(L, 2);
        auto y = tonumber(L, -1);
        pop(L);
        rawgeti(L, 3);
        auto z = tonumber(L, -1);
        pop(L);
        rawgeti(L, 4);
        auto w = tonumber(L, -1);
        pop(L);
        pop(L);
        return glm::quat(x, y, z, w);
    }

    inline glm::vec3 tovec3_stack(lua::State* L, int idx) {
        return glm::vec3(
            tonumber(L, idx), tonumber(L, idx + 1), tonumber(L, idx + 2)
        );
    }
    inline glm::vec4 tovec4_stack(lua::State* L, int idx) {
        return glm::vec4(
            tonumber(L, idx),
            tonumber(L, idx + 1),
            tonumber(L, idx + 2),
            tonumber(L, idx + 3)
        );
    }
    inline glm::mat4 tomat4(lua::State* L, int idx) {
        pushvalue(L, idx);
        if (!istable(L, idx) || objlen(L, idx) < 16) {
            throw std::runtime_error("value must be an array of 16 numbers");
        }
        glm::mat4 matrix;
        for (uint y = 0; y < 4; y++) {
            for (uint x = 0; x < 4; x++) {
                uint i = y * 4 + x;
                rawgeti(L, i + 1);
                matrix[y][x] = static_cast<float>(tonumber(L, -1));
                pop(L);
            }
        }
        pop(L);
        return matrix;
    }

    inline glm::vec4 tocolor(lua::State* L, int idx) {
        pushvalue(L, idx);
        if (!istable(L, -1) || objlen(L, idx) < 4) {
            throw std::runtime_error("RGBA array required");
        }
        rawgeti(L, 1);
        auto r = tonumber(L, -1);
        pop(L);
        rawgeti(L, 2);
        auto g = tonumber(L, -1);
        pop(L);
        rawgeti(L, 3);
        auto b = tonumber(L, -1);
        pop(L);
        rawgeti(L, 4);
        auto a = tonumber(L, -1);
        pop(L);
        pop(L);
        return glm::vec4(r / 255, g / 255, b / 255, a / 255);
    }

    int pushvalue(lua::State*, const dynamic::Value& value);
    dynamic::Value tovalue(lua::State*, int idx);

    inline bool getfield(lua::State* L, const std::string& name, int idx = -1) {
        lua_getfield(L, idx, name.c_str());
        if (isnil(L, -1)) {
            pop(L);
            return false;
        }
        return true;
    }

    inline bool hasfield(lua::State* L, const std::string& name, int idx = -1) {
        lua_getfield(L, idx, name.c_str());
        if (isnil(L, -1)) {
            pop(L);
            return false;
        }
        pop(L);
        return true;
    }

    inline const char* require_string(lua::State* L, int idx) {
        if (!isstring(L, idx)) {
            throw luaerror("string expected at " + std::to_string(idx));
        }
        return tostring(L, idx);
    }

    std::wstring require_wstring(lua::State*, int idx);

    inline bool rename(
        lua::State* L, const std::string& from, const std::string& to
    ) {
        getglobal(L, from);
        if (isnil(L, -1)) {
            pop(L, 1);
            return false;
        }
        setglobal(L, to);

        // remove previous
        pushnil(L);
        setglobal(L, from);
        return true;
    }

    inline void remove(lua::State* L, const std::string& name) {
        pushnil(L);
        setglobal(L, name);
    }

    inline int setfenv(lua::State* L, int idx = -2) {
        return lua_setfenv(L, idx);
    }

    inline void loadbuffer(
        lua::State* L, int env, const std::string& src, const std::string& file
    ) {
        if (luaL_loadbuffer(L, src.c_str(), src.length(), file.c_str())) {
            throw luaerror(tostring(L, -1));
        }
        if (env && getglobal(L, env_name(env))) {
            lua_setfenv(L, -2);
        }
    }

    inline void store_in(
        lua::State* L, const std::string& tableName, const std::string& name
    ) {
        if (getglobal(L, tableName)) {
            pushvalue(L, -2);
            setfield(L, name);
            pop(L, 2);
        } else {
            throw std::runtime_error("table " + tableName + " not found");
        }
    }

    inline int get_from(
        lua::State* L,
        const std::string& tableName,
        const std::string& name,
        bool required = false
    ) {
        if (getglobal(L, tableName)) {
            if (getfield(L, name)) {
                return 1;
            } else if (required) {
                throw std::runtime_error(
                    "table " + tableName + " has no member " + name
                );
            }
            return 0;
        } else {
            throw std::runtime_error("table " + tableName + " not found");
        }
    }

    int call(lua::State*, int argc, int nresults = -1);
    int call_nothrow(lua::State*, int argc, int nresults = 1);

    inline int eval(
        lua::State* L,
        int env,
        const std::string& src,
        const std::string& file = "<eval>"
    ) {
        auto srcText = "return " + src;
        loadbuffer(L, env, srcText, file);
        return call(L, 0);
    }

    inline int execute(
        lua::State* L,
        int env,
        const std::string& src,
        const std::string& file = "<eval>"
    ) {
        loadbuffer(L, env, src, file);
        return call_nothrow(L, 0);
    }

    runnable create_runnable(lua::State*);
    scripting::common_func create_lambda(lua::State*);

    inline int pushenv(lua::State* L, int env) {
        if (getglobal(L, env_name(env))) {
            return 1;
        }
        return 0;
    }
    int create_environment(lua::State*, int parent);
    void removeEnvironment(lua::State*, int id);
    void dump_stack(lua::State*);

    inline void addfunc(
        lua::State* L, const std::string& name, lua_CFunction func
    ) {
        pushcfunction(L, func);
        setglobal(L, name);
    }

    inline void openlib(
        lua::State* L, const std::string& name, const luaL_Reg* libfuncs
    ) {
        createtable(L, 0, 0);
        luaL_setfuncs(L, libfuncs, 0);
        setglobal(L, name);
    }
}

#endif  // LOGIC_SCRIPTING_LUA_UTIL_HPP_
