
#include "subsys-lua.h"
#include "../application.h"
#include "lua/globals.h"

#include "protocol/scripting.pb.h"

#include "boost/asio.hpp"

#if LUA_FOUND
#include "common/lua-wrapper/lua-wrapper.hpp"
#endif

#define LOG(lev) log_(lev, "lua")
#define LOGINF   LOG(level::info)
#define LOGDBG   LOG(level::debug)
#define LOGERR   LOG(level::error)
#define LOGWRN   LOG(level::warning)

namespace ta { namespace agent { namespace subsys {

    namespace {

        const std::string subsys_name( "lua" );

        using level = agent::logger::level;
        using connection_wptr = vtrc::common::connection_iface_wptr;

        class svc_impl: public proto::scripting::instance {

            lua::impl       *impl_;
            connection_wptr  cl_;

        public:

            svc_impl( lua::impl *imp, connection_wptr cl )
                :impl_(imp)
                ,cl_(cl)
            { }

            static std::string name( )
            {
                return proto::scripting::instance::descriptor( )->full_name( );
            }
        };

    }

    struct lua::impl {

        application     *app_;
        agent::logger   &log_;

#if LUA_FOUND
        const std::string conf_;
        ta::lua::state    state_;
#endif
        boost::asio::io_service::strand dispatcher_;

        impl( application *app, const std::string &conf )
            :app_(app)
            ,log_(app_->get_logger( ))
            ,conf_(conf)
            ,dispatcher_(app_->get_io_service( ))
        { }

        void init_file( )
        {
            LOGINF << "Init config file " << conf_;
            try {
                if( !conf_.empty( ) ) {
                    state_.check_call_error( state_.load_file( conf_.c_str( )));
                    luawork::load_config( state_.get_state( ), app_, "config" );
                }
            } catch( const std::exception &ex ) {
                LOGERR << "Failed to init config '" << conf_ << "'. "
                       << ex.what( );
            }
        }

        application::service_wrapper_sptr create_service(
                                      vtrc::common::connection_iface_wptr cl )
        {
            /// is always !cl.expired( ) here
            if( app_->is_ctrl_connection( cl.lock( ).get( ) ) ) {
                auto inst = std::make_shared<svc_impl>( this, cl );
                return app_->wrap_service( cl, inst );
            } else {
                return application::service_wrapper_sptr( );
            }
        }

        void reg_creator( )
        {
            app_->register_service_factory( svc_impl::name( ),
                [this]( agent::application */*app*/, connection_wptr c ) {
                    return create_service( c );
                });
        }

        void unreg_creator( )
        {
            app_->unregister_service_factory( svc_impl::name( ) );
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
        impl_->LOGINF << "Init";
        impl_->LOGINF << LUA_COPYRIGHT;
        impl_->LOGINF << LUA_AUTHORS;
        luawork::init_globals( impl_->state_.get_state( ), impl_->app_ );

    }

    void lua::start( )
    {
        impl_->reg_creator( );
        impl_->dispatcher_.post( [this]( ) {
            impl_->init_file( );
        } );
        impl_->LOGINF << "Started";
    }

    void lua::stop( )
    {
        impl_->unreg_creator( );
        impl_->LOGINF << "Stopped";
    }

}}}

    
