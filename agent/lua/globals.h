#ifndef LUA_GLOBALS_H
#define LUA_GLOBALS_H

struct lua_State;

namespace ta { namespace agent {
    class application;
}}

namespace ta { namespace agent { namespace luawork {
    void init_globals( lua_State *L, application &app );
}}}

#endif // LUA_GLOBALS_H

