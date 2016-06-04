
#include "subsys-lua.h"
#include "../application.h"

#define LOG(lev) log_(lev) << "[lua] "
#define LOGINF   LOG(level::info)
#define LOGDBG   LOG(level::debug)
#define LOGERR   LOG(level::error)
#define LOGWRN   LOG(level::warning)

namespace ta { namespace agent { namespace subsys {

    namespace {

        const std::string subsys_name( "lua" );

        using level = agent::logger::level;

        application::service_wrapper_sptr create_service(
                                      ta::agent::application * /*app*/,
                                      vtrc::common::connection_iface_wptr cl )
        {
            ///auto inst = std::make_shared<impl_type_here>( app, cl );
            ///return app->wrap_service( cl, inst );

            return application::service_wrapper_sptr( );
        }
    }

    struct lua::impl {

        application     *app_;
        agent::logger   &log_;

        impl( application *app )
            :app_(app)
            ,log_(app_->get_logger( ))
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

    lua::lua( application *app )
        :impl_(new impl(app))
    { }

    lua::~lua( )
    {
        delete impl_;
    }

    /// static
    lua::shared_type lua::create( application *app )
    {
        shared_type new_inst(new lua(app));
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
        impl_->LOGINF << "Started";
    }

    void lua::stop( )
    {
        impl_->LOGINF << "Stopped";
    }

}}}

    