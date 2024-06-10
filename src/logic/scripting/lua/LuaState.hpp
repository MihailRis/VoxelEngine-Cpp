#ifndef LOGIC_SCRIPTING_LUA_STATE_HPP_
#define LOGIC_SCRIPTING_LUA_STATE_HPP_

#include "lua_commons.hpp"

#include "../scripting_functional.hpp"
#include "../../../data/dynamic.hpp"
#include "../../../delegates.hpp"

#include <string>
#include <stdexcept>

namespace lua {
    class luaerror : public std::runtime_error {
    public:
        luaerror(const std::string& message);
    };

    class LuaState {
        lua_State* mainThread;

        int nextEnvironment = 1;

        void removeLibFuncs(lua_State*, const char* libname, const char* funcs[]);
        void createLibs(lua_State* L);

        std::shared_ptr<std::string> createLambdaHandler(lua_State*);
    public:
        LuaState();
        ~LuaState();

        static std::string envName(int env);
        void loadbuffer(lua_State*, int env, const std::string& src, const std::string& file);
        int pushenv(lua_State*, int env);
        int execute(lua_State*, int env, const std::string& src, const std::string& file="<string>");
        int eval(lua_State*, int env, const std::string& src, const std::string& file="<eval>");
        void openlib(lua_State*, const std::string& name, const luaL_Reg* libfuncs);
        void addfunc(lua_State*, const std::string& name, lua_CFunction func);
        bool rename(lua_State*, const std::string& from, const std::string& to);
        void remove(lua_State*, const std::string& name);;
        runnable createRunnable(lua_State*);
        scripting::common_func createLambda(lua_State*);

        int createEnvironment(lua_State*, int parent);
        void removeEnvironment(lua_State*, int id);
        bool emitEvent(lua_State*, const std::string& name, std::function<int(lua_State*)> args=[](auto*){return 0;});
        void dumpStack(lua_State*);
        lua_State* getMainThread() const;
    };
}

#endif // LOGIC_SCRIPTING_LUA_STATE_HPP_
