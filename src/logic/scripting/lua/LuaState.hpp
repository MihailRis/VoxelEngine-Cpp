#ifndef LOGIC_SCRIPTING_LUA_STATE_HPP_
#define LOGIC_SCRIPTING_LUA_STATE_HPP_

#include "lua_commons.hpp"

#include "../scripting_functional.hpp"
#include "../../../data/dynamic.hpp"
#include "../../../delegates.hpp"

#include <string>
#include <stdexcept>

namespace lua {
    class LuaState {
        lua_State* mainThread;

        void removeLibFuncs(lua_State*, const char* libname, const char* funcs[]);
        void createLibs(lua_State* L);
    public:
        LuaState();
        ~LuaState();

        void openlib(lua_State*, const std::string& name, const luaL_Reg* libfuncs);
        void addfunc(lua_State*, const std::string& name, lua_CFunction func);

        bool emitEvent(lua_State*, const std::string& name, std::function<int(lua_State*)> args=[](auto*){return 0;});
        lua_State* getMainThread() const;
    };
}

#endif // LOGIC_SCRIPTING_LUA_STATE_HPP_
