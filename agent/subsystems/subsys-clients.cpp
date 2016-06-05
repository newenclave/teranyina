#include <set>
#include <list>
#include <functional>

#include "subsys-clients.h"
#include "subsys-listeners.h"

#include "../application.h"
#include "common/utils.h"

#include "vtrc-server/vtrc-listener.h"
#include "vtrc-client/vtrc-client.h"
#include "vtrc-common/vtrc-connection-iface.h"
#include "vtrc-common/vtrc-mutex-typedefs.h"

#include "boost/system/error_code.hpp"

#include "protocol/ctrl.pb.h"

#define LOG(lev) log_(lev, "clients")
#define LOGINF   LOG(level::info)
#define LOGDBG   LOG(level::debug)
#define LOGERR   LOG(level::error)
#define LOGWRN   LOG(level::warning)
namespace ta { namespace agent { namespace subsys {

    namespace {

        using vtrc_client           = vtrc::client::vtrc_client;
        using vtrc_client_sptr      = vtrc::client::vtrc_client_sptr;
        using vtrc_client_wptr      = vtrc::client::vtrc_client_wptr;

        using connection_iface      = vtrc::common::connection_iface;
        using connection_iface_wptr = std::weak_ptr<connection_iface>;
        using vlistener             = vtrc::server::listener;
        using rpc_error_cont        = vtrc::rpc::errors::container;

        struct client_info {
            bool is_client_ = true;
            std::string      address_;
            vtrc_client_wptr client_;
        };

        using clinet_list = std::list<client_info>;

        using level = agent::logger::level;


        struct client_context {
            std::string path_;
        };

        const std::string subsys_name( "clients" );

//        application::service_wrapper_sptr create_service( application * /*app*/,
//                                      vtrc::common::connection_iface_wptr cl )
//        {
//            ///auto inst = std::make_shared<impl_type_here>( app, cl );
//            ///return app->wrap_service( cl, inst );

//            return application::service_wrapper_sptr( );
//        }

    }

    struct clients::impl {

        application                   *app_;
        logger                        &log_;
        std::vector<vtrc_client_sptr>  clients_;

        clinet_list         connections_;
        vtrc::shared_mutex  connections_lock_;

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

        void on_new_connection( connection_iface &c, vlistener &vl )
        {

        }

        void on_stop_connection( connection_iface &c )
        {

        }

        void on_client_connect( vtrc_client_sptr cl )
        {

        }

        void on_client_diconnect( vtrc_client_sptr cl )
        {

        }

        void on_client_init_error( vtrc_client_sptr cl,
                                   const rpc_error_cont &err,
                                   const char *mess )
        {

        }

        void on_client_ready( vtrc_client_sptr cl )
        {

        }

        void add_client( const std::string &path )
        {
            auto ep = utilities::get_endpoint_info( path );
            auto cl = vtrc_client::create( app_->get_io_service( ),
                                           app_->get_rpc_service( ) );

            cl->connection( );
            cl->on_connect_connect( [this, cl]( )
            {
                on_client_connect( cl );
            } );

            cl->on_disconnect_connect( [this, cl]( )
            {
                on_client_diconnect( cl );
            } );

            cl->on_ready_connect( [this, cl]( )
            {
                on_client_ready( cl );
            } );

            cl->on_init_error_connect( [this, cl]( const rpc_error_cont &err,
                                                   const char *mess )
            {
                on_client_init_error( cl, err, mess );
            } );
        }

    };


    clients::clients(application *app)
        :impl_(new impl(app))
    { }

    clients::~clients( )
    {
        delete impl_;
    }

    /// static
    clients::shared_type clients::create( application *app )
    {
        shared_type new_inst(new clients( app ));
        return new_inst;
    }

    void clients::add_client( const std::string &path )
    {
        impl_->add_client( path );
    }

    const std::string &clients::name( )  const
    {
        return subsys_name;
    }

    void clients::init( )
    {
//        auto &lstnr(impl_->app_->subsystem<listeners>( ));
//        namespace ph = std::placeholders;

//        lstnr.on_new_connection_connect(
//            std::bind( &impl::on_new_connection, impl_, ph::_1, ph::_2 ) );

//        lstnr.on_stop_connection_connect(
//            std::bind( &impl::on_stop_connection, impl_, ph::_1 ) );

//        if( !impl_->sd_.empty( ) ) {
//            add_client( impl_->sd_ );
//        }
//        add_client( "@127.0.0.1:12345" );
    }

    void clients::start( )
    {
        impl_->LOGINF << "Started";
    }

    void clients::stop( )
    {
        impl_->LOGINF << "Stopped";
    }

}}}

    
