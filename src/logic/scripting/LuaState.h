#ifndef LOGIC_SCRIPTING_LUA_STATE_H_
#define LOGIC_SCRIPTING_LUA_STATE_H_

#include <lua.hpp>
#include <string>
#include <stdexcept>

namespace lua {
    using luaint = lua_Integer;
    using luanumber = lua_Number;

    class luaerror : public std::runtime_error {
    public:
        luaerror(const std::string& message);
    };

    class LuaState {
        lua_State* L;
        int nextNamespace = 1;
        int currentNamespace = 0;

        void logError(const std::string& text);
        void initNamespace();
    public:
        LuaState();
        ~LuaState();

        void loadbuffer(const std::string& src, const std::string& file);
        int gettop() const;
        int pushivec3(luaint x, luaint y, luaint z);
        int pushinteger(luaint x);
        int pushnumber(luanumber x);
        int pushstring(const std::string& str);
        bool toboolean(int index);
        luaint tointeger(int index);
        int call(int argc);
        int callNoThrow(int argc);
        int execute(const std::string& src, const std::string& file="<string>");
        int eval(const std::string& src, const std::string& file="<eval>");
        void openlib(const std::string& name, const luaL_Reg* libfuncs, int nup);
        void addfunc(const std::string& name, lua_CFunction func);
        bool getglobal(const std::string& name);
        void setglobal(const std::string& name);
        bool rename(const std::string& from, const std::string& to);
        void remove(const std::string& name);;
        void createFuncs();
        int createNamespace();
        void setNamespace(int id);

        const std::string storeAnonymous();
    };
}

#endif // LOGIC_SCRIPTING_LUA_STATE_H_
