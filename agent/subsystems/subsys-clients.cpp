
#include "subsys-clients.h"
#include "../application.h"
#include "../utils.h"

#include "vtrc-client/vtrc-client.h"

#define LOG(lev) log_(lev) << "[ clients] "
#define LOGINF   LOG(level::info)
#define LOGDBG   LOG(level::debug)
#define LOGERR   LOG(level::error)
#define LOGWRN   LOG(level::warning)

namespace ta { namespace agent { namespace subsys {

    namespace {

        using level = agent::logger::level;

        using vtrc_client      = vtrc::client::vtrc_client;

        using vtrc_client_sptr = vtrc::client::vtrc_client_sptr;
        using vtrc_client_wptr = vtrc::client::vtrc_client_wptr;

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

    };


    clients::clients( application *app )
        :impl_(new impl(app))
    { }

    clients::~clients( )
    {
        delete impl_;
    }

    /// static
    vtrc::shared_ptr<clients> clients::create( application *app )
    {
        vtrc::shared_ptr<clients> new_inst(new clients(app));
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
                                [this]( const boost::system::error_code &e ) {
                                    impl_->LOGINF << "Connected!";
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
        //add_client( "@127.0.0.1:12346" );
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

    
