#include "common/lua-wrapper/lua-wrapper.hpp"
#include "common/lua-wrapper/lua-objects.hpp"
#include "common/lua-wrapper/lua-type-wrapper.hpp"
#include "globals.h"

#include "subsystems/subsys-list.hxx"
#include "application.h"

#define LOG(lev) app->get_logger( )(lev) << "[lua scpt] "
#define LOGINF   LOG(level::info)
#define LOGDBG   LOG(level::debug)
#define LOGERR   LOG(level::error)
#define LOGWRN   LOG(level::warning)

#define LUA_CALL_PREFIX( L ) lua::state ls( L );        \
                             auto app = get_app( ls )

namespace ta { namespace agent { namespace luawork {

    namespace  {
        const char *table_name        = "ta";
        const char *app_name          = "tahide.application";
        using namespace ::ta::lua;

        application *get_app( lua::state &ls )
        {
            return static_cast<application *>(ls.get<void *>( app_name ));
        }

        int log_write( lua_State *L )
        {
            LUA_CALL_PREFIX( L );

            try {
                auto level = logger::str2level(
                            ls.get<std::string>( 1 ).c_str( ) );
                 LOG(level) << "Hello!";
            } catch( const std::exception &ex ) {
                ls.push( ex.what( ) );
                return 1;
            }

            return 0;
        }

    }


    void init_globals( lua_State *L, application *app )
    {
        lua::state ls( L );
        ls.set( app_name, app );
        objects::table t;
        t.add( "log", objects::new_function( &log_write ) );
        ls.set_object( table_name, &t );
    }

}}}
