
#include "subsys-lua.h"
#include "../application.h"
#include "lua/globals.h"

#include "protocol/scripting.pb.h"

#include "boost/asio.hpp"

#include "vtrc-common/vtrc-closure-holder.h"
#include "vtrc-common/vtrc-exception.h"

#if LUA_FOUND
#include "common/lua-wrapper/lua-wrapper.hpp"
#endif

#include "google/protobuf/descriptor.h"

#define LOG(lev) log_(lev, "lua")
#define LOGINF   LOG(level::info)
#define LOGDBG   LOG(level::debug)
#define LOGERR   LOG(level::error)
#define LOGWRN   LOG(level::warning)

namespace ta { namespace agent { namespace subsys {

    namespace {

        const std::string subsys_name( "lua" );

        namespace vcomm = vtrc::common;
        namespace gpb = google::protobuf;

        using level           = agent::logger::level;
        using connection_wptr = vtrc::common::connection_iface_wptr;
        using connection_sptr = vtrc::common::connection_iface_sptr;

        class lua_service_wrapper: public application::service_wrapper_impl {

            boost::asio::io_service::strand dispatcher_;

        public:

            typedef application::service_wrapper_impl super_type;
            using service_sptr = super_type::service_sptr;

            lua_service_wrapper( application *app, connection_wptr c,
                                 service_sptr serv )
                :super_type(app, c, serv)
                ,dispatcher_(app->get_rpc_service( ))
            { }

            void call_method( const method_type *method,
                                 google::protobuf::RpcController* ctrl,
                                 const google::protobuf::Message* req,
                                 google::protobuf::Message* res,
                                 google::protobuf::Closure* done ) override
            {
                auto cl = client( );
                dispatcher_.post( [this, method, ctrl, req, res, done, cl]( ) {
                    auto lck = cl.lock( );
                    vcomm::closure_holder _(done);
                    if( lck ) {
                        service( )->CallMethod( method, ctrl, req, res, done );
                    }
                });
            }

        };

        class svc_impl: public proto::scripting::instance {

            lua::impl       *impl_;
            connection_wptr  cl_;
            ta::lua::state   state_;

        public:

            svc_impl( lua::impl *imp, connection_wptr cl );

            void run_function( gpb::RpcController* controller,
                               const std::string &func )
            {
                if( !func.empty( ) ) {
                    int res = state_.exec_function( func.c_str( ) );
                    if( 0 != res ) {
                        controller->SetFailed( state_.pop_error( ) );
                    }
                }
            }

            void execute_buf( gpb::RpcController* controller,
                              const std::string &buf,
                              const std::string &name,
                              const std::string &func )
            {
                int res = state_.load_buffer( buf.c_str( ), buf.size( ),
                                              name.c_str( ) );
                if( 0 != res ) {
                    controller->SetFailed( state_.pop_error( ) );
                } else {
                    run_function( controller, func );
                }
            }

            void execute_file( gpb::RpcController* controller,
                               const std::string &path,
                               const std::string &func )
            {
                int res = state_.load_file( path.c_str( ) );
                if( 0 != res ) {
                    controller->SetFailed( state_.pop_error( ) );
                } else {
                    run_function( controller, func );
                }
            }

            void init( ::google::protobuf::RpcController* controller,
                       const ::ta::proto::scripting::init_req* request,
                       ::ta::proto::scripting::init_res* response,
                       ::google::protobuf::Closure* done ) override;

            void execute_buffer(
                    ::google::protobuf::RpcController* controller,
                    const ::ta::proto::scripting::execute_buffer_req* request,
                    ::ta::proto::scripting::execute_buffer_res* response,
                    ::google::protobuf::Closure* done ) override;

            void execute_file( ::google::protobuf::RpcController* controller,
                    const ::ta::proto::scripting::execute_file_req* request,
                    ::ta::proto::scripting::execute_file_res* response,
                    ::google::protobuf::Closure* done) override;

            void execute_function(::google::protobuf::RpcController* controller,
                    const ::ta::proto::scripting::execute_function_req* request,
                    ::ta::proto::scripting::execute_function_res* response,
                    ::google::protobuf::Closure* done) override;


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
            ,dispatcher_(app_->get_rpc_service( ))
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
                return std::make_shared<lua_service_wrapper>( app_, cl, inst );
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

    namespace {

        svc_impl::svc_impl( lua::impl *imp, connection_wptr cl )
            :impl_(imp)
            ,cl_(cl)
        { }

        void svc_impl::init(
                ::google::protobuf::RpcController* /*controller*/,
                const ::ta::proto::scripting::init_req* /*request*/,
                ::ta::proto::scripting::init_res* /*response*/,
                ::google::protobuf::Closure* /*done */)
        {
            luawork::init_globals( state_.get_state( ), impl_->app_ );
        }

        void svc_impl::execute_buffer(
            ::google::protobuf::RpcController* controller,
            const ::ta::proto::scripting::execute_buffer_req* request,
            ::ta::proto::scripting::execute_buffer_res* response,
            ::google::protobuf::Closure* /*done*/ )
        {
            execute_buf( controller,
                         request->buffer( ), request->name( ),
                         request->function( ) );
        }

        void svc_impl::execute_file(
                ::google::protobuf::RpcController* controller,
                const ::ta::proto::scripting::execute_file_req* request,
                ::ta::proto::scripting::execute_file_res* /*response*/,
                ::google::protobuf::Closure* /*done*/ )
        {
            execute_file( controller, request->path( ),
                          request->function( ) );
        }

        void svc_impl::execute_function(
                ::google::protobuf::RpcController* controller,
                const ::ta::proto::scripting::execute_function_req* request,
                ::ta::proto::scripting::execute_function_res* /*response*/,
                ::google::protobuf::Closure* /*done*/)
        {
            run_function( controller, request->name( ) );
        }

    }

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

    const std::string &lua::name( ) const NOEXCEPT
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

    
