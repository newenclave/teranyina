#include <set>
#include <list>
#include <functional>

#include "subsys-clients.h"
#include "subsys-listeners.h"

#include "../application.h"
#include "../utils.h"

#include "vtrc-server/vtrc-listener.h"
#include "vtrc-client/vtrc-client.h"
#include "vtrc-common/vtrc-connection-iface.h"

#include "boost/system/error_code.hpp"

#define LOG(lev) log_(lev) << "[ clients] "
#define LOGINF   LOG(level::info)
#define LOGDBG   LOG(level::debug)
#define LOGERR   LOG(level::error)
#define LOGWRN   LOG(level::warning)

namespace ta { namespace agent { namespace subsys {

    namespace {

        using level = agent::logger::level;

        using vtrc_client           = vtrc::client::vtrc_client;
        using vtrc_client_sptr      = vtrc::client::vtrc_client_sptr;
        using vtrc_client_wptr      = vtrc::client::vtrc_client_wptr;

        using connection_iface      = vtrc::common::connection_iface;
        using connection_iface_wptr = std::weak_ptr<connection_iface>;
        using vlistener             = vtrc::server::listener;

        using client_set            = std::vector<const connection_iface_wptr>;

        struct client_context {
            std::string path_;
        };

        const std::string subsys_name( "clients" );

        application::service_wrapper_sptr create_service( application * /*app*/,
                                      vtrc::common::connection_iface_wptr cl )
        {
            ///auto inst = std::make_shared<impl_type_here>( app, cl );
            ///return app->wrap_service( cl, inst );

            return application::service_wrapper_sptr( );
        }

    }

    struct clients::impl {

        application     *app_;
        logger          &log_;
        std::vector<vtrc_client_sptr>  clients_;

        //client_set      connections_;

        impl( application *app )
            :app_(app)
            ,log_(app_->get_logger( ))
        { }

        void reg_creator( const std::string &name,
                          application::service_getter_type func )
        {
            app_->register_service_creator( name, func );
        }

        void unreg_creator( const std::string &name )
        {
            app_->unregister_service_creator( name );
        }

        void on_new_connection( const connection_iface &c, vlistener &vl )
        {

        }

        void on_stop_connection( const connection_iface &c )
        {

        }

    };


    clients::clients( application *app )
        :impl_(new impl(app))
    { }

    clients::~clients( )
    {
        delete impl_;
    }

    /// static
    clients::shared_type clients::create( application *app )
    {
        shared_type new_inst(new clients(app));
        return new_inst;
    }

    void clients::add_client( const std::string &path )
    {
        auto ep = utilities::get_endpoint_info( path );
        auto cl = vtrc_client::create( impl_->app_->get_io_service( ),
                                       impl_->app_->get_rpc_service( ) );

        cl->connection( );
        impl_->clients_.push_back( cl );

        if( ep.is_local( ) ) {
        } else {
            cl->async_connect( ep.addpess, ep.service,
                            [this, cl]( const boost::system::error_code &e ) {
                                impl_->LOGINF << "Connected to "
                                     << cl->connection( )->name( )
                                     << "; " << e.message( )
                                      ;
                            }, true );
        }

        //impl_->LOGINF << ep;
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

//        add_client( "@127.0.0.1:12345" );
    }

    void clients::start( )
    {
        add_client( "@127.0.0.1:12345" );
        impl_->LOGINF << "Started";
    }

    void clients::stop( )
    {
        impl_->LOGINF << "Stopped";
    }


}}}

    
