
#include "subsys-lua.h"
#include "../application.h"

#if LUA_FOUND
#include "common/lua-wrapper/lua-wrapper.hpp"
#endif

#define LOG(lev) log_(lev) << "[     lua] "
#define LOGINF   LOG(level::info)
#define LOGDBG   LOG(level::debug)
#define LOGERR   LOG(level::error)
#define LOGWRN   LOG(level::warning)

namespace ta { namespace agent { namespace subsys {

    namespace {

        const std::string subsys_name( "lua" );

        using level = agent::logger::level;

//        application::service_wrapper_sptr create_service(
//                                      ta::agent::application * /*app*/,
//                                      vtrc::common::connection_iface_wptr cl )
//        {
//            ///auto inst = std::make_shared<impl_type_here>( app, cl );
//            ///return app->wrap_service( cl, inst );

//            return application::service_wrapper_sptr( );
//        }
    }

    struct lua::impl {

        application     *app_;
        agent::logger   &log_;

#if LUA_FOUND
        const std::string conf_;
        ta::lua::state    state_;
#endif

        impl( application *app, const std::string &conf )
            :app_(app)
            ,log_(app_->get_logger( ))
            ,conf_(conf)
        { }

        void reg_creator( const std::string &name,
                          application::service_getter_type func )
        {
            app_->register_service_factory( name, func );
        }

        void unreg_creator( const std::string &name )
        {
            app_->unregister_service_factory( name );
        }
    };

    lua::lua( application *app, const std::string &conf )
        :impl_(new impl(app, conf))
    { }

    lua::~lua( )
    {
        delete impl_;
    }

    /// static
    lua::shared_type lua::create( application *app, const std::string &conf )
    {
        shared_type new_inst(new lua(app, conf));
        return new_inst;
    }

    const std::string &lua::name( ) const noexcept
    {
        return subsys_name;
    }

    void lua::init( )
    {

    }

    void lua::start( )
    {
        impl_->LOGINF << LUA_COPYRIGHT;
        impl_->LOGINF << LUA_AUTHORS;
        impl_->LOGINF << "Started.";
    }

    void lua::stop( )
    {
        impl_->LOGINF << "Stopped";
    }

}}}

    
