#include "engine.hpp"
#include "window/Window.hpp"
#include "api_lua.hpp"

static void setfield(lua_State *L, const char *key, int value) {
    lua_pushinteger(L, value);
    lua_setfield(L, -2, key);
}

static void setboolfield(lua_State *L, const char *key, int value) {
    if (value < 0) /* undefined? */
        return;    /* does not set field */
    lua_pushboolean(L, value);
    lua_setfield(L, -2, key);
}

static int getboolfield(lua_State *L, const char *key) {
    int res;
    lua_getfield(L, -1, key);
    res = lua_isnil(L, -1) ? -1 : lua_toboolean(L, -1);
    lua_pop(L, 1);
    return res;
}

static int getfield(lua_State *L, const char *key, int d) {
    int res;
    lua_getfield(L, -1, key);
    if (lua_isnumber(L, -1)) {
        res = (int)lua_tointeger(L, -1);
    } else {
        if (d < 0)
            throw std::runtime_error("field " + std::string(key) + " missing in date table");
        res = d;
    }
    lua_pop(L, 1);
    return res;
}

static int l_os_date(lua::State* L) {
    const char *s = luaL_optstring(L, 1, "%c");
    time_t t = luaL_opt(L, (time_t)luaL_checknumber, 2, time(NULL));
    struct tm *stm;
    if (*s == '!') { /* UTC? */
        s++;         /* Skip '!' */
        stm = gmtime(&t);
    } else {
        stm = localtime(&t);
    }
    if (stm == NULL) { /* Invalid date? */
        lua_pushnil(L); //setnilV(L->top++);
    } else if (strcmp(s, "*t") == 0) {
        lua_createtable(L, 0, 9); /* 9 = number of fields */
        setfield(L, "sec", stm->tm_sec);
        setfield(L, "min", stm->tm_min);
        setfield(L, "hour", stm->tm_hour);
        setfield(L, "day", stm->tm_mday);
        setfield(L, "month", stm->tm_mon + 1);
        setfield(L, "year", stm->tm_year + 1900);
        setfield(L, "wday", stm->tm_wday + 1);
        setfield(L, "yday", stm->tm_yday + 1);
        setboolfield(L, "isdst", stm->tm_isdst);
    } else if (*s) {
        char buf[32] = {};
		size_t len = strftime(buf, sizeof(buf), s, stm);
		lua_pushlstring(L, buf, len);
    } else {
        lua_pushstring(L, ""); //setstrV(L, L->top++, &G(L)->strempty);
    }
    return 1;
}

static int l_os_time(lua::State* L) {
    time_t t;
    if (lua_isnoneornil(L, 1)) { /* called without args? */
        t = time(NULL);          /* get current time */
    } else {
        struct tm ts;
        luaL_checktype(L, 1, LUA_TTABLE);
        lua_settop(L, 1); /* make sure table is at the top */
        ts.tm_sec = getfield(L, "sec", 0);
        ts.tm_min = getfield(L, "min", 0);
        ts.tm_hour = getfield(L, "hour", 12);
        ts.tm_mday = getfield(L, "day", -1);
        ts.tm_mon = getfield(L, "month", -1) - 1;
        ts.tm_year = getfield(L, "year", -1) - 1900;
        ts.tm_isdst = getboolfield(L, "isdst");
        t = mktime(&ts);
    }
    if (t == (time_t)(-1))
        lua_pushnil(L);
    else
        lua_pushnumber(L, (lua_Number)t);
    return 1;
}

static int l_os_difftime(lua::State* L) {
    lua_pushnumber(L, difftime((time_t)(luaL_checknumber(L, 1)),
                (time_t)(luaL_optnumber(L, 2, (lua_Number)0))));
    return 1;
}

static int l_time_uptime(lua::State* L) {
    return lua::pushnumber(L, Window::time());
}

static int l_time_delta(lua::State* L) {
    return lua::pushnumber(L, scripting::engine->getDelta());
}

const luaL_Reg timelib[] = {
    {"date", lua::wrap<l_os_date>},
	{"time", lua::wrap<l_os_time>},
	{"difftime", lua::wrap<l_os_difftime>},

    {"uptime", lua::wrap<l_time_uptime>},
    {"delta", lua::wrap<l_time_delta>},
    {NULL, NULL}};
