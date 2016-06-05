#include "common/lua-wrapper/lua-wrapper.hpp"
#include "common/lua-wrapper/lua-objects.hpp"
#include "common/lua-wrapper/lua-type-wrapper.hpp"
#include "globals.h"

#include "subsystems/subsys-list.hxx"
#include "application.h"

#define LOG(lev) app->get_logger( )(lev, "lua scpt")
#define LOGINF   LOG(common::logger::level::info)
#define LOGDBG   LOG(common::logger::level::debug)
#define LOGERR   LOG(common::logger::level::error)
#define LOGWRN   LOG(common::logger::level::warning)

#define LUA_CALL_PREFIX( L ) lua::state ls( L )

#define LUA_CALL_PREFIX_APP( L ) lua::state ls( L );        \
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

        int log_write_impl( lua_State *L, bool split )
        {
            LUA_CALL_PREFIX_APP( L );

            try {
                auto level = logger::str2level(
                             ls.get_opt<std::string>( 1, "inf" ).c_str( ) );
                std::ostringstream oss;
                if( split ) {
                    bool next = false;
                    for( int i=2; i<=ls.get_top( ); ++i ) {
                        oss << ls.get_object( i )->str( );
                        if( next ) {
                            oss << " ";
                        }
                        next = true;
                    }
                } else {
                    for( int i=2; i<=ls.get_top( ); ++i ) {
                        oss << ls.get_object( i )->str( );
                    }
                }
                LOG(level) << oss.str( );
            } catch( const std::exception &ex ) {
                ls.push( ex.what( ) );
                return 1;
            }

            return 0;
        }

        int log_write( lua_State *L )
        {
            return log_write_impl( L, false );
        }

        int log_write2( lua_State *L )
        {
            return log_write_impl( L, true );
        }

        int add_listener( lua_State *L )
        {
            LUA_CALL_PREFIX_APP( L );
            for( int i=0; i<ls.get_top( ); ++i ) {
                auto o = ls.get_object( i + 1 );
                auto name  = object_by_path( L, o.get( ), "name" );
                auto ossl   = object_by_path( L, o.get( ), "ssl" );
                auto odummy = object_by_path( L, o.get( ), "dummy" );
                if( name->type_id( ) == objects::base::TYPE_STRING ) {
                    LOGINF << "Add endpoint: " << name->str( );
                    bool ssl = (ossl ? ossl->num( )     != 0 : false );
                    bool dum = (odummy ? odummy->num( ) != 0 : false );
                    app->subsystem<subsys::listeners>( ).add_listener(
                                name->str( ), ssl, dum );
                } else {
                    LOGERR << "Invalid endpoint: " << name->str( );
                }
            }
            //app->subsystem<subsys::listeners>( ).
            return 0;
        }

    }

    void init_globals( lua_State *L, application *app )
    {
        lua::state ls( L );
        ls.set( app_name, app );
        objects::table t;

        using namespace ::ta::lua::objects;

        t.add( "log", new_function( &log_write ) );
        t.add( "logs", new_function( &log_write2 ) );
        t.add( "listener", new_table( )->add(
                   "add", new_function( &add_listener ) )
              );

        ls.set_object( table_name, &t );
    }

}}}
