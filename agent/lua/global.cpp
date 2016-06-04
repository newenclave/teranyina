#include "common/lua-wrapper/lua-wrapper.hpp"
#include "globals.h"

#include "subsystems/subsys-list.hxx"
#include "application.h"

namespace ta { namespace agent { namespace luawork {

    namespace  {
        const std::string table_name = "ta";
    }

    void init_globals( lua_State *L, application &app )
    {
        lua::state ls( L );
    }

}}}
