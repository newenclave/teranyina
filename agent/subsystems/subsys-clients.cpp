
#include "subsys-clients.h"
#include "../application.h"
//#include "subsys-log.h"

//#include "vtrc-memory.h"

//#define LOG(lev) log_(lev) << "[clients] "
//#define LOGINF   LOG(logger::info)
//#define LOGDBG   LOG(logger::debug)
//#define LOGERR   LOG(logger::error)
//#define LOGWRN   LOG(logger::warning)

namespace ta { namespace agent { namespace subsys {

    namespace {

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
//        logger          &log_;

        impl( application *app )
            :app_(app)
//            ,log_(app_->subsystem<subsys::log>( ).get_logger( ))
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

    const std::string &clients::name( )  const
    {
        return subsys_name;
    }

    void clients::init( )
    {

    }

    void clients::start( )
    {
//        impl_->LOGINF << "Started.";
    }

    void clients::stop( )
    {
//        impl_->LOGINF << "Stopped.";
    }


}}}

    
