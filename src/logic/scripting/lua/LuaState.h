#ifndef LOGIC_SCRIPTING_LUA_STATE_H_
#define LOGIC_SCRIPTING_LUA_STATE_H_

#include "lua_commons.h"
#include <string>
#include <stdexcept>

#include "../../../data/dynamic.h"

#ifndef LUAJIT_VERSION
#error LuaJIT required
#endif

namespace lua {
    class luaerror : public std::runtime_error {
    public:
        luaerror(const std::string& message);
    };

    class LuaState {
        lua_State* L;
        int nextEnvironment = 1;

        void logError(const std::string& text);
        void removeLibFuncs(const char* libname, const char* funcs[]);
        void createLibs();
    public:
        LuaState();
        ~LuaState();

        static const std::string envName(int env);
        void loadbuffer(int env, const std::string& src, const std::string& file);
        int gettop() const;
        int pushivec3(luaint x, luaint y, luaint z);
        int pushinteger(luaint x);
        int pushnumber(luanumber x);
        int pushboolean(bool x);
        int pushstring(const std::string& str);
        int pushenv(int env);
        int pushvalue(int idx);
        int pushvalue(const dynamic::Value& value);
        int pushnil();
        int pushglobals();
        void pop(int n=1);
        bool getfield(const std::string& name, int idx = -1);
        void setfield(const std::string& name, int idx = -2);
        bool toboolean(int idx);
        luaint tointeger(int idx);
        luanumber tonumber(int idx);
        dynamic::Value tovalue(int idx);
        const char* tostring(int idx);
        bool isstring(int idx);
        bool isfunction(int idx);
        int call(int argc);
        int callNoThrow(int argc);
        int execute(int env, const std::string& src, const std::string& file="<string>");
        int eval(int env, const std::string& src, const std::string& file="<eval>");
        void openlib(const std::string& name, const luaL_Reg* libfuncs, int nup);
        void addfunc(const std::string& name, lua_CFunction func);
        bool getglobal(const std::string& name);
        void setglobal(const std::string& name);
        bool hasglobal(const std::string& name);
        bool rename(const std::string& from, const std::string& to);
        void remove(const std::string& name);;
        int createEnvironment(int parent);
        void removeEnvironment(int id);
        const std::string storeAnonymous();

        void dumpStack();
    };
}

#endif // LOGIC_SCRIPTING_LUA_STATE_H_
